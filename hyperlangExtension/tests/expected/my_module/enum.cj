// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/***********ENUM***********/
// 字符串枚举
/* enum Colors {
	 Red = RED,
	 Green = GREEN,
	 Blue = BLUE,
	 Yellow = 'YELLOW'
	}*/
public enum Colors <: ToString & Equatable < Colors > {
    | Red
    | Green
    | Blue
    | Yellow 

    func get(): String {
        match(this) {
            case Red => "RED" 
            case Green => "GREEN" 
            case Blue => "BLUE" 
            case Yellow => "YELLOW"  //todo: please check the value
        }
    }
    
    static func parse(val: String): Colors {
        match(val) {
            case "RED" => Red 
            case "GREEN" => Green 
            case "BLUE" => Blue 
            case "YELLOW" => Yellow  //todo: please check the value
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?Colors {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: Colors): Bool {
        match((this, that)) {
            case(Red, Red) => true
            case(Green, Green) => true
            case(Blue, Blue) => true
            case(Yellow, Yellow) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: Colors): Bool {
        !(this == that)
    }
}

// 数字枚举
/* enum Status {
	 Pending,
	 Approved,
	 Rejected
	}*/
public enum Status <: ToString & Equatable < Status > {
    | Pending
    | Approved
    | Rejected 

    func get(): Int32 {
        match(this) {
            case Pending => 0  //todo: please check the value
            case Approved => 1  //todo: please check the value
            case Rejected => 2  //todo: please check the value
        }
    }
    
    static func parse(val: Int32): Status {
        match(val) {
            case 0 => Pending  //todo: please check the value
            case 1 => Approved  //todo: please check the value
            case 2 => Rejected  //todo: please check the value
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?Status {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case Pending => "Pending"
            case Approved => "Approved"
            case Rejected => "Rejected"
        }
    }
    
    public override operator func ==(that: Status): Bool {
        match((this, that)) {
            case(Pending, Pending) => true
            case(Approved, Approved) => true
            case(Rejected, Rejected) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: Status): Bool {
        !(this == that)
    }
}

// 常量枚举
// constants.d.ts
/* enum Status1 {
	 Pending = 3,
	 Approved = 4,
	 Rejected = 5
	}*/
public enum Status1 <: ToString & Equatable < Status1 > {
    | Pending
    | Approved
    | Rejected 

    func get(): Int32 {
        match(this) {
            case Pending => 3 
            case Approved => 4 
            case Rejected => 5 
        }
    }
    
    static func parse(val: Int32): Status1 {
        match(val) {
            case 3 => Pending 
            case 4 => Approved 
            case 5 => Rejected 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?Status1 {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case Pending => "Pending"
            case Approved => "Approved"
            case Rejected => "Rejected"
        }
    }
    
    public override operator func ==(that: Status1): Bool {
        match((this, that)) {
            case(Pending, Pending) => true
            case(Approved, Approved) => true
            case(Rejected, Rejected) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: Status1): Bool {
        !(this == that)
    }
}

// 异构枚举
// response.d.ts
/* enum Response1 {
	 No = 0,
	 Yes = YES
	}*/
public enum Response1 <: ToString & Equatable < Response1 > {
    | No
    | Yes 

    func get(): String {
        match(this) {
            case No => "0" 
            case Yes => "YES" 
        }
    }
    
    static func parse(val: String): Response1 {
        match(val) {
            case "0" => No 
            case "YES" => Yes 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?Response1 {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: Response1): Bool {
        match((this, that)) {
            case(No, No) => true
            case(Yes, Yes) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: Response1): Bool {
        !(this == that)
    }
}

// this is systemapi
/*// @systemapi
/* enum SystemErrorCode {
	 success = 0,
	 invalidInput = 1,
	 networkError = 2,
	 internalError = 3
	}*/
public enum SystemErrorCode <: ToString & Equatable < SystemErrorCode > {
    | success
    | invalidInput
    | networkError
    | internalError 

    func get(): Int32 {
        match(this) {
            case success => 0 
            case invalidInput => 1 
            case networkError => 2 
            case internalError => 3 
        }
    }
    
    static func parse(val: Int32): SystemErrorCode {
        match(val) {
            case 0 => success 
            case 1 => invalidInput 
            case 2 => networkError 
            case 3 => internalError 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?SystemErrorCode {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case success => "success"
            case invalidInput => "invalidInput"
            case networkError => "networkError"
            case internalError => "internalError"
        }
    }
    
    public override operator func ==(that: SystemErrorCode): Bool {
        match((this, that)) {
            case(success, success) => true
            case(invalidInput, invalidInput) => true
            case(networkError, networkError) => true
            case(internalError, internalError) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: SystemErrorCode): Bool {
        !(this == that)
    }
}
*/

// this api is deprecated
/*// @deprecated
/* enum LegacyStatus {
	 active = 0,
	 inactive = 1,
	 pending = 2
	}*/
public enum LegacyStatus <: ToString & Equatable < LegacyStatus > {
    | active
    | inactive
    | pending 

    func get(): Int32 {
        match(this) {
            case active => 0 
            case inactive => 1 
            case pending => 2 
        }
    }
    
    static func parse(val: Int32): LegacyStatus {
        match(val) {
            case 0 => active 
            case 1 => inactive 
            case 2 => pending 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?LegacyStatus {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case active => "active"
            case inactive => "inactive"
            case pending => "pending"
        }
    }
    
    public override operator func ==(that: LegacyStatus): Bool {
        match((this, that)) {
            case(active, active) => true
            case(inactive, inactive) => true
            case(pending, pending) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: LegacyStatus): Bool {
        !(this == that)
    }
}
*/


