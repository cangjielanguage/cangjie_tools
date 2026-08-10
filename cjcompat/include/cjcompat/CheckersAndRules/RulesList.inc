// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifdef RULEKIND
// RULEKIND(ID, isAPI, isABI, description)
// Implemented in VarDeleted
RULEKIND(VAR_DELETED, true, true, "Variable declaration visible outside the module deleted or made private.")
RULEKIND(VAR_LOCATION_CHANGED, false, true,
    "Changed location of private variable declaration visible outside the module.")
// Implemented in FuncDeleted
RULEKIND(FUNC_DELETED, true, true, "Function declaration visible outside the module deleted or made private.")
RULEKIND(FUNC_LOCATION_CHANGED, false, true,
    "Changed location of private function declaration visible outside the module.")
// Implemented in StructDeleted
RULEKIND(STRUCT_VAR_MODIFIER_STATIC_DELETED, true, true,
    "Deleted the modifier 'static' of the member var of the struct.")
RULEKIND(STRUCT_DELETED, true, true, "Struct declaration visible outside the module deleted or made private.")
RULEKIND(STRUCT_LOCATION_CHANGED, false, true,
    "Changed location of private struct declaration visible outside the module.")
// Implemented in EnumDeleted
RULEKIND(ENUM_DELETED, true, true, "Enum declaration visible outside the module deleted or made private.")
RULEKIND(ENUM_LOCATION_CHANGED, false, true,
    "Changed location of private enum declaration visible outside the module.")
// Implemented in ClassDeleted
RULEKIND(CLASS_DELETED, true, true, "Class declaration visible outside the module deleted or made private.")
RULEKIND(CLASS_LOCATION_CHANGED, false, true,
    "Changed location of private class declaration visible outside the module.")
// Implemented in InstanceDeleted
RULEKIND(INTERFACE_DELETED, true, true, "Interface declaration visible outside the module deleted or made private.")
RULEKIND(INTERFACE_LOCATION_CHANGED, false, true,
    "Changed location of private interface declaration visible outside the module.")
// Implemented in ExtendDeleted
RULEKIND(EXTEND_DELETED, true, true, "Extend declaration visible outside the module deleted or made private.")
// Implemented in FuncParametersChanged
RULEKIND(FUNC_NUM_PARAMS, true, true, "Changed number of parameters of function visible outside the module.")
RULEKIND(FUNC_PARAMS_TYPE, true, true, "Changed parameter type of function visible outside the module.")
// Implemented in FuncReturnType
RULEKIND(
    FUNC_RETURN_TYPE_SUBTYPE, true, true, "Changed return type of function visible outside the module. The new return \
type is not a subtype of the old return type.")
RULEKIND(FUNC_RETURN_TYPE_CLASS_LIKE, false, true,
    "Changed return type of function visible outside the module. The two types are compatible only when both are \
classes or interfaces types.")
RULEKIND(FUNC_RETURN_TYPE_OPEN, true, true, "Changed return type of open function visible outside the module.")
// Implemented in ConstOrFrozenBodyChange
RULEKIND(CONST_OR_FROZEN_BODY_CHANGE, false, true,
    "The body of a const function or function marked with @Frozen changed.")
// Implemented in FuncParametersChanged
RULEKIND(FUNC_CHANGE_NAME_OF_NAMED_PARAMETER, true, true, "Cannot change name of named parameter.")
RULEKIND(FUNC_PARAMETER_NAMED_TO_UNNAMED, true, true, "Named parameter cannot be made unnamed parameter.")
RULEKIND(FUNC_PARAMETER_UNNAMED_TO_NAMED, true, true, "Unnamed parameter cannot be made named parameter.")
RULEKIND(FUNC_DROP_DEFAUT_VALUE_OF_NAMED_PARAMETER, true, true, "Deleted default value of named parameter.")
RULEKIND(FUNC_MEMBER_PARAMETER_VALUE_ADD_OF_CONST_OR_FROZEN, false, true,
    "Added member parameter default value of a const function or function marked with @Frozen.")
RULEKIND(FUNC_PARAMETER_VALUE_CHANGE_OF_CONST_OR_FROZEN, false, true,
    "Changed parameter default value of a const function or function marked with @Frozen.")
RULEKIND(FUNC_CHANGE_TYPE_OF_PARAMETER_NOT_PARENT_TYPE, true, true,
    "Changed parameter type of function visible outside the module. \
The new param type is not a parent type of the old param type.")
RULEKIND(FUNC_CHANGE_TYPE_OF_PARAMETER_OPEN, true, true,
    "Changed parameter type of open function visible outside the module.")
RULEKIND(FUNC_CHANGE_TYPE_OF_PARAMETER, false, true, "Changed parameter type of function visible outside the module.")
RULEKIND(FUNC_CHANGE_TYPE_OF_MEMBER_PARAMETER, true, true,
    "Changed parameter type of constructor visible outside the module.")
RULEKIND(FUNC_CHANGE_ORDER_OF_UNNAMED_PARAMETER, true, true,
    "Changed unnamed parameter order of function visible outside the module.")
RULEKIND(FUNC_CHANGE_ORDER_OF_NAMED_PARAMETER, false, true,
    "Changed named parameter order of function visible outside the module.")
RULEKIND(FUNC_PARAM_ADDED_ABI, false, true, "Added param with default value to the end of constructor.")

// Implemented in VarDepreAnno
RULEKIND(VAR_DEPRE_ANNO_MOD, true, true, "Modified variable deprecated annotation with compilation error.")
RULEKIND(VAR_DEPRE_ANNO_ADD, true, true, "Added variable deprecated annotation with compilation error.")
// Implemented in StructVarDepreAnno
RULEKIND(STRUCT_VAR_DEPRE_ANNO_MOD, true, true,
    "Struct member modified variable deprecated annotation with compilation error.")
