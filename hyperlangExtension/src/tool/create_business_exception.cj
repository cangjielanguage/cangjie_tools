// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.time.DateTime

public func createBusinessExceptioinKit(gconfig: GlobalConfig): String {
    let moduleName = gconfig.cjModuleName

    let content = """
// Copyright (c) Huawei Technologies Co., Ltd. ${DateTime.nowUTC().year}. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ${moduleName}

import std.collection.HashMap
import ohos.labels.APILevel

protected const ERR_NO_PERMISSION: Int32 = 201
protected const ERR_NOT_SYSTEM_APP: Int32 = 202
protected const ERR_PARAMETER_ERROR: Int32 = 401
protected const ERR_NOT_SUPPOERTED: Int32 = 801
protected let UNIVERSAL_ERROR_MAP = HashMap<Int32, String>(
    [
        (ERR_NO_PERMISSION,
            "Permission verification failed. The application does not have the permission required to call the API."),
        (ERR_NOT_SYSTEM_APP, "Permission verification failed. A non-system application calls a system API."),
        (ERR_PARAMETER_ERROR, "Parameter error. Parameter verification failed."),
        (ERR_NOT_SUPPOERTED, "Capability not supported. Failed to call the API due to limited device capabilities.")
    ]
)

protected func getUniversalErrorMsg(code: Int32): ?String {
    UNIVERSAL_ERROR_MAP.get(code)
}

/**
 * Defines the exception interface.
 */
@!APILevel[
    since: "22"
]
public class BusinessException <: Exception {
    /**
     * Defines the basic error code.
     */
    @!APILevel[
        since: "22"
    ]
    public let code: Int32

    private var errData: ?Any

    /**
     * Defines the additional information for business.
     *
     * @returns { ?T } Return the business value.
     */
    @!APILevel[
        since: "22"
    ]
    public func getData<T>(): ?T {
        if (let Some(v) <- errData) {
            return v as T
        } else {
            return None
        }
    }

    protected init(code: Int32, msg: String, data!: ?Any = None) {
        super(msg)
        this.code = code
        this.errData = data
    }

    protected override func getClassName(): String {
        "BusinessException"
    }

    /**
     * Translate the exception to String.
     *
     * @returns { String } String representation of the exception.
     */
    @!APILevel[
        since: "22"
    ]
    public func toString(): String {
        let className: String = getClassName()
        let message: String = this.message
        if (message.isEmpty()) {
            return "\${className}:  errorcode: \${code}"
        }
        return "\${className}: errorcode: \${code}, message: \${message}"
    }
}

/**
 * Defines the basic async callback.
 */
public type AsyncCallback<T> = (Option<BusinessException>, Option<T>) -> Unit
"""
    return content
}
