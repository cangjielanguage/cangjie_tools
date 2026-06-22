// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Analyzer/HttpServer.h"
#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Logger.h"
#include <httplib.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>
#endif

namespace cjprof {

constexpr int HTTP_STATUS_BAD_REQUEST = 400;
constexpr int HTTP_STATUS_NOT_FOUND = 404;

static std::string guessMimeType(const std::string& path)
{
    size_t lastDot = path.rfind('.');
    if (lastDot == std::string::npos) {
        return "application/octet-stream";
    }
    std::string ext = path.substr(lastDot + 1);
    if (ext == "html" || ext == "htm") {
        return "text/html";
    }
    if (ext == "js") {
        return "application/javascript";
    }
    if (ext == "css") {
        return "text/css";
    }
    if (ext == "json") {
        return "application/json";
    }
    if (ext == "png") {
        return "image/png";
    }
    if (ext == "jpg" || ext == "jpeg") {
        return "image/jpeg";
    }
    if (ext == "svg") {
        return "image/svg+xml";
    }
    if (ext == "ico") {
        return "image/x-icon";
    }
    return "application/octet-stream";
}

HttpServer::HttpServer(int port) : port_(port)
{
    staticRootPath_ = getExeDir() + "/../config/static";
}

HttpServer::~HttpServer()
{
    stop();
}

std::string HttpServer::getExeDir()
{
#ifdef _WIN32
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    // Find last backslash to get directory
    char* lastSlash = strrchr(buf, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
    }
    return std::string(buf);
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, PATH_MAX);
    if (len > 0) {
        buf[len] = '\0';
        char* lastSlash = strrchr(buf, '/');
        if (lastSlash) {
            *lastSlash = '\0';
        }
        return std::string(buf);
    }
    // Fallback: use current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX)) {
        return std::string(cwd);
    }
    return ".";
#endif
}

bool HttpServer::isPortInUse(int port)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // Note: Do NOT set SO_REUSEADDR here. On Windows, SO_REUSEADDR allows
    // multiple sockets to bind the same port, making this test unreliable.
    // Without it, bind() correctly detects ports already in use on all platforms.

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    int result = bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return result != 0;
}

void HttpServer::start()
{
    if (running_) {
        return;
    }
    running_ = true;

    auto svr = std::make_unique<httplib::Server>();
    serverPtr_ = svr.get();
    auto ctx = context_;

    // API routes
    svr->Get("/api/snapshot", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleSnapshot(*ctx), "application/json");
    });

    svr->Get("/api/dominance/tree", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleDominanceTree(*ctx), "application/json");
    });

    svr->Get("/api/dominance/tree-by-type", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleDominanceTreeByType(*ctx), "application/json");
    });

    svr->Get("/api/dominance/children", [ctx](const httplib::Request& req, httplib::Response& res) {
        uint64_t parentId = 0;
        auto it = req.params.find("parent_id");
        if (it != req.params.end()) {
            try {
                parentId = std::stoull(it->second);
            } catch (const std::exception&) {
                res.status = HTTP_STATUS_BAD_REQUEST;
                res.set_content("{\"error\":\"Invalid parent_id\"}", "application/json");
                return;
            }
        }
        res.set_content(HttpHandlers::handleDominanceChildren(*ctx, parentId), "application/json");
    });

    svr->Get("/api/dominance/children-by-type", [ctx](const httplib::Request& req, httplib::Response& res) {
        std::string nodeId = "";
        auto it = req.params.find("node_id");
        if (it != req.params.end()) {
            nodeId = it->second;
        }
        res.set_content(HttpHandlers::handleDominanceChildrenByType(*ctx, nodeId), "application/json");
    });

    svr->Get("/api/dominance/top10", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleDominanceTop10(*ctx), "application/json");
    });

    svr->Get("/api/dominance/cluster-expand", [ctx](const httplib::Request& req, httplib::Response& res) {
        std::vector<uint64_t> instanceIds;
        auto it = req.params.find("instance_ids");
        if (it != req.params.end()) {
            // Parse comma-separated instance IDs
            std::string idsStr = it->second;
            std::stringstream ss(idsStr);
            std::string idStr;
            while (std::getline(ss, idStr, ',')) {
                try {
                    instanceIds.push_back(std::stoull(idStr));
                } catch (...) {
                    // Skip invalid IDs
                }
            }
        }
        res.set_content(HttpHandlers::handleDominanceClusterExpand(*ctx, instanceIds), "application/json");
    });

    svr->Get("/api/fragment/overview", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleFragmentOverview(*ctx), "application/json");
    });

    svr->Get("/api/fragment/layout", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleFragmentLayout(*ctx), "application/json");
    });

    svr->Get("/api/fragment/summary", [ctx](const httplib::Request&, httplib::Response& res) {
        res.set_content(HttpHandlers::handleFragmentSummary(*ctx), "application/json");
    });

    // Static files under /static/
    svr->Get(R"(/static/(.+))", [staticRoot = staticRootPath_](const httplib::Request& req, httplib::Response& res) {
        std::string requested = req.matches[1].str();
        if (requested.find("..") != std::string::npos) {
            res.status = HTTP_STATUS_BAD_REQUEST;
            res.set_content("Bad Request", "text/plain");
            return;
        }
        std::string filepath = staticRoot + "/" + requested;
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            res.status = HTTP_STATUS_NOT_FOUND;
            res.set_content("Not Found", "text/plain");
            return;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.set_content(content, guessMimeType(filepath));
    });

    // Root -> serve index.html directly
    svr->Get("/", [staticRoot = staticRootPath_](const httplib::Request&, httplib::Response& res) {
        std::string filepath = staticRoot + "/html/index.html";
        std::ifstream file(filepath, std::ios::binary);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = HTTP_STATUS_NOT_FOUND;
            res.set_content("Not Found", "text/plain");
        }
    });

    serverThread_ = std::thread([this, svr = std::move(svr)]() mutable {
        LOG_DEBUG("HTTP server starting on port {}", port_);
        if (!svr->listen("127.0.0.1", port_)) {
            LOG_ERROR("Failed to start HTTP server on port {}", port_);
        }
    });
}

void HttpServer::stop()
{
    if (!running_) {
        return;
    }
    running_ = false;
    if (serverPtr_) {
        static_cast<httplib::Server*>(serverPtr_)->stop();
    }
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    serverPtr_ = nullptr;
}

void HttpServer::setContext(std::shared_ptr<HttpContext> ctx)
{
    context_ = ctx;
}

} // namespace cjprof