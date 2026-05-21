#include "Analyzer/HttpServer.h"
#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Logger.h"
#include <httplib.h>
#include <fstream>
#include <sstream>

namespace cjprof {

static std::string guessMimeType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".json") != std::string::npos) return "application/json";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return "image/jpeg";
    return "application/octet-stream";
}

HttpServer::HttpServer(int port) : port_(port) {}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::isPortInUse(int port) {
    httplib::Client cli("127.0.0.1", port);
    cli.set_connection_timeout(0, 100 * 1000); // 100ms
    auto res = cli.Get("/");
    return res != nullptr;
}

void HttpServer::start() {
    if (running_) return;
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
            parentId = std::stoull(it->second);
        }
        res.set_content(HttpHandlers::handleDominanceChildren(*ctx, parentId), "application/json");
    });

    svr->Get("/api/dominance/children-by-type", [ctx](const httplib::Request& req, httplib::Response& res) {
        std::string parentType = "";
        auto it = req.params.find("parent_type");
        if (it != req.params.end()) {
            parentType = it->second;
        }
        res.set_content(HttpHandlers::handleDominanceChildrenByType(*ctx, parentType), "application/json");
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
    svr->Get(R"(/static/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string filepath = "static/" + req.matches[1].str();
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
            return;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        res.set_content(content, guessMimeType(filepath));
    });

    // Root -> serve index.html directly
    svr->Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("static/html/index.html", std::ios::binary);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
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

void HttpServer::stop() {
    if (!running_) return;
    running_ = false;
    if (serverPtr_) {
        static_cast<httplib::Server*>(serverPtr_)->stop();
    }
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    serverPtr_ = nullptr;
}

void HttpServer::setContext(std::shared_ptr<HttpContext> ctx) {
    context_ = ctx;
}

} // namespace cjprof
