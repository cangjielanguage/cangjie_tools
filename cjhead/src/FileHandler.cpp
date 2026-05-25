// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FileHandler.h"

#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <exception>
#include <memory>
#include <regex>
#include <list>
#include <ios>

#include "CJHeadCompilerInvocation.h"
#include "FormatCodeProcessor.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Cangjie::Format;

namespace CJHead {
FileHandler::FileHandler(int argc, const std::vector<std::string> &argv,
                         const std::unordered_map<std::string, std::string> &env)
    : argc_(argc),
      argv_(argv),
      environment_vars_(env)
{
}

const std::unordered_map<std::string, std::function<void(CJHeadParam &, const std::string &)>>
    FileHandler::option_map_ = {{"-i",
                                 [](CJHeadParam &param, const std::string &optarg) {
                                     if (CJHeadUtil::CheckPath(optarg)) {
                                         param.input_dir_ = optarg;
                                     }
                                 }},
                                {"-o",
                                 [](CJHeadParam &param, const std::string &optarg) {
                                     try {
                                         fs::path p{optarg};
                                         param.output_dir_ = optarg;
                                     } catch (const std::exception &e) {
                                         std::cerr << "Illegal path: " << e.what() << std::endl;
                                         exit(-1);
                                     }
                                 }},
                                {"--import-path",
                                 [](CJHeadParam &param, const std::string &optarg) {
                                     if (CJHeadUtil::CheckPath(optarg)) {
                                         param.import_path_ = optarg;
                                     }
                                 }},
                                {"--macro-path",
                                 [](CJHeadParam &param, const std::string &optarg) {
                                     if (CJHeadUtil::CheckPath(optarg)) {
                                         param.macro_path_ = optarg;
                                     }
                                 }},
                                {"--exclude", [](CJHeadParam &param, const std::string &optarg) {
                                     if (CJHeadUtil::CheckPath(optarg)) {
                                         param.exclude_config_ = optarg;
                                     }
                                 }}};

auto FileHandler::ParseArguments() -> bool
{
    if (argc_ == 1) {
        CJHeadUtil::PrintHelp();
        return false;
    }
    CJHeadParam params;
    params.package_mode_ = false;
    int index = 0;
    while (index < argc_ - 1) {
        std::string arg = argv_[index];
        if (arg == "-h") {
            CJHeadUtil::PrintHelp();
            return false;
        }
        if (arg == "-p") {
            params.package_mode_ = true;
            ++index;
            continue;
        }
        auto handle_option = option_map_.find(arg);
        if (handle_option == option_map_.end())
            return false;
        if (index + 1 >= argc_ - 1)
            return false;
        handle_option->second(params, argv_[index + 1]);
        index++;
        index++;
    }

    input_dir_ = fs::path(params.input_dir_);
    if (!fs::exists(input_dir_))
        return false;

    if (fs::is_regular_file(input_dir_)) {
        input_file_ = input_dir_.filename().string();
        input_dir_ = input_dir_.parent_path();
    } else {
        input_file_.clear();
    }

    output_dir_ = fs::path(params.output_dir_);
    if (!params.exclude_config_.empty()) {
        exclude_config_ = fs::path(params.exclude_config_);
        ParseExcludeConfig();
    }
    package_mode_ = params.package_mode_;
    import_path_ = params.import_path_;
    macro_path_ = params.macro_path_;
    return true;
}

auto FileHandler::ParseExcludeConfig() -> bool
{
    std::ifstream exclude_file(exclude_config_);
    std::string line;
    while (std::getline(exclude_file, line)) {
        if (line.empty()) {
            continue;
        }
        try {
            exclude_regs_.emplace_back(line);
        } catch (std::regex_error &e) {
            std::cerr << "invalid regex : " << line << " " << e.what() << "\n";
        }
    }
    return true;
}

auto FileHandler::IsExcludeFile(const std::string &file_name) -> bool
{
    return std::find_if(exclude_regs_.begin(), exclude_regs_.end(), [&](const std::regex &reg) {
               return std::regex_search(file_name, reg);
           }) != exclude_regs_.end();
}

auto FileHandler::Run() -> bool
{
    return TraverseAllFiles(input_dir_);
}

auto FileHandler::TraverseAllFiles(const fs::path &input_dir) -> bool
{
    if (fs::is_directory(input_dir)) {
        HandlePackage(input_dir);
        for (auto &sub_dir : fs::directory_iterator(input_dir)) {
            if (fs::is_directory(sub_dir)) {
                TraverseAllFiles(sub_dir.path());
            }
        }
    } else if (fs::is_regular_file(input_dir)) {
        fs::path file_path = input_dir;
        fs::path file_dir = file_path.parent_path();
        std::string file_name = file_path.filename().string();
        if (IsExcludeFile(file_name)) {
            return true;
        }

        SourceManager sm{};
        DiagnosticEngine diag{};
        diag.SetSourceManager(&sm);
        auto invocation = std::make_unique<CJHeadCompilerInvocation>();
        invocation->SetCangjieHome(environment_vars_);
        invocation->SetCompilePackage(false);
        invocation->SetPackagePath(file_dir.string());
        invocation->SetImportPath(import_path_);
        invocation->SetImportPath(macro_path_);
        invocation->globalOptions.enableAddCommentToAst = true;
        invocation->globalOptions.enableMacroDebug = true;
        invocation->globalOptions.enableMacroInLSP = true;
        auto compiler_instance = std::make_unique<CJHeadCompilerInstance>(*invocation, diag);
        bool is_succeed = compiler_instance->Compile(CompileStage::SEMA);
        auto source_packages = compiler_instance->GetSourcePackages();

        std::vector<OwnedPtr<File>> &files = (source_packages.front())->files;
        for (auto &file : files) {
            if (!exclude_config_.empty() && IsExcludeFile(file->fileName)) {
                continue;
            }
            std::string formatted_content = HandleSingleFile(compiler_instance, file);
            Output(file_path, formatted_content);
        }
    }
    return true;
}

void SortImports(OwnedPtr<File>& package_file)
{
    std::sort(package_file->imports.begin(), package_file->imports.end(), [](auto &l, auto &r) {
        std::string a = l->content.ToString();
        std::string b = r->content.ToString();

        size_t pos_a = a.find(' ');
        size_t pos_b = b.find(' ');
        if (pos_a == std::string::npos || pos_b == std::string::npos) {
            return a < b;
        }

        std::string word_a = a.substr(0, pos_a);
        std::string word_b = b.substr(0, pos_b);

        return word_a < word_b;
    });
}

void MergeFiles(std::vector<OwnedPtr<File>>& pruned_files, OwnedPtr<File>& package_file)
{
    int length = pruned_files.size();
    std::unordered_set<std::string> import_set;

    for (auto &import : package_file->imports)
        import_set.insert(import->content.ToString());

    auto addImport = [&import_set, &package_file](OwnedPtr<ImportSpec>& import) {
        auto content = import->content.ToString();
        if (!import_set.count(content)) {
            import_set.insert(content);
            package_file->imports.emplace_back(std::move(import));
        }
    };

    for (int i = 1; i < length; i++) {
        auto &file = pruned_files[i];

        for (auto &decl : file->decls)
            package_file->decls.emplace_back(std::move(decl));

        for (auto &import : file->imports)
            addImport(import);
    }

    SortImports(package_file);
}

auto FileHandler::HandlePackage(const fs::path &input_dir) -> bool
{
    SourceManager sm{};
    DiagnosticEngine diag{};
    diag.SetSourceManager(&sm);
    auto invocation = std::make_unique<CJHeadCompilerInvocation>();
    invocation->SetCangjieHome(environment_vars_);
    invocation->SetCompilePackage(true);
    invocation->SetPackagePath(input_dir.string());
    invocation->SetImportPath(import_path_);
    invocation->SetImportPath(macro_path_);
    invocation->globalOptions.enableAddCommentToAst = true;
    invocation->globalOptions.enableMacroDebug = true;
    invocation->globalOptions.enableMacroInLSP = true;
    auto compiler_instance = std::make_unique<CJHeadCompilerInstance>(*invocation, diag);
    bool is_succeed = compiler_instance->Compile(CompileStage::SEMA);
    auto source_packages = compiler_instance->GetSourcePackages();
    std::vector<OwnedPtr<File>> &files = (source_packages.front())->files;
    if (files.empty())
        return true;
    full_package_name_ = (source_packages.front())->fullPackageName;
    std::list<std::string> formatted_files;
    std::vector<OwnedPtr<File>> pruned_files;

    for (auto &file : files) {
        if (!exclude_config_.empty() && IsExcludeFile(file->fileName))
            continue;
        std::string formatted_content = HandleSingleFile(compiler_instance, file);
        if (!package_mode_)
            Output(fs::path(file->filePath), formatted_content);
        else
            pruned_files.emplace_back(std::move(file));
    }

    if (package_mode_ && !pruned_files.empty()) {
        fs::path output_path = output_dir_ / (full_package_name_ + ".cj.d");
        fs::create_directories(output_path.parent_path());
        auto &package_file = pruned_files.front();
        MergeFiles(pruned_files, package_file);
        std::string formatted_content = HandleSingleFile(compiler_instance, package_file);
        std::ofstream out_header(output_path);
        out_header << formatted_content;
    }
    return true;
}

std::string GetExecutableDirectory() {
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len == -1) {
        std::cerr << "⚠️ Failed to read /proc/self/exe" << std::endl;
        return ".";
    }
    buffer[len] = '\0';

