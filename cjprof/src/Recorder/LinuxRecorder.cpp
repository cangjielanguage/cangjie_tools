// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tuple>
#include <regex>
#include <ratio>
#include <cxxabi.h>
#include "Demangler/CangjieDemangle.h"
#include "Symbol/Elf.h"
#include "Recorder/LinuxRecorder.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

/*
 * Since the mmap size of the perf data should be 1+2^n pages, where the first page is a metadata page,
 * and the rest is the ring-buffer pages, so the ring-buffer size should be 2^n pages.
 */
static constexpr uint64_t RBUF_SIZE = 256 * PAGE_SIZE;
static const std::string TMP_SAMPLE_DATA_DIR = "./cjprof-logs";
static const std::string TMP_SAMPLE_DATA_FILE_PREFIX = "./cjprof-logs/cjprof-cpu-sample-data-";

bool LinuxRecorder::Start()
{
    if (!createDirIfNotExists(TMP_SAMPLE_DATA_DIR)) {
        return false;
    }
    m_stop = false;
    for (auto pid : TraceProcess()) {
        ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT);
        Start(pid);
    }

    signal(SIGINT, [](int sig) {
        LinuxRecorder::GetInstance().Stop();
        signal(sig, SIG_DFL);

        /* Create a child process that exits immediately to wake up the `wait()` system call in the following loop. */
        if (fork() == 0) {
            _exit(0);
        }
    });

    /* Check the stop event by status */
    auto traceEventChecker = [](uint64_t status, uint64_t event) { return (status >> 8U) == (SIGTRAP | (event << 8U)); };
    while (!m_sampleThreads.empty() && !m_stop) {
        int status = 0;
        auto pid = wait(&status);
        if (traceEventChecker(status, PTRACE_EVENT_CLONE)) {
            pid_t spid = 0;
            ptrace(PTRACE_GETEVENTMSG, pid, nullptr, &spid);
            ptrace(PTRACE_CONT, pid, nullptr, nullptr);
            Start(spid);
        } else if (traceEventChecker(status, PTRACE_EVENT_EXIT)) {
            Stop(pid);
        } else if (WIFSTOPPED((unsigned int)status)) {
            ptrace(PTRACE_CONT, pid, nullptr, WSTOPSIG((unsigned int)status));
        }
    }

    while (!m_sampleThreads.empty()) {
        Stop(m_sampleThreads.begin()->first);
    }

    if (m_stop && (m_cfg.mode == LinuxRecorder::Cfg::LAUNCH)) {
        kill(m_pids[0], SIGTERM);
        wait(nullptr);
    }

    MergeSampleData();
    deleteDirIfExists(TMP_SAMPLE_DATA_DIR);

    return true;
}

bool LinuxRecorder::createDirIfNotExists(const std::string& dirPath)
{
    struct stat sb;
    if (stat(dirPath.c_str(), &sb) == 0) {
        if (S_ISDIR(sb.st_mode)) {
            return true; // Directory already exists
        }
    }

    mode_t mode = 0666;
    if (mkdir(dirPath.c_str(), mode) == -1) {
        fprintf(stderr, "error: create directory (%s) failed.\n", dirPath.c_str());
        return false;
    }

    return true;
}

void LinuxRecorder::Stop()
{
    m_stop = true;
}

void LinuxRecorder::SampleThreadEntry(pid_t pid)
{
    perf_event_attr attr = {0};
    attr.size = sizeof(perf_event_attr);
    attr.type = PERF_TYPE_SOFTWARE;
    attr.config = PERF_COUNT_SW_CPU_CLOCK;
    attr.freq = 1;
    attr.sample_freq = m_cfg.freq;
    attr.sample_type = PERF_SAMPLE_CALLCHAIN;
    attr.mmap = 1;
    attr.exclude_kernel = 1;
    attr.disabled = 1;

    int fd = static_cast<int>(syscall(__NR_perf_event_open, &attr, pid, -1, -1, 0));
    if (fd < 0) {
        fprintf(stderr, "error: Open perf event failed (%d).\n", errno);
        return;
    }

    auto addr = mmap(0, PAGE_SIZE + RBUF_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "error: No memory enough for sample data.\n");
        close(fd);
        return;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK | O_ASYNC);
    fcntl(fd, F_SETSIG, SIGIO);

    f_owner_ex owner = { F_OWNER_TID, (pid_t)syscall(__NR_gettid) };
    fcntl(fd, F_SETOWN_EX, &owner);

    signal(SIGIO, [](int sig) { (void)sig; });

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

    std::ofstream ofs(TMP_SAMPLE_DATA_FILE_PREFIX + std::to_string(pid));
    if (!ofs.fail()) {
        auto rinfo = (const perf_event_mmap_page *)addr;
        std::string_view rbuf((const char *)addr + PAGE_SIZE, RBUF_SIZE);
        uint64_t last = 0;
        while (!m_stopSampleThread[pid]) {
            uint64_t head = rinfo->data_head % RBUF_SIZE;
            uint64_t tail = last;
            if (head >= tail) {
                ofs.write(rbuf.data() + tail, head - tail);
            } else {
                ofs.write(rbuf.data() + tail, RBUF_SIZE - tail);
                ofs.write(rbuf.data(), head);
            }

            last = head;
            usleep(1);
        }

        ofs.close();
    }

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    close(fd);

    munmap(addr, PAGE_SIZE + RBUF_SIZE);
}