RULEKIND(STRUCT_VAR_DEPRE_ANNO_ADD, true, true,
    "Struct member added variable deprecated annotation with compilation error.")
// Implemented in VarGlobalType
RULEKIND(VAR_GLOBAL_TYPE_MOD, true, true, "Modified global variable type. The new type is not a subtype of the old.")
RULEKIND(VAR_GLOBAL_TYPE_MOD_CLASS_LIKE, false, true,
    "Modified global variable type. The two types are compatible only when both are classes or interfaces types.")
// Implemented in VarGlobalInit
RULEKIND(VAR_GLOBAL_INIT, false, true, "Modified global variable const initial value.")
// Implemented in FuncDepreAnno
RULEKIND(FUNC_DEPRE_ANNO_MOD, true, true, "Modified function deprecated annotation with compilation error.")
RULEKIND(FUNC_DEPRE_ANNO_ADD, true, true, "Added function deprecated annotation with compilation error.")
// Implemented in StructDepreAnno
RULEKIND(STRUCT_DEPRE_ANNO_MOD, true, true, "Modified struct deprecated annotation with compilation error.")
RULEKIND(STRUCT_DEPRE_ANNO_ADD, true, true, "Added struct deprecated annotation with compilation error.")
// Implemented in EnumDepreAnno
RULEKIND(ENUM_DEPRE_ANNO_MOD, true, true, "Modified enum deprecated annotation with compilation error.")
RULEKIND(ENUM_DEPRE_ANNO_ADD, true, true, "Added enum deprecated annotation with compilation error.")
// Implemented in ClassDepreAnno
RULEKIND(CLASS_DEPRE_ANNO_MOD, true, true, "Modified class deprecated annotation with compilation error.")
RULEKIND(CLASS_DEPRE_ANNO_ADD, true, true, "Added class deprecated annotation with compilation error.")
// Implemented in InterfaceDepreAnno
RULEKIND(INTERFACE_DEPRE_ANNO_MOD, true, true, "Modified interface deprecated annotation with compilation error.")
RULEKIND(INTERFACE_DEPRE_ANNO_ADD, true, true, "Added interface deprecated annotation with compilation error.")
// Implemented in FuncCAnno
RULEKIND(FUNC_C_ANNO_ADD, true, true, "Added function C attribute without unsafe compilation error.")
RULEKIND(FUNC_C_ANNO_DEL, false, true, "Remove function C attribute with compilation error.")
// Implemented in StructCAnno
RULEKIND(STRUCT_C_ANNO_ADD, false, true, "Added struct C attribute without unsafe compilation error.")
RULEKIND(STRUCT_C_ANNO_DEL, false, true, "Remove struct C attribute with compilation error.")
// Implemented in FuncCallingConvAnno
RULEKIND(FUNC_CALLINGCONV_ANNO, false, true,
    "Modified function Call Convention annotation without same argument convention compilation error.")
// Implemented in FuncFrozenAnno
RULEKIND(FUNC_FROZEN_ANNO, false, true, "Removed function Frozen annotation with compilation error.")

