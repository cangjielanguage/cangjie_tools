// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

/**
 * TomlException class.
 */
public class TomlException <: Exception {
    /**
     * Create a new TomlException instance.
     *
     *
     */
    public init() {
        super()
    }

    /**
     * Create a new TomlException instance.
     * Construction with parameters.
     *
     * @param message exception message.
     *
     *
     */
    public init(message: String) {
        super(message)
    }

    /**
     * Get class name.
     *
     * @return class name.
     *
     *
     */
    protected override func getClassName(): String {
        return "TomlException"
    }
}
