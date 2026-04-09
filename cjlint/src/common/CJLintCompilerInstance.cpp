// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CJLintCompilerInstance.h"
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace Cangjie;
using namespace Cangjie::CodeCheck;
using namespace AST;

namespace {
constexpr int PROCESS_LAUNCH_FAILED = -1;
constexpr int NO_PIPELINE_ERROR = 0;
constexpr int CMD_ESCAPE_FACTOR = 2;
constexpr int ESCAPE_QUOTE_EXTRA = 1;
#ifndef _WIN32
constexpr int CHILD_EXEC_FAILED_EXIT_CODE = 127;
#endif

#ifdef _WIN32
static std::wstring ToWideString(const std::string& s)
{
    if (s.empty()) {
        return {};
    }
    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    if (len <= 0) {
        // Fallback to system code page if input is not valid UTF-8.
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    }
    if (len <= 0) {
        return {};
    }
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

static std::wstring QuoteWindowsArg(const std::wstring& arg)
{
    // Escape/quote one argv token using Windows command-line parsing rules so
    // CreateProcessW receives the exact intended argument boundaries.
    if (arg.empty()) {
        return L"\"\"";
    }
    bool needsQuotes = arg.find_first_of(L" \t\r\n\v\"") != std::wstring::npos;
    if (!needsQuotes) {
        return arg;
    }

    std::wstring res;
    res.push_back(L'"');
    size_t backslashes = 0;
    for (wchar_t ch : arg) {
        if (ch == L'\\') {
            backslashes++;
            continue;
        }
        if (ch == L'"') {
            // Escape backslashes for a quote.
            res.append(backslashes * CMD_ESCAPE_FACTOR + ESCAPE_QUOTE_EXTRA, L'\\');
            res.push_back(L'"');
            backslashes = 0;
            continue;
        }
        if (backslashes > 0) {
            res.append(backslashes, L'\\');
            backslashes = 0;
        }
        res.push_back(ch);
    }
    if (backslashes > 0) {
        // Trailing backslashes before the closing quote must be doubled.
        res.append(backslashes * CMD_ESCAPE_FACTOR, L'\\');
    }
    res.push_back(L'"');
    return res;
}

static int RunCjcCompileMacroWindows(const std::string& executablePath, const std::string& package)
{
    // Run cjc directly instead of using `system`/shell to avoid command injection.
    std::wstring exeW = ToWideString(executablePath);
    std::wstring pkgW = ToWideString(package);
    if (exeW.empty() || pkgW.empty()) {
        return PROCESS_LAUNCH_FAILED;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;
    HANDLE nulHandle = CreateFileW(
        L"NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (nulHandle == INVALID_HANDLE_VALUE) {
        return PROCESS_LAUNCH_FAILED;
    }

    std::wstring cmdLine;
    cmdLine += L"\"";
    cmdLine += exeW;
    cmdLine += L"\"";
    cmdLine += L" " + QuoteWindowsArg(L"-p") + L" " + QuoteWindowsArg(pkgW);
    cmdLine += L" " + QuoteWindowsArg(L"--compile-macro") + L" " + QuoteWindowsArg(L"-o") + L" " +
        QuoteWindowsArg(pkgW);

    std::vector<wchar_t> buf(cmdLine.begin(), cmdLine.end());
    buf.push_back(L'\0');

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = nulHandle;
    si.hStdError = nulHandle;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcessW(
        exeW.c_str(), buf.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
    if (!ok) {
        CloseHandle(nulHandle);
        return PROCESS_LAUNCH_FAILED;
    }

    CloseHandle(nulHandle);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return static_cast<int>(exitCode);
}
#else
static int RunCjcCompileMacroLinux(const std::string& executablePath, const std::string& package)
{
    // Run cjc directly without shell expansion to prevent command injection.
    pid_t pid = fork();
    if (pid < 0) {
        return PROCESS_LAUNCH_FAILED;
    }
    if (pid == 0) {
        // Redirect stdout/stderr to /dev/null to mimic previous `> /dev/null 2>&1`.
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull != -1) {
            (void)dup2(devNull, STDOUT_FILENO);
            (void)dup2(devNull, STDERR_FILENO);
            if (devNull > 2) {
                (void)close(devNull);
            }
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(executablePath.c_str()));
        argv.push_back(const_cast<char*>("-p"));
        argv.push_back(const_cast<char*>(package.c_str()));
        argv.push_back(const_cast<char*>("--compile-macro"));
        argv.push_back(const_cast<char*>("-o"));
        argv.push_back(const_cast<char*>(package.c_str()));
        argv.push_back(nullptr);

        execv(executablePath.c_str(), argv.data());
        _exit(CHILD_EXEC_FAILED_EXIT_CODE);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        return PROCESS_LAUNCH_FAILED;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return PROCESS_LAUNCH_FAILED;
}
#endif
} // namespace

#ifdef _WIN32
const std::string EXTENSION = ".dll";
#else
const std::string EXTENSION = ".so";
#endif

bool CJLintCompilerInstance::PerformImportPackage()
{
    std::vector<std::string> importPaths;
    for (auto it = srcPkgs.begin(); it != srcPkgs.end();) {
        auto srcPkg = it->get();
        if (!srcPkg->isMacroPackage) {
            ++it;
            continue;
        }
        auto macroPkgName = FileUtil::SplitStr(srcPkg->fullPackageName, '.');
        std::string package = FileUtil::JoinPath(invocation.globalOptions.moduleSrcPath, macroPkgName.back());
        std::string output;
        if (FileUtil::GetAbsPath(package).has_value()) {
            output = package + "/lib-macro_" + srcPkg->fullPackageName + EXTENSION;
        } else {
            ++it;
            continue;
        }
        int res = 0;
#ifdef _WIN32
        if (!FileUtil::FileExist(output.c_str()) || remove(output.c_str()) == 0) {
#endif
            res = NO_PIPELINE_ERROR;
#ifdef _WIN32
            res = RunCjcCompileMacroWindows(invocation.globalOptions.executablePath, package);
#else
            res = RunCjcCompileMacroLinux(invocation.globalOptions.executablePath, package);
#endif
#ifdef _WIN32
        }
#endif
        if (res != 0) {
            (void)diag.Diagnose(DiagKind::macro_expand_failed);
            return false;
        }
        (void)macroPath.insert(package + PATH_SEPARATOR + srcPkg->fullPackageName);

        importPaths.emplace_back(package);
        it = srcPkgs.erase(it);
    }
    for (auto& importPath : importPaths) {
        invocation.globalOptions.importPaths.emplace_back(importPath);
    }
    return CompilerInstance::PerformImportPackage();
}

bool CJLintCompilerInstance::Compile(CompileStage stage)
{
    if ((this->currentStage == CompileStage::PARSE) || this->currentStage > stage) {
        bool ret = CompilerInstance::Compile(stage);
        if (ret) {
            this->currentStage = static_cast<CompileStage>(static_cast<int>(stage) + 1);
        }
        return ret;
    }
    if ((this->currentStage == CompileStage::PARSE) && !InitCompilerInstance()) {
        diag.ReportErrorAndWarningCount();
        return false;
    }
    for (int i = static_cast<int>(this->currentStage); i <= static_cast<int>(stage); i++) {
        Cangjie::ICE::TriggerPointSetter iceSetter(static_cast<CompileStage>(i));
        if (!performMap[static_cast<CompileStage>(i)](this)) {
            diag.ReportErrorAndWarningCount();
            return false;
        }
    }
    Cangjie::ICE::TriggerPointSetter iceSetter(stage);
    diag.ReportErrorAndWarningCount();
    this->currentStage = static_cast<CompileStage>(static_cast<int>(stage) + 1);
    return true;
}
