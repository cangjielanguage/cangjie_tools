// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


var empty = 1

//////////////////////////////////////////////////////////////////////////////////////////////

// LSP_Hover_Comment_Line_001.test_above
var LSP_Hover_Comment_Line_001 = 1    // LSP_Hover_Comment_Line_001.test_back
// LSP_Hover_Comment_Line_002.test_above1
// LSP_Hover_Comment_Line_002.test_above2
var LSP_Hover_Comment_Line_002 = 1  // LSP_Hover_Comment_Line_002.test_back

//////////////////////////////////////////////////////////////////////////////////////////////

/* LSP_Hover_Comment_Block_001.test_above */
var LSP_Hover_Comment_Block_001 = 1   /* LSP_Hover_Comment_Block_001.test_back */
/* LSP_Hover_Comment_Block_001.test_above1 */
/*
*
*LSP_Hover_Comment_Block_001.test_above2 */
var LSP_Hover_Comment_Block_002 = 1   /* LSP_Hover_Comment_Block_001.test_back */

//////////////////////////////////////////////////////////////////////////////////////////////

// LSP_Hover_Comment_Line_Block_001_001.test_above1
/*
*
*LSP_Hover_Comment_Line_Block_001.test_above2 */
var LSP_Hover_Comment_Line_Block_001 = 1   // LSP_Hover_Comment_Line_Block_001_001.test_back
// LSP_Hover_Comment_Line_Block_001_001.test_above1

/*
*
*LSP_Hover_Comment_Line_Block_001.test_above2 */
var LSP_Hover_Comment_Line_Block_002 = 1   // LSP_Hover_Comment_Line_Block_001_001.test_back

//////////////////////////////////////////////////////////////////////////////////////////////

/**
 * LSP_Hover_Comment_Doc_001.test_above2
 * @brief  LSP_Hover_Comment_Doc_001.test_above1
 * @param param1 short parameter
 * @param param2 long parameter
 * @return Int64
 */
func LSP_Hover_Comment_Doc_001(param1: Int64, param2: Int64): Int64 { 0 }

//////////////////////////////////////////////////////////////////////////////////////////////

/**
 * LSP_Hover_Comment_Doc_001.test_above2
 * @brief  LSP_Hover_Comment_Doc_001.test_above1
 * @param param1 short parameter
 * @param param2222222222222222 long parameter
 * @return Int64
 */
 // LSP_Hover_Comment_Line_Block_Doc_001.test_above1

 /*
 *
 *LSP_Hover_Comment_Line_Block_Doc_001.test_above2 */
func LSP_Hover_Comment_Line_Block_Doc_001(param1: Int64, param2222222222222222: Int64): Int64 { 0 }