// Implemented in FuncGenericAddedDeletedOrChanged
RULEKIND(FUNC_GENERIC_PARAM_ADDED, true, true, "Added generic param of function visible outside the module.")
RULEKIND(FUNC_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of function visible outside the module.")
RULEKIND(FUNC_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of function visible outside the module.")
RULEKIND(FUNC_GENERIC_CONSTRAINT_ADDED, true, true, "Added generic constraint of function visible outside the module.")
RULEKIND(
    FUNC_GENERIC_CONSTRAINT_DELETED, false, true, "Deleted generic constraint of function visible outside the module.")
RULEKIND(
    FUNC_GENERIC_CONSTRAINT_CHANGED, false, true, "Changed generic constraint of function visible outside the module.")
RULEKIND(FUNC_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of function visible outside the module. \
The new type is not parent type of the old type.")
// Implemented in StructGenericAddedDeletedOrChanged
RULEKIND(STRUCT_GENERIC_PARAM_ADDED, true, true, "Added generic param of struct visible outside the module.")
RULEKIND(STRUCT_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of struct visible outside the module.")
RULEKIND(STRUCT_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of struct visible outside the module.")
RULEKIND(STRUCT_GENERIC_CONSTRAINT_ADDED, true, true, "Added generic constraint of struct visible outside the module.")
RULEKIND(
    STRUCT_GENERIC_CONSTRAINT_DELETED, false, true, "Deleted generic constraint of struct visible outside the module.")
RULEKIND(
    STRUCT_GENERIC_CONSTRAINT_CHANGED, false, true, "Changed generic constraint of struct visible outside the module.")
RULEKIND(STRUCT_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of struct visible outside the module. \
The new type is not parent type of the old type.")
// Implemented in EnumGenericAddedDeletedOrChanged
RULEKIND(ENUM_GENERIC_PARAM_ADDED, true, true, "Added generic param of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_CONSTRAINT_ADDED, true, true, "Added generic constraint of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_CONSTRAINT_DELETED, false, true,
    "Deleted generic constraint of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_CONSTRAINT_CHANGED, false, true,
    "Changed generic constraint of enum visible outside the module.")
RULEKIND(ENUM_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of enum visible outside the module. The new type is not parent type of the old type.")
// Implemented in ClassGenericAddedDeletedOrChanged
RULEKIND(CLASS_GENERIC_PARAM_ADDED, true, true, "Added generic param of class visible outside the module.")
RULEKIND(CLASS_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of class visible outside the module.")
RULEKIND(CLASS_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of class visible outside the module.")
RULEKIND(CLASS_GENERIC_CONSTRAINT_ADDED, true, true, "Added generic constraint of class visible outside the module.")
RULEKIND(
    CLASS_GENERIC_CONSTRAINT_DELETED, false, true, "Deleted generic constraint of class visible outside the module.")
RULEKIND(
    CLASS_GENERIC_CONSTRAINT_CHANGED, false, true, "Changed generic constraint of class visible outside the module.")
RULEKIND(CLASS_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of class visible outside the module. The new type is not parent type of the old type.")
// Implemented in InterfaceGenericAddedDeletedOrChanged
RULEKIND(INTERFACE_GENERIC_PARAM_ADDED, true, true, "Added generic param of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_CONSTRAINT_ADDED, true, true,
    "Added generic constraint of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_CONSTRAINT_DELETED, false, true,
    "Deleted generic constraint of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_CONSTRAINT_CHANGED, false, true,
    "Changed generic constraint of interface visible outside the module.")
RULEKIND(INTERFACE_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of interface visible outside the module. \
The new type is not parent type of the old type.")

// Implemented in InterfaceMemberFuncAddedDeletedOrChanged
RULEKIND(INTERFACE_MEMBER_FUNC_MODIFIER_CHANGED, true, true,
    "Changed member function's modifier of interface visible outside the module. \
public/const/static/mut deleted or unsafe/mut/static added.")
RULEKIND(INTERFACE_MEMBER_FUNC_DELETED, true, true, "Deleted member function of interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_ADDED, false, true, "Added member function of interface.")
RULEKIND(INTERFACE_INSTANCE_MEMBER_FUNC_ADDED, false, true,
    "Added overriding instance member function of interface.")
RULEKIND(INTERFACE_STATIC_MEMBER_FUNC_ADDED, false, true,
    "Added overriding static member function of interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_WITHOUT_DEFAULT_IMP_ADDED, true, true,
    "Added member function without a default implementation of interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_WITHOUT_DEFAULT_IMP_ADDED2, true, false,
    "Added member function without a default implementation of interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_MODIFIER_CONST_DELETED, true, true,
    "Deleted the modifier 'const' of the member function of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_MODIFIER_CONST_ADDED, false, true,
    "Added the modifier const of the member function of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_MODIFIER_UNSAFE_ADDED, true, true,
    "Added the modifier 'unsafe' of the member function of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_FROZEN_DELETED, false, true,
    "Deleted the annotation '@Frozen' of the member function of the interface.")
RULEKIND(INTERFACE_INSTANCE_MEMBER_FUNCS_ORDER_CHANGED, false, true,
    "Changed order of one or more member functions or properties in interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_DEPRECATED_CHANGED, true, true,
    "Changed the annotation '@Deprecated' of the member function of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_MODIFIER_MUT_ADDED, true, true,
    "Added the modifier 'mut' of the member function/property of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_DELETED, true, true,
    "Deleted the modifier 'static' of the member function/property of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_PROP_MODIFIER_STATIC_ADDED, true, true,
    "Added the modifier 'static' of the member function/property of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_PROP_MODIFIER_MUT_DELETED, true, true,
    "Deleted the modifier 'mut' of the member function/property of the interface.")
RULEKIND(INTERFACE_MEMBER_FUNC_RETURN_TYPE, true, true,
    "Changed return type of function visible outside the module.")

// Implemented in InterfaceMemberPropAddedDeletedOrChanged
RULEKIND(INTERFACE_MEMBER_PROP_DELETED, true, true,
    "Deleted public member property of interface visible outside the module.")
RULEKIND(INTERFACE_MEMBER_PROP_ADDED, false, true,
    "Added public member property of interface visible outside the module.")
RULEKIND(INTERFACE_INSTANCE_MEMBER_PROP_ADDED, false, true,
    "Added overriding instance member property of interface.")
RULEKIND(INTERFACE_STATIC_MEMBER_PROP_ADDED, false, true,
    "Added overriding static member property of interface.")
RULEKIND(INTERFACE_MEMBER_PROP_WITHOUT_DEFAULT_IMP_ADDED, true, true,
    "Added member property without a default implementation of interface.")
RULEKIND(INTERFACE_MEMBER_PROP_WITHOUT_DEFAULT_IMP_ADDED2, true, false,
    "Added member property without a default implementation of interface.")
RULEKIND(INTERFACE_MEMBER_PROP_DEPRECATED_CHANGED, true, true,
    "Changed the annotation '@Deprecated' of the member property of the interface visible outside the module.")
RULEKIND(INTERFACE_MEMBER_PROP_FROZEN_DELETED, false, true,
    "Deleted the annotation '@Frozen' of the member property of the interface visible outside the module.")
RULEKIND(INTERFACE_MEMBER_PROP_MODIFIER_MUT_ADDED, true, true,
    "Added the modifier 'mut' of the member function/property of the interface.")
RULEKIND(INTERFACE_MEMBER_PROP_RETURN_TYPE_CHANGED, true, true,
    "Changed return type of the member property of the interface.")

// Implemented in ExtendGenericAddedDeletedOrChanged
RULEKIND(EXTEND_GENERIC_PARAM_ADDED, true, true, "Added generic param of extend visible outside the module.")
RULEKIND(EXTEND_GENERIC_PARAM_DELETED, true, true, "Deleted generic param of extend visible outside the module.")
RULEKIND(EXTEND_GENERIC_PARAM_CHANGED, true, true, "Changed generic param of extend visible outside the module.")
RULEKIND(EXTEND_GENERIC_CONSTRAINT_ADDED, true, true, "Added generic constraint of extend visible outside the module.")
RULEKIND(
    EXTEND_GENERIC_CONSTRAINT_DELETED, false, true, "Deleted generic constraint of extend visible outside the module.")
RULEKIND(
    EXTEND_GENERIC_CONSTRAINT_CHANGED, false, true, "Changed generic constraint of extend visible outside the module.")
RULEKIND(EXTEND_GENERIC_CONSTRAINT_CHANGED_NOT_PARENT_TYPE, true, true,
    "Changed generic constraint of extend visible outside the module.")
// Implemented in ExtendMemberFuncDeletedOrChanged
RULEKIND(EXTEND_STATIC_MEMBER_FUNC_ADDED, false, true,
    "Added static member function with default implementation of extend.")
RULEKIND(EXTEND_STATIC_MEMBER_PROP_ADDED, false, true,
    "Added static property with default implementation of extend.")
RULEKIND(EXTEND_INSTANCE_MEMBER_FUNC_ADDED, false, true,
    "Added instance member function with default implementation of extend.")
RULEKIND(EXTEND_INSTANCE_MEMBER_PROP_ADDED, false, true,
    "Added instance property with default implementation of extend.")

RULEKIND(EXTEND_MEMBER_FUNC_DELETED1, true, true,
    "Deleted public/protected member function of extend or made it private when the extended type is class.")
RULEKIND(EXTEND_MEMBER_FUNC_DELETED2, true, true,
    "Deleted public member function of extend or made it private when the extended type is struct.")
RULEKIND(EXTEND_PRIVATE_MEMBER_FUNC_CHANGED, true, true, "Changed private member function of extend.")
RULEKIND(EXTEND_MEMBER_FUNC_VISIBILITY_CHANGED, true, true,
    "Changed visibility of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_PUBLIC_DELETED, true, true,
    "Delete the modifier 'public' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_CONST_DELETED, true, true,
    "Deleted the modifier 'const' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_CONST_ADDED, false, true,
    "Added the modifier const of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_STATIC_DELETED, true, true,
    "Deleted the modifier 'static' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_STATIC_ADDED, true, true,
    "Added the modifier 'static' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_MUT_ADDED, true, true,
    "Added the modifier 'mut' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_MUT_DELETED, false, true,
    "Deleted the modifier 'mut' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_MODIFIER_UNSAFE_ADDED, true, true,
    "Added the modifier 'unsafe' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_FROZEN_DELETED, false, true,
    "Deleted the annotation '@Frozen' of the member function of the extend.")
RULEKIND(EXTEND_MEMBER_FUNC_DEPRECATED_CHANGED, true, true,
    "Changed the annotation '@Deprecated' of the member function of the extend.")

// Implemented in ExtendMemberPropAddedDeletedOrChanged
RULEKIND(EXTEND_MEMBER_PROP_DELETED, true, true,
    "Extend member property visible outside the module deleted or made private.")
RULEKIND(EXTEND_MEMBER_PROP_ADDED, false, true,
    "Added public member property of extend visible outside the module.")
RULEKIND(EXTEND_MEMBER_PROP_VISIBILITY_CHANGED, true, true,
    "Changed visibility of the member property of the extend.")
RULEKIND(EXTEND_MEMBER_PROP_MODIFIER_STATIC_DELETED, true, true,
    "Deleted the modifier 'static' of the member property of the extend.")
RULEKIND(EXTEND_MEMBER_PROP_MODIFIER_STATIC_ADDED, true, true,
    "Added the modifier 'static' of the member property of the extend.")
RULEKIND(EXTEND_MEMBER_PROP_MODIFIER_PUBLIC_DELETED, true, true,
    "Deleted the modifier 'public' of the member property of the extend.")
RULEKIND(EXTEND_MEMBER_PROP_MUT_DELETED, true, true,
    "Deleted 'Mut' member property of extend visible outside the module.")
RULEKIND(EXTEND_MEMBER_PROP_MUT_ADDED, true, true,
    "Added 'Mut' member property of extend visible outside the module.")
RULEKIND(EXTEND_MEMBER_PROP_DEPRECATED_CHANGED, true, true,
    "Changed the annotation '@Deprecated' of the member property of the extend visible outside the module.")
RULEKIND(EXTEND_MEMBER_PROP_FROZEN_DELETED, false, true,
    "Deleted the annotation '@Frozen' of the member property of the extend visible outside the module.")
RULEKIND(EXTEND_MEMBER_PROP_RETURN_TYPE_CHANGED, true, true,
    "Changed return type of the member property of the extend.")

// Implemented in StructMemberFuncOrPropAdded
RULEKIND(STRUCT_STATIC_MEMBER_FUNC_ADDED, false, true, "Added overriding static member function of struct.")
RULEKIND(STRUCT_STATIC_MEMBER_PROP_ADDED, false, true, "Added overriding static member property of struct.")
RULEKIND(STRUCT_INSTANCE_MEMBER_FUNC_ADDED, false, true, "Added overriding instance member function of struct.")
RULEKIND(STRUCT_INSTANCE_MEMBER_PROP_ADDED, false, true, "Added overriding instance member property of struct.")

// Implemented in StructMemberFuncDeletedOrChanged
RULEKIND(STRUCT_MEMBER_FUNC_DELETED, true, true,
    "Deleted public member function of struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_FUNC_ANNO_CHANGED, true, true,
    "Changed member function's annotation of struct visible outside the module. Deprecated strict added.")
RULEKIND(STRUCT_MEMBER_FUNC_ANNO_CHANGED2, false, true,
    "Changed member function's annotation of struct visible outside the module. Frozen deleted.")
RULEKIND(STRUCT_MEMBER_FUNC_MODIFIER_CHANGED, true, true,
    "Changed member function's modifier of struct visible outside the module. \
public/const/static deleted or unsafe/mut/static added.")
RULEKIND(STRUCT_STATIC_MEMBER_VAR_TO_LET, true, true,
    "Changed member variable's modifier of struct visible outside the module. \
var to let.")
RULEKIND(STRUCT_STATIC_MEMBER_VAR_TO_CONST, true, true,
    "Changed member variable's modifier of struct visible outside the module. \
var to const.")
RULEKIND(STRUCT_STATIC_MEMBER_CONST_TO_LET, true, true,
    "Changed member variable's modifier of struct visible outside the module. \
const to let.")
RULEKIND(STRUCT_STATIC_MEMBER_CONST_TO_VAR, true, true,
    "Changed member variable's modifier of struct visible outside the module. \
const to var.")
RULEKIND(STRUCT_STATIC_MEMBER_LET_TO_CONST, false, true,
    "Changed member variable's modifier of struct visible outside the module. \
let to const.")
RULEKIND(STRUCT_MEMBER_FUNC_MODIFIER_CHANGED2, false, true,
    "Changed member function's modifier of struct visible outside the module. mut deleted.")
RULEKIND(STRUCT_INIT_FUNC_DELETED, true, true,
    "Deleted public init function of struct visible outside the module.")
RULEKIND(STRUCT_INIT_FUNC_ANNO_CHANGED, true, true,
    "Changed init function's annotation of struct visible outside the module. Deprecated strict added.")
RULEKIND(STRUCT_INIT_FUNC_ANNO_CHANGED2, false, true,
    "Changed init function's annotation of struct visible outside the module. Frozen deleted.")
RULEKIND(STRUCT_INIT_FUNC_MODIFIER_CHANGED, true, true,
    "Changed init function's modifier of struct visible outside the module. \
public/const/static deleted or unsafe/mut/static added.")
RULEKIND(STRUCT_INIT_FUNC_MODIFIER_CHANGED2, false, true,
    "Changed init function's modifier of struct visible outside the module. mut deleted.")

// Implemented in StructMemberPropDeletedOrChanged
RULEKIND(STRUCT_MEMBER_PROP_DELETED, true, true,
    "Deleted public member property of struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_PROP_ANNO_CHANGED, true, true,
    "Changed member property's annotation of struct visible outside the module. Deprecated strict added.")
RULEKIND(STRUCT_MEMBER_PROP_ANNO_CHANGED2, false, true,
    "Changed member property's annotation of struct visible outside the module. Frozen deleted.")
RULEKIND(STRUCT_MEMBER_PROP_MODIFIER_CHANGED, true, true,
    "Changed member property's modifier of struct visible outside the module. \
public/mut/static deleted or mut/static added.")
RULEKIND(STRUCT_MEMBER_PROP_RETURN_TYPE_CHANGED, true, true,
    "Changed member property's return type of struct visible outside the module.")

// Implemented in EnumMemberFuncOrPropAdded
RULEKIND(ENUM_STATIC_MEMBER_FUNC_ADDED, false, true, "Added overriding static member function of enum.")
RULEKIND(ENUM_STATIC_MEMBER_PROP_ADDED, false, true, "Added overriding static member property of enum.")
RULEKIND(ENUM_INSTANCE_MEMBER_FUNC_ADDED, false, true, "Added overriding instance member function of enum.")
RULEKIND(ENUM_INSTANCE_MEMBER_PROP_ADDED, false, true, "Added overriding instance member property of enum.")

// Implemented in EnumMemberPropDeletedOrChanged
RULEKIND(ENUM_MEMBER_PROP_DELETED, true, true,
    "Deleted public member property of enum visible outside the module.")
RULEKIND(ENUM_MEMBER_PROP_ANNO_CHANGED, true, true,
    "Changed member property's annotation of enum visible outside the module. Deprecated strict added.")
RULEKIND(ENUM_MEMBER_PROP_ANNO_CHANGED2, false, true,
    "Changed member property's annotation of enum visible outside the module. Frozen deleted.")
RULEKIND(ENUM_MEMBER_PROP_MODIFIER_CHANGED, true, true,
    "Changed member property's modifier of enum visible outside the module. \
public/static deleted or static added.")
RULEKIND(ENUM_MEMBER_PROP_RETURN_TYPE_CHANGED, true, true,
    "Changed member property's return type of enum visible outside the module.")

// Implemented in EnumMemberFuncDeletedOrChanged
RULEKIND(ENUM_MEMBER_FUNC_DELETED, true, true, "Deleted public member function of enum visible outside the module.")
RULEKIND(ENUM_MEMBER_FUNC_ANNO_CHANGED, true, true,
    "Changed member function's annotation of enum visible outside the module. Deprecated strict added.")
RULEKIND(ENUM_MEMBER_FUNC_ANNO_CHANGED2, false, true,
    "Changed member function's annotation of enum visible outside the module. Frozen deleted.")
RULEKIND(ENUM_MEMBER_FUNC_MODIFIER_CHANGED, true, true,
    "Changed member function's modifier of enum visible outside the module. \
public/const/static deleted or const/static added.")

// Implemented in InterfaceStructImplDeleted
RULEKIND(INTERFACE_STRUCT_IMPL_DELETED, true, true, "Deleted interfaces that struct implemented.")
// Implemented in InterfaceStructImplChanged
RULEKIND(INTERFACE_STRUCT_IMPL_CHANGED, true, true, "Changed interfaces that struct implemented.")
// Implemented in InterfaceEnumImplDeleted
RULEKIND(INTERFACE_ENUM_IMPL_DELETED, true, true, "Deleted interfaces that enum implemented.")
// Implemented in InterfaceEnumImplChanged
RULEKIND(INTERFACE_ENUM_IMPL_CHANGED, true, true, "Changed interfaces that enum implemented.")
// Implemented in InterfaceInterfaceImplAdded
RULEKIND(INTERFACE_INTERFACE_IMPL_ADDED, true, false,
    "Added inherited interface with unimplemented function or property of interface.")
// Implemented in InterfaceInterfaceImplDeleted
RULEKIND(INTERFACE_INTERFACE_IMPL_DELETED, true, true, "Deleted interfaces that interface implemented.")
// Implemented in InterfaceInterfaceImplChanged
RULEKIND(INTERFACE_INTERFACE_IMPL_CHANGED, true, true, "Changed interfaces that interface implemented.")
// Implemented in InterfaceClassImplAdded
RULEKIND(INTERFACE_CLASS_IMPL_ADDED, true, false,
    "Added inherited interface with unimplemented function or property of abstract class.")
// Implemented in InterfaceClassImplDeleted
RULEKIND(INTERFACE_CLASS_IMPL_DELETED, true, true, "Deleted interfaces that class implemented.")
// Implemented in InterfaceClassImplChanged
RULEKIND(INTERFACE_CLASS_IMPL_CHANGED, true, true, "Changed interfaces that class implemented.")
// Implemented in SuperclassClassInheritDeletedOrAdded
RULEKIND(SUPERCLASS_CLASS_INHERIT_DELETED, true, true, "Deleted superclasses that class inherited.")
RULEKIND(SUPERCLASS_CLASS_INHERIT_ADDED, false, true, "Added superclasses that class inherited.")
// Implemented in SuperclassClassInheritChanged
RULEKIND(SUPERCLASS_CLASS_INHERIT_CHANGED, true, true, "Changed superclasses that class inherited.")
// Implemented in InterfaceExtendImplDeleted
RULEKIND(INTERFACE_EXTEND_IMPL_DELETED, true, true, "Deleted interfaces that extend implemented.")
// Implemented in InterfaceExtendImplChanged
RULEKIND(INTERFACE_EXTEND_IMPL_CHANGED, true, true, "Changed interfaces that extend implemented.")
// Implemented in AddDelImportStatement
RULEKIND(ADD_IMPORT_STATEMENT, false, true, "Added import statement or changed modifier of an existing one.")
RULEKIND(DEL_IMPORT_STATEMENT, true, true, "Deleted import statement or changed modifier of an existing one.")
RULEKIND(DEL_NONPUBLIC_IMPORT_STATEMENT, false, true, "Deleted non public import statement.")
// Implemented in DelModTypeAlias
RULEKIND(DEL_TYPE_ALIAS, true, true, "Public type alias deleted.")
RULEKIND(MOD_TYPE_ALIAS, true, true, "Public type alias modified.")
// Implemented in VarModifierModified
RULEKIND(
    VAR_NON_ACCESS_MODIFIER_MODIFIED, true, true, "Modified variable non-access modifier visible outside the module.")
RULEKIND(VAR_NON_ACCESS_MODIFIER_MODIFIED_LET_CONST, false, true,
    "Modified variables' non-access modifier from let to const for class visible outside the module.")
RULEKIND(VAR_ACCESS_MODIFIER_MODIFIED, true, true,
    "Variable declaration visible outside the module made protected or internal.")
// Implemented in FuncModifierModified
RULEKIND(FUNC_ACCESS_MODIFIER_MODIFIED, true, true,
    "Function declaration visible outside the module made protected or internal.")
RULEKIND(
    FUNC_NON_ACCESS_MODIFIER_UNSAFE_ADDED, true, true, "Function declaration visible outside the module made unsafe.")
RULEKIND(FUNC_NON_ACCESS_MODIFIER_CONST_REMOVED, true, true,
    "Const modifier removed for function declaration visible outside the module.")
// Implemented in StructModifierModified
RULEKIND(STRUCT_ACCESS_MODIFIER_MODIFIED, true, true,
    "Struct declaration visible outside the module made protected or internal.")
// Implemented in EnumModifierModified
RULEKIND(ENUM_ACCESS_MODIFIER_MODIFIED, true, true,
    "Enum declaration visible outside the module made protected or internal.")
// Implemented in ClassModifierModified
RULEKIND(CLASS_ACCESS_MODIFIER_MODIFIED, true, true,
    "Class declaration visible outside the module made protected or internal.")
RULEKIND(CLASS_NON_ACCESS_MODIFIER_ABSTRACT_ADDED, true, true,
    "Class declaration visible outside the module added abstract.")
RULEKIND(CLASS_NON_ACCESS_MODIFIER_ABSTRACT_DELETED, true, true,
    "Class declaration visible outside the module deleted abstract.")
RULEKIND(CLASS_MODIFIER_PUBLIC_DELETED, true, true,
    "Class declaration visible outside the module deleted public.")
RULEKIND(CLASS_MODIFIER_SEALED_DELETED, true, true,
    "Class declaration visible outside the module deleted sealed.")
RULEKIND(
    CLASS_NON_ACCESS_MODIFIER_SEALED_ADDED, true, true, "Class declaration visible outside the module made sealed.")
RULEKIND(CLASS_NON_ACCESS_MODIFIER_OPEN_REMOVED, true, true,
    "Open modifier removed for class declaration visible outside the module.")
// Implemented in InterfaceModifierModified
RULEKIND(INTERFACE_ACCESS_MODIFIER_MODIFIED, true, true,
    "Interface declaration visible outside the module made protected or internal.")
RULEKIND(INTERFACE_MODIFIER_SEALED_ADDED, true, true,
    "Interface declaration visible outside the module added modifier 'sealed'.")
RULEKIND(INTERFACE_MODIFIER_SEALED_DELETED, true, true,
    "Interface declaration deleted modifier 'sealed' when interface without modifier 'public'.")
RULEKIND(INTERFACE_MODIFIER_PUBLIC_DELETED, true, true,
    "Interface declaration visible outside the module deleted or changed modifier 'public'.")
// Implemented in PackageDeclarationChanges
RULEKIND(PACKAGE_DECL_CHANGED, true, true, "Package name or package access level changed.")
// Implemented in StructMemberVarDeleted
RULEKIND(STRUCT_STATIC_VAR_DEL, true, true, "Deleted static variable member from the struct.")
// Implemented in StructMemberVarDeleted
RULEKIND(STRUCT_INSTANCE_VAR_DEL, true, true, "Deleted instance public variable member from the struct.")
// Implemented in StructMemberVarModified
RULEKIND(STRUCT_INSTANCE_VAR_ACCESS_MODIFIED, true, true,
    "Modified instance public variable member modifier to not visible outside the struct.")
// Implemented in StructMemberVarDeleted
RULEKIND(STRUCT_INST_NO_PUBLIC_VAR_DEL, false, true, "Deleted instance non public variable member from the struct.")
// Implemented in StructStaticMemberVarModified
RULEKIND(STRUCT_STATIC_VAR_PUBLIC_DEL, true, true,
    "Changed static variable member's modifier of struct visible outside the module. public deleted.")
RULEKIND(STRUCT_STATIC_VAR_INIT_MODIFIED, false, true,
    "Change the initial value of the const variable.")
RULEKIND(STRUCT_STATIC_VAR_TYPE_MODIFIED, true, true,
    "Modified static variables' type for struct visible outside the module. The new type is not a subtype of the old "
    "type.")
RULEKIND(STRUCT_STATIC_VAR_TYPE_MODIFIED_CLASS_LIKE, false, true, "Modified static variables' type for struct visible \
outside the module. The two types are compatible only when both are classes or interfaces types.")

// Implemented in ClassMemberFuncOrPropAdded
RULEKIND(CLASS_STATIC_MEMBER_FUNC_ADDED, false, true, "Added overriding static member function of class.")
RULEKIND(CLASS_STATIC_MEMBER_PROP_ADDED, false, true, "Added overriding static member property of class.")
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_ADDED, true, false,
    "Added instance member function without default implementation of class.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_ADDED, true, false,
    "Added instance member property without default implementation of class.")
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_ADDED2, false, true,
    "Added overriding instance member function of class.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_ADDED2, false, true,
    "Added overriding instance member property of class.")

// Implemented in ClassOpenInstanceMemberAdded
RULEKIND(CLASS_OPEN_INSTANCE_MEMBER_FUNCS_ADDED, false, true, "Added open instance member functions for classes.")
RULEKIND(CLASS_OPEN_INSTANCE_MEMBER_PROPS_ADDED, false, true, "Added open instance member properties for classes.")
// Implemented in ClassOpenInstanceMemberOrderChanged
RULEKIND(CLASS_OPEN_INSTANCE_MEMBER_FUNCS_ORDER_CHANGED, false, true,
    "Changed order of open instance member functions for classes.")
RULEKIND(CLASS_OPEN_INSTANCE_MEMBER_PROPS_ORDER_CHANGED, false, true,
    "Changed order of open instance member properties for classes.")
// Implemented in ClassInstanceMemberDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_FUNCS_DELETED, true, true,
    "Class member function visible outside the module deleted or made private.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROPS_DELETED, true, true,
    "Class member property visible outside the module deleted or made private.")
RULEKIND(PRIMARY_CONSTRUCTOR_ADDED, false, true, "Added primary constructor with member variable parameter.")
RULEKIND(PRIMARY_CONSTRUCTOR_NON_PUBLIC_DELETED, true, true,
    "Deleted non public primary constructor with member variable parameter.")
RULEKIND(PRIMARY_CONSTRUCTOR_NON_PUBLIC_DELETED2, false, true,
    "Deleted non public primary constructor with private member variable parameter.")
RULEKIND(CLASS_CONSTRUCTOR_DELETED, true, true,
    "Class constructor visible outside the module deleted or made private.")
// Implemented in ClassInstanceMemberFrozenAnnoDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_FROZEN_ANNO_DELETED, false, true,
    "Deleted frozen annotation of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_FROZEN_ANNO_DELETED, false, true,
    "Deleted frozen annotation of instance member properties for classes.")
RULEKIND(CLASS_CONSTRUCTOR_FROZEN_ANNO_DELETED, false, true,
    "Deleted frozen annotation of constructors for classes.")
// Implemented in ClassInstanceMemberVisibilityChanged
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_VISIBILITY_CHANGED, true, true,
    "Changed visibility of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_VISIBILITY_CHANGED, true, true,
    "Changed visibility of instance member properties for classes.")
RULEKIND(CLASS_CONSTRUCTOR_VISIBILITY_CHANGED, true, true,
    "Changed visibility of constructors for classes.")
// Implemented in ClassInstanceMemberFuncUnsafeAttrAdded
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_UNSAFE_ATTR_ADDED, true, true,
    "Added unsafe attribute of instance member functions for classes.")
// Implemented in ClassInstanceMemberFuncConstAttrAdded
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_CONST_ATTR_ADDED, false, true,
    "Added const attribute of instance member functions for classes.")
// Implemented in ClassInstanceMemberFuncConstAttrDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_CONST_ATTR_DELETED, true, true,
    "Deleted const attribute of instance member functions for classes.")
RULEKIND(CLASS_CONSTRUCTOR_CONST_ATTR_DELETED, true, true,
    "Deleted const attribute of constructors for classes.")
// Implemented in ClassInstanceMemberFuncStaticAttrAddedDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_STATIC_ATTR_DELETED, true, true,
    "Deleted static attribute of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_STATIC_ATTR_ADDED, true, true,
    "Added static attribute of instance member functions for classes.")
// Implemented in ClassInstanceMemberFuncOpenAttrAddedDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_OPEN_ATTR_DELETED, true, true,
    "Deleted open attribute of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_OPEN_ATTR_ADDED, false, true,
    "Added open attribute of instance member functions for classes.")
// Implemented in ClassInstanceMemberPropStaticAttrAddedDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_STATIC_ATTR_ADDED, true, true,
    "Added static attribute of instance member properties for classes.")
// Implemented in ClassInstanceMemberPropStaticAttrAddedDeleted
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_STATIC_ATTR_DELETED, true, true,
    "Deleted static attribute of instance member properties for classes.")
