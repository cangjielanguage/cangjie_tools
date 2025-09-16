// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <cangjie/Basic/Version.h>

#include "../json-rpc/StdioTransport.h"
#include "../languageserver/ArkLanguageServer.h"
#include "../languageserver/logger/CrashReporter.h"
#include "../languageserver/capabilities/shutdown/Shutdown.h"
#include "../languageserver/logger/CrashReporter.h"
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace Cangjie::FileUtil;
namespace {
ark::Environment StringifyEnvironmentPointer(const char *envp[])
{
    ark::Environment environment;
    if (!envp) {
        return environment;
    }
    // Read all environment variables
    const std::string cangjiePath = "CANGJIE_PATH";
    const std::string cangjieHome = "CANGJIE_HOME";
    std::string ldLibraryPath = "LD_LIBRARY_PATH";
#ifdef WIN32
    ldLibraryPath = "PATH";
#endif

#ifdef __APPLE__
    ldLibraryPath = "DYLD_LIBRARY_PATH";
#endif
    for (int i = 0;; i++) {
        // the last element is a null pointer in the envp array
        if (!envp[i]) {
            break;
        }
        std::string item(envp[i]);
        auto pos = item.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        auto key = item.substr(0, pos);
        auto value = item.substr(pos + 1);
#ifdef WIN32
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
#endif
        if (key == cangjiePath) {
            environment.cangjiePath = Cangjie::FileUtil::GetAbsPath(value).value_or("");
            continue;
        }
        if (key == cangjieHome) {
            environment.cangjieHome = Cangjie::FileUtil::GetAbsPath(value).value_or("");
            continue;
        }
        if (key == ldLibraryPath && environment.runtimePath.empty()) {
            environment.runtimePath = ark::PathWindowsToLinux(value);
        }
    }
    return environment;
}

void WriteVersionInfo(const std::string &validFile)
{
    std::ofstream versionInfo;
    versionInfo.open(validFile);
    if (!versionInfo.is_open()) {
        Trace::Log("Create index version file failed");
    }
    versionInfo << Cangjie::CANGJIE_VERSION;
    if (versionInfo.fail()) {
        Trace::Log("Write index version file failed");
    }
    versionInfo.close();
}
} // namespace

int main(int argc, const char *argv[], const char *envp[])
{
    ark::Environment environment = StringifyEnvironmentPointer(envp);
    ark::Options &opts = ark::Options::GetInstance();
    opts.Parse(argc, argv);
    if (opts.IsOptionSet("log-path")) {
        ark::Logger::SetPath(opts.GetLongOption("log-path").value());
    }
    if (opts.IsOptionSet("enable-log") && opts.GetLongOption("enable-log").value() == "true") {
        ark::Logger::SetLogEnable(true);
    }
    if (opts.IsOptionSet('V')) {
        ark::CrashReporter::RegisterHandlers();
    }
    std::string cachePath;
    if (opts.IsOptionSet("cache-path")) {
        cachePath = opts.GetLongOption("cache-path").value();
    }
    Trace::Log("LSP Starting over stdin/stdout");
    // add if/else can add another transport layer
    ark::TransportRegistrar<ark::Transport, ark::StdioTransport> stdioTransportRegistrar("stdio");
    stdioTransportRegistrar.Regist();
    ark::Transport *pStdioTransport = ark::TransportFactory<ark::Transport>::Instance().GetTransport("stdio");
    if (pStdioTransport == nullptr) {
        (void)fprintf(stderr, "error: get transport fail.");
        return 0;
    }
#ifdef WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    pStdioTransport->SetIO(stdin, stdout);

#ifdef MACRO_DYNAMIC
    char splitStr = ':';
    std::string cjRuntimeLibPath = "runtime/lib/linux_x86_64_cjnative/libcangjie-runtime.so";
#ifdef WIN32
    cjRuntimeLibPath = "runtime/lib/windows_x86_64_cjnative/libcangjie-runtime.dll";
    splitStr = ';';
#elif __APPLE__
    cjRuntimeLibPath = "runtime/lib/darwin_x86_64_cjnative/libcangjie-runtime.dylib";
    splitStr = ':';
#endif
    std::string path = environment.runtimePath;
    std::regex pathRegex("/runtime/lib/");
    std::vector<std::string> vectorString(std::sregex_token_iterator(path.begin(), path.end(), pathRegex, -1),
                                          std::sregex_token_iterator());
    path = vectorString[0];
    auto startPos = path.find_last_of(splitStr);
    if (startPos != std::string::npos) {
        path = path.substr(startPos + 1, std::string::npos);
    }
    std::string runtimeLibPath = JoinPath(path, cjRuntimeLibPath);
    environment.runtimePath = runtimeLibPath;
#endif
    std::unique_ptr<ark::lsp::IndexDatabase> indexDB;
    indexDB = std::make_unique<ark::lsp::IndexDatabase>();
    if (cachePath.empty()) {
        ark::CompilerCangjieProject::SetUseDB(false);
    } else {
        ark::CompilerCangjieProject::SetUseDB(true);
        std::string cacheRoot = JoinPath(cachePath, ".cache");
        cachePath = JoinPath(cacheRoot, "index/");
        if (!FileExist(cachePath)) {
            auto ret = CreateDirs(cachePath);
            if (ret == -1) {
                (void)fprintf(stderr, "error: fail to create dir for index.");
                return 0;
            }
        }
        const std::string &validFile = JoinPath(cachePath, "valid.txt");
        std::string dBfilePath = JoinPath(cachePath, "index.db");
        std::string reason;
        bool isinvalidDb = !FileExist(validFile) ||
            ReadFileContent(validFile, reason).value_or("") != Cangjie::CANGJIE_VERSION;
        if (isinvalidDb) {
            bool isDeleteFailed = FileExist(dBfilePath) && !Remove(dBfilePath);
            if (isDeleteFailed) {
                Trace::Log("Remove old db index file failed");
            }
            if (Remove(validFile)) {
                WriteVersionInfo(validFile);
            }
            if (!FileExist(validFile)) {
                WriteVersionInfo(validFile);
            }
        }
        ark::lsp::OpenIndexDatabase(*indexDB, dBfilePath);
    }
    ark::ArkLanguageServer lspServer(*pStdioTransport, environment, indexDB.get());
    Cangjie::ICE::TriggerPointSetter iceSetter(Cangjie::ICE::LSP_TP);
    ark::LSPRet exitCode = lspServer.Run();
    std::string message = "exit mode is abnormal, it's necessary to send shutdown request before exit notify.";
#ifdef MACRO_DYNAMIC
    Cangjie::MacroProcMsger::GetInstance().CloseMacroSrv();
#endif
    if (exitCode == ark::LSPRet::NORMAL_EXIT) {
        Trace::Log("LSP finished");
    }
    if (exitCode == ark::LSPRet::ABNORMAL_EXIT) {
        Trace::Wlog(message);
        (void)fprintf(stderr, "warning: %s", message.c_str());
    }
    if (exitCode == ark::LSPRet::ERR_IO && ark::ShutdownRequested()) {
        std::thread timer([]()-> void {
            const int64_t deadTime = 10;
            std::this_thread::sleep_for(std::chrono::seconds(deadTime));
            std::exit(0);
        });
        timer.detach();
    }
    return 0;
}
