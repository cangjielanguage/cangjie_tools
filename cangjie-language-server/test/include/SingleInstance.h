// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include<string>
#include <map>

class SingleInstance {
public:
    std::string pathIn = "";
    std::string pathOut = "";
    std::string workPath = "";
    std::string pathPwd = "";
    std::string messagePath = "";
    std::string testFolder = "";
    std::string pathBuildScript = "";
    std::string caseProPath = "";
    std::string binaryPath = "";
    bool useDB = false;
    std::map<std::string, std::string> replaceMap = {};
    static SingleInstance* GetInstance()
    {
        if (m_pInstance == nullptr) {
            m_pInstance = new SingleInstance();
        }
        return m_pInstance;
    }
private:
    SingleInstance() {};
    static SingleInstance *m_pInstance;
    class CGarbo {
    public:
        ~CGarbo()
        {
            if (SingleInstance::m_pInstance) {
                delete SingleInstance::m_pInstance;
            }
        }
    };
    static CGarbo Garbo;
};