// Implemented in ClassMemberPropMutAttrDeleted
RULEKIND(CLASS_MEMBER_PROP_MUT_ATTR_DELETED, true, true,
    "Deleted mut attribute of member properties for classes.")
// Implemented in ClassInstanceMemberDepreAnnoAddedOrChanged
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_DEPRE_ANNO_ADDED, true, true,
    "Added deprecated annotation of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_FUNC_DEPRE_ANNO_CHANGED, true, true,
    "Changed deprecated annotation of instance member functions for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_DEPRE_ANNO_ADDED, true, true,
    "Added deprecated annotation of instance member properties for classes.")
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_DEPRE_ANNO_CHANGED, true, true,
    "Changed deprecated annotation of instance member properties for classes.")
RULEKIND(CLASS_CONSTRUCTOR_DEPRE_ANNO_ADDED, true, true,
    "Added deprecated annotation of constructors for classes.")
RULEKIND(CLASS_CONSTRUCTOR_DEPRE_ANNO_CHANGED, true, true,
    "Changed deprecated annotation of constructors for classes.")
// Implemented in StructMemberVarModified
RULEKIND(STRUCT_MEMBER_VAR_NON_ACCESS_MODIFIER_MODIFIED, true, true,
    "Modified member variables' non-access modifier for struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_VAR_NAME_MODIFIED, true, true,
    "Modified public member variables' name for struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_VAR_TYPE_MODIFIED, true, true,
    "Modified member variables' type for struct visible outside the module.")
