#ifndef CJPROF_HTTP_SERVER_H
#define CJPROF_HTTP_SERVER_H

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "Analyzer/HttpContext.h"

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    typedef SOCKET socket_t;
    #define SOCKET_CLOSE closesocket
    #define SOCKET_INVALID INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int socket_t;
    #define SOCKET_CLOSE close
    #define SOCKET_INVALID -1
#endif

namespace cjprof {

class HttpServer {
public:
    HttpServer(int port);
    ~HttpServer();

    void start();
    void stop();

    void setContext(std::shared_ptr<HttpContext> ctx);

    // Check if a port is in use
    static bool isPortInUse(int port);

private:
    int port_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
    std::shared_ptr<HttpContext> context_;
    socket_t serverSocket_{SOCKET_INVALID};

    // Request routing
    std::string routeRequest(const std::string& path, const std::string& query);

    // Platform-specific setup
    bool setupSocket();
    void cleanupSocket();
};

} // namespace cjprof

#endif // CJPROF_HTTP_SERVER_H
