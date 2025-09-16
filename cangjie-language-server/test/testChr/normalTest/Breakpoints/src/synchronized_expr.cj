// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

from std import sync.*

func A() {
    let m: MultiConditionMonitor = MultiConditionMonitor()
    synchronized (m) {
        if (true) {
            synchronized (m) {
                if (true) {
                    1;
                } else {
                    synchronized (m) {
                        if (true) {
                            1;
                        } else {
                            2;
                        }
                    }
                }
            }
        } else {
            2;
        }
    }
}