RULEKIND(STRUCT_NONPUBLIC_MEMBER_VAR_TYPE_MODIFIED, false, true,
    "Modified nonpublic member variables' type for struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_VAR_ORDER_MODIFIED, false, true,
    "Modified member variables' order for struct visible outside the module.")
RULEKIND(STRUCT_MEMBER_VAR_ADDED_ABI, false, true, "Added member variable to a struct.")
RULEKIND(STRUCT_CLASS_MEMBER_VAR_INIT_MODIFIED, false, true,
    "Change the initial value of member variable when struct/class has the const variable or @Frozen init func.")
// Implemented in ClassInstMemberVarAddedOrDeleted
RULEKIND(CLASS_MEMBER_VAR_DELETED_ABI, false, true, "Deleted private/internal member variable of a class.")
RULEKIND(CLASS_STATIC_MEMBER_VAR_DELETED, true, true,
    "Class public static member variable visible outside the module deleted or made private.")
RULEKIND(CLASS_PUBLIC_MEMBER_VAR_DELETED, true, true, "Deleted class public/protected member variable.")
RULEKIND(CLASS_MEMBER_VAR_ADDED_ABI, false, true, "Added member variable to a class.")
// Implemented in ClassStaticMemberVarModified
RULEKIND(CLASS_STATIC_CONST_VAR_INIT_MODIFIED, false, true,
    "Modified static constant variables' initial value for class visible outside the module.")
