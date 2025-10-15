// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_G_SER_03_CHECKER_H
#define DATAFLOW_RULE_G_SER_03_CHECKER_H

#include "../DataflowRule.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

namespace Cangjie::CodeCheck {
class DataflowRuleGSER03Checker : public DataflowRule {
public:
    explicit DataflowRuleGSER03Checker(CodeCheckDiagnosticEngine *diagEngine);
    ~DataflowRuleGSER03Checker() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    using PositionPair = std::pair<Cangjie::Position, Cangjie::Position>;
    using TypeWithPosition = std::pair<std::string, PositionPair>;
    // Retrieve whether it is a serialization type from the function call,
    // and if so, record the serialization code address.
    TypeWithPosition TypeInSer(const CHIR::Apply *apply, bool inDataStruct = false);
    // Retrieve whether it is a deserialization type from the function call,
    // and if so, record the deserialization code address.
    TypeWithPosition TypeInDeser(const CHIR::Apply *apply);
    // Serialization Types and Corresponding DataModel Types
    std::map<std::string, std::string> typeToDataModel = {{"Bool", "DataModelBool"}, {"Int8", "DataModelInt"},
        {"Int16", "DataModelInt"}, {"Int32", "DataModelInt"}, {"Int64", "DataModelInt"}, {"UInt8", "DataModelInt"},
        {"UInt16", "DataModelInt"}, {"UInt32", "DataModelInt"}, {"UInt64", "DataModelInt"},
        {"Float16", "DataModelFloat"}, {"Float32", "DataModelFloat"}, {"Float64", "DataModelFloat"},
        {"String", "DataModelString"}, {"Rune", "DataModelString"}};
    // Records the serialization and deserialization functions of a class or struct.
    using SerialisePair = std::pair<CHIR::Func *, CHIR::Func *>;
    using SerialiseMap = std::map<std::string, std::vector<TypeWithPosition>>;
    // Build a map of serialization and deserialization function pairs
    // key: ClassDef.identifier, value: {serial func, deserial func}
    std::map<CHIR::CustomTypeDef *, SerialisePair> allSerialiseClass;
    void SetSerInRetToFieldSerMapHelper(CHIR::Load *load, TypeWithPosition &typeStr, SerialiseMap &serTypeMap);
    void SetSerInRetToFieldSerMap(CHIR::Apply *apply, TypeWithPosition &typeStr, SerialiseMap &serTypeMap);
    void CollectClassDefWithSer(CHIR::Package &package);

    void CollectSerRelatedFuncsInDef(CHIR::CustomTypeDef *classDef);
    std::string GetValueIdentifier(const CHIR::Value *value);
    // Collect all serialization types
    void CollectSerType(const CHIR::Expression &expr, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap, CHIR::LocalVar *ret);
    // Check all deSerialization types
    void CheckSerType(const CHIR::Expression &expr, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap);
    std::string JointSerializeTypeAndLine(std::vector<TypeWithPosition> &serTypes, bool dataModel = false);
    std::pair<Cangjie::Position, Cangjie::Position> deSerFuncLocation;
    bool CollectSerTypeInStore(
        const CHIR::Store *store, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap, CHIR::LocalVar *ret);
    bool CollectSerTypeInInitApply(const CHIR::Apply *apply, CHIR::ImportedValue *imported,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &fieldSerMap);
    bool CollectSerTypeInAddApply(const CHIR::Apply *apply, CHIR::ImportedValue *imported,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &fieldSerMap);
    void CheckSerTypeInCallee(const CHIR::Apply *apply, std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap,
        SerialiseMap &fieldSerMap);
    void CheckSerTypeInArgs(const CHIR::Apply *apply, DataflowRuleGSER03Checker::TypeWithPosition &deserTypeStr,
        std::vector<TypeWithPosition> &serTypes, SerialiseMap &serMap, SerialiseMap &fieldSerMap);
};
} // namespace Cangjie::CodeCheck

#endif