static std::string GetSymNameByAddr(const std::vector<Symbol> &syms, uint64_t addr)
{
    Symbol sym = { addr, 1, "" };
    auto it = std::lower_bound(syms.begin(), syms.end(), sym);
    if (it == syms.end()) {
        return std::string();
    }

    /* 'sym' equals to '*it' */
    if (!(sym < *it) && !(*it < sym)) {
        /* Try using the demangle rule of Cangjie first. */
        auto demangle = Cangjie::Demangle((*it).name).GetFullName();
        /* If it is not a Cangjie mangled name, try using the demangle rule of C++. */
        if (demangle == (*it).name) {
            int status;
            char* demangled = abi::__cxa_demangle((*it).name.c_str(), nullptr, nullptr, &status);
            if (status == 0 && demangled) {
                std::string result(demangled);
                free(demangled);
                return result;
            }
        }

        return demangle;
    }

    return "";
}

void LinuxRecorder::MergeSampleData()
{
    printf("Generating sampling data...\n");
    for (auto mmap : m_mmaps) {
        auto elfSyms = Elf::ParseSymbols(mmap.file);
        if (mmap.dynamic) {
            for (auto &sym : elfSyms) {
                sym.addr += m_dylibBases[mmap.file];
            }
        }

        if (elfSyms.empty()) {
            continue;
        }

        std::sort(elfSyms.begin(), elfSyms.end());
        m_symbols.insert(m_symbols.end(), elfSyms.begin(), elfSyms.end());
    }

    Hprof hprof;
    for (auto pid : m_pids) {
        std::string file = TMP_SAMPLE_DATA_FILE_PREFIX + std::to_string(pid);
        std::ifstream ifs(file);
        if (ifs.fail()) {
            continue;
        }

        ifs.seekg(0, ifs.end);
        auto pos = ifs.tellg();
        if (pos <= 0) {
            fprintf(stderr, "error: Cannot determine file size.\n");
            return;
        }
        uint64_t size = static_cast<uint64_t>(pos);
        auto data = std::make_unique<char[]>(size);

        ifs.seekg(0, ifs.beg);
        ifs.read(data.get(), size);
        ifs.close();

        unlink(file.c_str());

        AnalyzeSampleData(hprof, std::move(data), size);
    }

    std::ofstream ofs(m_cfg.output);
    if (ofs.fail()) {
        fprintf(stderr, "error: Create out file '%s' failed.\n", m_cfg.output.c_str());
        return;
    }

    auto data = hprof.GenData();
    ofs.write(data.data(), data.size());
    ofs.close();
}

void LinuxRecorder::AnalyzeSampleData(Hprof &hprof, std::unique_ptr<char[]> data, size_t size)
{
    if (!data) {
        return;
    }
    auto period = std::micro::den / m_cfg.freq;
    uint64_t offset = 0;
    while (offset < size) {
        auto header = (const perf_event_header *)(data.get() + offset);
        if ((header->size == 0) || (header->size > size - offset)) {
            break;
        }

        if (header->type != PERF_RECORD_SAMPLE) {
            offset += header->size;
            continue;
        }

        auto sample = (const PerfRecordSample *)header;
        std::vector<std::string> callChain;
        for (size_t i = 0; i < sample->nr; i++) {
            auto addr = sample->ips[i];
            if (addr == 0) {
                break;
            }

            auto mmap = std::lower_bound(m_mmaps.begin(), m_mmaps.end(), Mmap({ addr, 1, "", false }));
            if (mmap == m_mmaps.end() || addr < mmap->addr) {
                continue;
            }

            auto name = GetSymNameByAddr(m_symbols, addr);
            if (!name.empty()) {
                callChain.push_back(name);
            } else {
                std::stringstream ss;
                ss << "[" << mmap->file.substr(mmap->file.rfind('/') + 1) << "+0x" << std::hex <<
                    (mmap->dynamic ? addr - m_dylibBases[mmap->file] : addr) << "]";
                callChain.push_back(ss.str());
            }
        }

        if (!callChain.empty()) {
            hprof.PutCpuSample(callChain, static_cast<unsigned int>(period));
        }

        offset += header->size;
    }
}