RULEKIND(CLASS_STATIC_VAR_TYPE_MODIFIED, true, true,
    "Modified static variables' type for class visible outside the module. The new type is not a subtype of the old "
    "type.")
RULEKIND(CLASS_STATIC_VAR_TYPE_MODIFIED_CLASS_LIKE, false, true, "Modified static variables' type for class visible \
outside the module. The two types are compatible only when both are classes or interfaces types.")
RULEKIND(CLASS_STATIC_VAR_ACCESS_MODIFIER_MODIFIED, true, true,
    "Class static member variable visible outside the module made protected or internal.")
RULEKIND(CLASS_STATIC_VAR_NON_ACCESS_MODIFIER_MODIFIED, true, true,
    "Modified static member variables' non-access modifier for class visible outside the module.")
RULEKIND(CLASS_STATIC_VAR_DELETED_STATIC, true, true,
    "Modified static member variables deleted static.")
RULEKIND(CLASS_STATIC_VAR_NON_ACCESS_MODIFIER_MODIFIED_LET_CONST, false, true,
    "Modified member variables' non-access modifier from let to const for class visible outside the module.")
// Implemented in ClassInstMemberVarModified
RULEKIND(CLASS_MEMBER_VAR_NON_ACCESS_MODIFIER_MODIFIED, true, true,
    "Modified member variables' non-access modifier for class visible outside the module.")
