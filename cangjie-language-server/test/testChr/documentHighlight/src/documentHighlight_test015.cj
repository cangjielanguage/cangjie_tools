// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

abstract class BaseAbs {
	func getA() : int32
	static func getB() : int32 {
		return 2
	}
	func getC() : int32 {
		return 100
	}
}

class DataAbs <: BaseAbs {
	public func getA() : int32 {
		return getB() + 1
	}
}

func run_abs(): int32 {
	var value : DataAbs = DataAbs()
	if  value.getA() != 3 {
		return 1
	}
	if  BaseAbs.getB() != 2 {
		return 2
	}
	var a = value.getC()
	return 0
}
