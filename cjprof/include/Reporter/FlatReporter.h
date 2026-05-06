// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef FLAT_REPORTER_H
#define FLAT_REPORTER_H

#include "Utility/Singleton.h"
#include "Reporter.h"

class FlatReporter : public Reporter, public Singleton<FlatReporter> {
    friend Singleton<FlatReporter>;

public:
    void Report() override;

private:
    FlatReporter() = default;
};

#endif // FLAT_REPORTER_H