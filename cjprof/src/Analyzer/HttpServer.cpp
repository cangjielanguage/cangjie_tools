#include "Analyzer/HttpServer.h"
#include "Analyzer/HttpHandlers.h"
#include "Analyzer/Logger.h"
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cstring>

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

namespace cjprof {

HttpServer::HttpServer(int port) : port_(port) {
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::isPortInUse(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SOCKET_INVALID) {
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bool inUse = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR);
    SOCKET_CLOSE(sock);
#ifdef _WIN32
    WSACleanup();
#endif

    return inUse;
}

bool HttpServer::setupSocket() {
#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_ERROR("Failed to initialize Winsock");
        return false;
    }
#endif

    // Create socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ == SOCKET_INVALID) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // Allow socket reuse
    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    // Bind
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
#ifdef _WIN32
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
#else
    inet_aton("127.0.0.1", &serverAddr.sin_addr);
#endif

    if (bind(serverSocket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_ERROR("Failed to bind socket");
        cleanupSocket();
        return false;
    }

    // Listen
    if (listen(serverSocket_, SOMAXCONN) < 0) {
        LOG_ERROR("Failed to listen");
        cleanupSocket();
        return false;
    }

    return true;
}

void HttpServer::cleanupSocket() {
    if (serverSocket_ != SOCKET_INVALID) {
        SOCKET_CLOSE(serverSocket_);
        serverSocket_ = SOCKET_INVALID;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

std::string HttpServer::routeRequest(const std::string& path, const std::string& query) {
    // API endpoints
    if (path == "/api/snapshot") {
        return HttpHandlers::handleSnapshot(*context_);
    }
    if (path == "/api/dominance/tree") {
        return HttpHandlers::handleDominanceTree(*context_);
    }
    if (path == "/api/dominance/children") {
        // Section 9.3: Incremental loading - parse parent_id param
        uint64_t parentId = 0;
        if (!query.empty()) {
            size_t pos = query.find("parent_id=");
            if (pos != std::string::npos) {
                parentId = std::stoull(query.substr(pos + 10));
            }
        }
        return HttpHandlers::handleDominanceChildren(*context_, parentId);
    }
    if (path == "/api/dominance/top10") {
        return HttpHandlers::handleDominanceTop10(*context_);
    }
    if (path == "/api/fragment/overview") {
        return HttpHandlers::handleFragmentOverview(*context_);
    }
    if (path == "/api/fragment/layout") {
        return HttpHandlers::handleFragmentLayout(*context_);
    }
    if (path == "/api/fragment/summary") {
        return HttpHandlers::handleFragmentSummary(*context_);
    }

    // Root redirect to index.html
    if (path == "/" || path == "/index.html") {
        std::ifstream file("static/html/index.html");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
        return "<html><body><h1>cjprof</h1><p>Open /static/html/index.html</p></body></html>";
    }

    // Static files
    if (path.find("/static/") == 0) {
        std::string filepath = path.substr(1);  // Remove leading /
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
        return "404 Not Found";
    }

    return "404 Not Found";
}

void HttpServer::start() {
    running_ = true;

    if (!setupSocket()) {
        return;
    }

    serverThread_ = std::thread([this]() {
        while (running_) {
            // Accept connection
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            socket_t clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);

            if (clientSocket == SOCKET_INVALID) {
                if (running_) {
                    LOG_ERROR("Failed to accept connection");
                }
                continue;
            }

            // Receive request
            char buffer[8192];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';

                // Parse HTTP request
                std::string request(buffer);
                std::istringstream iss(request);
                std::string method, path, version;
                iss >> method >> path >> version;

                // Extract query string
                std::string query;
                size_t queryPos = path.find('?');
                if (queryPos != std::string::npos) {
                    query = path.substr(queryPos + 1);
                    path = path.substr(0, queryPos);
                }

                // Route request
                std::string responseBody = routeRequest(path, query);

                // Determine Content-Type based on path
                std::string contentType = "application/json";
                if (path == "/" || path == "/index.html" || path.find(".html") != std::string::npos) {
                    contentType = "text/html";
                } else if (path.find(".js") != std::string::npos) {
                    contentType = "application/javascript";
                } else if (path.find(".css") != std::string::npos) {
                    contentType = "text/css";
                }

                // Build HTTP response
                std::ostringstream response;
                response << "HTTP/1.1 200 OK\r\n";
                response << "Content-Type: " << contentType << "\r\n";
                response << "Access-Control-Allow-Origin: *\r\n";
                response << "Content-Length: " << responseBody.size() << "\r\n";
                response << "\r\n";
                response << responseBody;

                send(clientSocket, response.str().c_str(), response.str().length(), 0);
            }

            SOCKET_CLOSE(clientSocket);
        }

        cleanupSocket();
    });

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void HttpServer::stop() {
    running_ = false;
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

void HttpServer::setContext(std::shared_ptr<HttpContext> ctx) {
    context_ = ctx;
}

} // namespace cjprof