    std::string path(buffer);
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) return ".";
    return path.substr(0, pos);
}

auto FileHandler::HandleSingleFile(const std::unique_ptr<CJHeadCompilerInstance> &compiler_instance,
                                   const OwnedPtr<File> &cur_file) -> std::string
{
    const Region region = Region::wholeFile;
    FormatCodeProcessor processor("", cur_file->filePath, region);

    HeadFileVisitor head_file_visitor(compiler_instance, cur_file);
    head_file_visitor.Run();
    std::string formatted_content = processor.TransformAstToText(cur_file);

    std::string copyright_content;
    {
        std::string current_dir = GetExecutableDirectory();
        std::string resource_path = current_dir + "/resources/COPYRIGHT.txt";
        std::ifstream file(resource_path, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            copyright_content = std::string(
                std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>()
            );
            file.close();

            auto first_non_space = copyright_content.find_first_not_of(" \t\n\r");
            if (first_non_space == std::string::npos) {
                copyright_content.clear();
            }
        }
    }

    if (!copyright_content.empty()) {
        return copyright_content + "\n" + formatted_content;
    } else {
        return formatted_content;
    }
}

std::vector<std::string> SplitPath(const std::string &path)
{
    std::vector<std::string> parts;
    std::string part;
    std::istringstream iss(path);
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}

std::string GetRelativePath(const std::string &inputPath, const std::string &basePath)
{
    std::string relativePath = inputPath.substr(basePath.length());
    if (relativePath.empty() || relativePath[0] != '/') {
        relativePath = "/" + relativePath;
    }
    return relativePath;
}

void EnsureDirectoryExists(const std::string &path)
{
    std::string dirPath = path.substr(0, path.find_last_of('/'));
    if (!dirPath.empty()) {
        std::string currentPath = "";
        for (const auto &part : SplitPath(dirPath)) {
            currentPath += part + "/";
            if (!fs::exists(currentPath)) {
                fs::create_directory(currentPath);
            }
        }
    }
}

auto FileHandler::Output(const fs::path &input_path, const std::string &formatted_content) -> bool
{
    std::string output_path = output_dir_.string();
    if (fs::is_directory(output_dir_)) {
        std::string relative_path = GetRelativePath(input_path.string(), input_dir_.string());
        output_path += relative_path;
        output_path += ".d";
    } else {
        output_path = output_dir_.string();
    }

    EnsureDirectoryExists(output_path);

    std::ofstream out_header(output_path);
    out_header << formatted_content;

    return true;
}

}  // namespace CJHead