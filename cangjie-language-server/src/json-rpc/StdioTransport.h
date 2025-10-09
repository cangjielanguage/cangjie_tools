// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_STDIOTRANSPORT_H
#define LSPSERVER_STDIOTRANSPORT_H

#include <iostream>
#include <sstream>

#include "Transport.h"

namespace ark {
constexpr int MAX_MESSAGE_LENGTH = 30;

class StdioTransport : public Transport {
public:
    static StdioTransport& Instance()
    {
        static StdioTransport instance {};
        return instance;
    }

    void SetIO(std::FILE *in, std::FILE *out) override ;

    void Notify(std::string method, ValueOrError params) override ;

    void Reply(nlohmann::json id, ValueOrError result) override ;

    LSPRet Loop(MessageHandler &handler) override ;

    std::mutex stdoutMutex {};

    ~StdioTransport() override {}
private:
    StdioTransport(): pFileIn(nullptr), pFileOut(nullptr)  {}
    StdioTransport(const StdioTransport &);
    const StdioTransport &operator=(const StdioTransport &);

    LSPRet HandleMessage(nlohmann::json message, MessageHandler &handler);

    void SendMsg(const nlohmann::json &message);

    std::string ReadRawMessage();

    std::string ReadStandardMessage();

    std::FILE *pFileIn = nullptr;
    std::FILE *pFileOut = nullptr;
};
}
#endif // LSPSERVER_STDIOTRANSPORT_H
