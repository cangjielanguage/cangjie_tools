// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public class LinkedListNode {
    var val: Int64
    var next: ?LinkedListNode
    init() {
        val = 0
        next = None
    }
    init(val: Int64) {
        this.val = val
        next = None
    }
}

class Solution2 {
    func addTwoNumbers(l1: ?LinkedListNode, l2: ?LinkedListNode): ?LinkedListNode {
        return add(l1, l2, 0)
    }
}

//  对两个倒序的链表中的数进行相加操作，返回一个倒序的链表
func add(l1: ?LinkedListNode, l2: ?LinkedListNode, carry: Int64): ?LinkedListNode {
    //  匹配两个链表，获取相加值与下个节点的地址
    var (num, n1, n2) = match ((l1, l2)) {
        case (Some(v1), Some(v2)) =>
            (v1.val + v2.val + carry, v1.next, v2.next)
        case (Some(v), None) =>
            (v.val + carry, v.next, None<LinkedListNode>)
        case (None, Some(v)) =>
            (v.val + carry, None<LinkedListNode>, v.next)
        case _ =>
            //  所有数字都相加完成后如果需要进位，则进位
            if (carry > 0) {
                return LinkedListNode(1)
            } else {
                return None
            }
    }

    //  相加结果的个位作为当前节点的值
    var node = LinkedListNode(num % 10)
    //  嵌套调用 add 函数，生成完整链表
    node.next = add(n1, n2, num / 10)
    return node
}