void LinuxRecorder::deleteDirIfExists(const std::string& dirPath)
{
    struct stat sb;
    if (stat(dirPath.c_str(), &sb) == 0) {
        if (S_ISDIR(sb.st_mode)) {
            rmdir(dirPath.c_str());
        }
    }
    return;
}


void LinuxRecorder::Start(pid_t pid)
{
    m_pids.push_back(pid);
    m_stopSampleThread[pid] = false;
    m_sampleThreads[pid] = std::thread(&LinuxRecorder::SampleThreadEntry, this, pid);
    ptrace(PTRACE_CONT, pid, nullptr, nullptr);
}

void LinuxRecorder::Stop(pid_t pid)
{
    auto it = m_sampleThreads.find(pid);
    if (it == m_sampleThreads.end()) {
        return;
    }

    if (pid == m_pids[0]) {
        HandleProcessMaps(pid);
    }

    m_stopSampleThread[pid] = true;
    if (it->second.joinable()) {
        it->second.join();
    }

    m_sampleThreads.erase(it);
    ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
}

void LinuxRecorder::HandleProcessMaps(pid_t pid)
{
    std::ifstream ifs("/proc/" + std::to_string(pid) + "/maps");
    if (ifs.fail()) {
        return;
    }

    std::vector<std::string> lines;
    while (!ifs.eof()) {
        std::string line;
        std::getline(ifs, line);
        lines.push_back(line);
    }

    ifs.close();

    /*
     * address           perms offset  dev   inode       pathname
     * 00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/dbus-daemon
     */
    std::regex ex("^([0-9a-f]+)-([0-9a-f]+) ([rwxp-]{4}) [0-9a-f]+ [0-9a-f]+:[0-9a-f]+ [0-9]+[\\s]+([\\S]+)$");
    std::smatch sm;
    for (const auto &line : lines) {
        if (!std::regex_match(line, sm, ex)) {
            continue;
        }

        auto start = std::strtoull(sm.str(1).c_str(), nullptr, 16);
        auto end = std::strtoull(sm.str(2).c_str(), nullptr, 16);
        auto perms = sm.str(3);
        auto name = sm.str(4);
        if (m_dylibBases.find(name) == m_dylibBases.end()) {
            m_dylibBases[name] = start;
        }

        if (perms == "r-xp") {
            m_mmaps.push_back({ start, end - start, name, Elf::GetType(name) == Elf::Type::DYN });
        }
    }
}

static std::vector<pid_t> GetSubPid(pid_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/task";
    char *canonicalPath = realpath(path.c_str(), nullptr);
    if (canonicalPath == nullptr) {
        return {};
    }
    DIR *dir = opendir(canonicalPath);
    free(canonicalPath);
    if (!dir) {
        return {};
    }
    std::vector<pid_t> spids;
    for (auto ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        std::string sdir(ent->d_name);
        if ((sdir == ".") || (sdir == "..")) {
            continue;
        }

        spids.push_back(std::stoi(sdir));
    }

    closedir(dir);

    return spids;
}

std::vector<pid_t> LinuxRecorder::TraceProcess()
{
    std::vector<pid_t> pids;
    pid_t pid;
    if (m_cfg.mode == LinuxRecorder::Cfg::LAUNCH) {
        pid = fork();
        if (pid == 0) {
            if ((ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) || (execve(m_cfg.argv[0], m_cfg.argv, environ) < 0)) {
                exit(errno);
            }
        }
    } else {
        pid = m_cfg.pid;
        if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0) {
            fprintf(stderr, "error: Trace process (%d) failed (%d).\n", pid, errno);
            return pids;
        }
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED((unsigned int)status)) {
        fprintf(stderr, "error: Execute '%s' failed (%u).\n", m_cfg.argv[0], WEXITSTATUS((unsigned int)status));
        return pids;
    }

    pids.push_back(pid);
    if (m_cfg.mode == LinuxRecorder::Cfg::ATTACH) {
        for (auto spid : GetSubPid(pid)) {
            if (spid == pid) {
                continue;
            }

            ptrace(PTRACE_ATTACH, spid, 0, 0);
            waitpid(spid, nullptr, 0);

            pids.push_back(spid);
        }
    }

    return pids;
}