RULEKIND(CLASS_MEMBER_VAR_NAME_MODIFIED, true, true,
    "Modified public member variables' name for class visible outside the module.")
RULEKIND(CLASS_PUBLIC_PROTECTED_MEMBER_VAR_TYPE_MODIFIED, true, true,
    "Modified member variables' type for class visible outside the module.")
RULEKIND(CLASS_PRIVATE_INTERNAL_MEMBER_VAR_TYPE_MODIFIED, false, true,
    "Modified private/internal member variables' type for class visible outside the module.")
RULEKIND(CLASS_MEMBER_VAR_ORDER_MODIFIED, false, true,
    "Modified member variables' order for class visible outside the module.")
RULEKIND(CLASS_MEMBER_VAR_ACCESS_MODIFIER_MODIFIED, true, true,
    "Class member variable declaration visible outside the module made internal, or private.")
RULEKIND(CLASS_MEMBER_VAR_DEPRE_ANNO_MOD, true, true,
    "Modified class member variables deprecated annotation with compilation error.")
RULEKIND(CLASS_MEMBER_VAR_DEPRE_ANNO_ADD, true, true,
    "Added deprecated annotation to class member variable with compilation error.")
// Implemented in EnumConstructorDeleted
RULEKIND(ENUM_CONSTRUCTOR_DELETED, true, true, "Deleted enum constructor.")
// Implemented in EnumInstMemberVarModified
RULEKIND(ENUM_MEMBER_VAR_ORDER_MODIFIED, false, true,
    "Modified order of constructor for enum visible outside the module.")
// Implemented in EnumConstructorAdded
RULEKIND(ENUM_CONSTRUCTOR_ADDED_EXHAUSTIVE, true, true, "Added constructor to exhaustive enum.")
RULEKIND(ENUM_CONSTRUCTOR_ADDED_NON_EXHAUSTIVE, false, true,
    "Added constructor to non-exhaustive enum, and either the constructor is not at the end or the memory layout has "
    "changed.")
// Implemented in ClassInstanceMemberPropRetTypeOrBodyChanged
RULEKIND(CLASS_INSTANCE_MEMBER_PROP_RETURN_TYPE_CHANGED, true, true,
    "Changed member property's return type of class visible outside the module.")
#endif