// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJPROF_HTTP_SERVER_H
#define CJPROF_HTTP_SERVER_H

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "Analyzer/HttpContext.h"

namespace cjprof {

class HttpServer {
public:
    HttpServer(int port);
    ~HttpServer();

    void start();
    void stop();

    void setContext(std::shared_ptr<HttpContext> ctx);

    static bool isPortInUse(int port);

private:
    int port_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
    std::shared_ptr<HttpContext> context_;
    void* serverPtr_ = nullptr; // httplib::Server*
};

} // namespace cjprof

#endif // CJPROF_HTTP_SERVER_H
