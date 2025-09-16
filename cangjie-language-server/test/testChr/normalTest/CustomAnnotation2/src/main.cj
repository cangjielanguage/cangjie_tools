// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package CustomAnnotation2

import std.unittest.testmacro.Test

@Annotation
public class CustomAnnotation {
   let name: String
   let version: Int64

   public const init(name: String, version!: Int64 = 0) {
       this.name = name
       this.version = version
   }
}

@!CustomAnnotation["Another sample", version: 1]
func foo(
   @!CustomAnnotation["Another sample", version: 1]
   a: String,
   @!CustomAnnotation["Another sample", version: 1]
   b: Int32,
   @!CustomAnnotation["Another sample", version: 1]
   c: String
):Unit {
   var x: Int32 = 10 + 20
   x = x + 1
}

class A {
   @!CustomAnnotation["Another sample", version: 1]
   A(
       @!CustomAnnotation["Another sample", version: 1]
   public let x: String,
       @!CustomAnnotation["Another sample", version: 1]
   public let xx: String){
   }

   @CustomAnnotation["Another sample", version: 1]
   public init(
       @!CustomAnnotation["Another sample", version: 1]
       y: String,
       @!CustomAnnotation["Another sample", version: 1]
       yy: String,
       @!CustomAnnotation["Another sample", version: 1]
       z: String) {
       x = y;
       xx = yy;
   }
   @!CustomAnnotation["Another sample", version: 1]
   public prop z: Int32 {
       get() {
           1
       }
   }
}


class B {
   @!CustomAnnotation["Another sample", version: 1]
   public init() {
       0
       0
       "0123"
   }
}
