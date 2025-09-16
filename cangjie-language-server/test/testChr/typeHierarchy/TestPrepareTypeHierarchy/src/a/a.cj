// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package b.a;
import b.f.*;
/*测试中文*/public interface People{}
/*测试中文*/public open class xzf <: ArithmeticException& People{
   public static var xiaomingname:String = "aaaa";
   public static func xzfage(){}
}

/*测试中文*/public struct work <: People{
    public static var workname:String = "aaaa";
   public static func workage(){}
}

/*测试中文*/public enum eat <: People{ 
    red | blue | yellow
}

/*测试中文*/public interface love <: People{}
/*测试中文*/public class fafaa <:ArithmeticException{}
/*测试中文*/extend testsub <: People{}
/*测试中文*/extend testsub1 <: People{
    func a(){}
}
 var aadaa:eat = eat.red
