// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入漏洞修复验证
// 覆盖路径：
//   - trans_object.cj generate() → escapeRawComment(arkComment)
//   - trans_object.cj genPropDeclare → escapeRawComment(arkProp.comment)
//   - trans_enum.cj generate() → escapeRawComment(comment)
// 攻击载荷：interface/enum/属性行注释中包含 */ 

// evil_interface_inject() */
export interface IObjectInjection {
    // evil_property_inject() */
    evilProperty: string;

    // evil_property2_inject() */
    anotherProperty: number;
}

// evil_enum_inject() */
declare enum EvilEnum {
    // evil_enum_member_inject() */
    EvilMember = 0,
    Normal = 1,
}
