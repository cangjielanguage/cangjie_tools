// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.fs.*
import std.collection.*
import stdx.serialization.serialization.*
import cjpm.util.*

private const SOURCE_SET_SRC_DIR_TOML = "src-dir"
private const SOURCE_SET_CONDITION_TOML = "condition"
private const SOURCE_SET_COMMON_NAME = "common"
private const SOURCE_SET_OTHER_NAME = "other"
private const SOURCE_SET_MAIN_NAME = "main"
private const SOURCE_SET_LINUX_NAME = "linux"
private const SOURCE_SET_WINDOWS_NAME = "windows"
private const SOURCE_SET_DARWIN_NAME = "darwin"
private const SOURCE_SET_ANDROID_NAME = "android"
private const SOURCE_SET_HOS_NAME = "hos"
private const SOURCE_SET_OHOS_NAME = "ohos"
private let SOURCE_SET_SPECIAL_NAMES = [SOURCE_SET_COMMON_NAME, SOURCE_SET_CONDITION_TOML, SOURCE_SET_SRC_DIR_TOML,
    SOURCE_SET_OTHER_NAME]
private let SOURCE_SET_RESERVED_KEYWORDS = [SOURCE_SET_SRC_DIR_TOML, SOURCE_SET_CONDITION_TOML]
private let SOURCE_SET_DEFAULTABLE = [SOURCE_SET_COMMON_NAME, SOURCE_SET_LINUX_NAME, SOURCE_SET_WINDOWS_NAME,
    SOURCE_SET_DARWIN_NAME, SOURCE_SET_ANDROID_NAME, SOURCE_SET_HOS_NAME, SOURCE_SET_OHOS_NAME]

private func collectDataModelStruct(it: Iterable<Field>): DataModelStruct {
    DataModelStruct(it |> collectArrayList)
}

private func collectDataModelSeq(it: Iterable<DataModel>): DataModelSeq {
    DataModelSeq(it |> collectArrayList)
}

extend DataModelStruct {
    prop names: Array<String> {
        get() {
            this.getFields() |> map {it: Field => it.getName()} |> collectHashSet |> collectArray
        }
    }

    func anyNotNames(fieldNames: Array<String>): Array<String> {
        this.names |> filter {it: String => !fieldNames.contains(it)} |> collectArray
    }

    func containsAnyNot(fieldNames: Array<String>): Bool {
        (anyNotNames(fieldNames) |> count) != 0
    }

    func intersect(fieldNames: Array<String>): Array<String> {
        this.getFields() |> map {it: Field => it.getName()} |> filter {it: String => fieldNames.contains(it)} |>
            collectArray
    }

    func merge(other: DataModelStruct): DataModelStruct {
        var newdm = DataModelStruct(other.getFields())
        for (field in this.getFields()) {
            if (newdm.get(field.getName()) is DataModelNull) {
                newdm.add(field)
            }
        }
        newdm
    }
}

abstract sealed class CjpmConfigVisitor {
    protected open func visitRoot(_: DataModel): SourceSetsConfiguration

    protected open func visitSourceSetsConfiguration(_: DataModel): ?SourceSetsConfiguration

    protected open func visitSourceSet(_: DataModel): ?SourceSet

    protected open func visitSourceDirectories(_: DataModel): ?SourceDirectories

    protected open func visitConditions(_: DataModel): ?Conditions
}

private struct VisitorContext {
    VisitorContext(
        private var _parentFieldFqName: String,
        private var _currentFieldName: String
    ) {}

    public prop fieldFqName: String {
        get() {
            let val = if (!_parentFieldFqName.isEmpty()) {
                "${_parentFieldFqName}."
            } else {
                ""
            }
            "${val}${_currentFieldName}"
        }
    }

    public prop parentFieldFqName: String {
        get() {
            _parentFieldFqName
        }
    }

    public prop currentName: String {
        get() {
            _currentFieldName
        }
    }

    public func forward(name: String): VisitorContext {
        VisitorContext(this.fieldFqName, name)
    }
}

// Storate for all of the mutable irrevertible state of the visitor
private class VisitorState {
    var packageSrcDir: WithDefault<String> = WithDefault(DIR_SRC)
    VisitorState() {}
}

// `cjpm.toml` parser implemented as visitor pattern specifically for source-set configuration
// In future might be extended for CJPM configuration as a whole
public open class ConfigParserVisitor <: CjpmConfigVisitor {
    private let vstate: VisitorState = VisitorState()
    private var ctx: VisitorContext = VisitorContext("", "")
    private var errors: ArrayList<String> = ArrayList()

    private static let defaulsSourceSets: HashMap<String, DataModelStruct> = HashMap(
        [
            (SOURCE_SET_LINUX_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_LINUX_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_LINUX}", "${ENV_GNU}"])] |> collectDataModelStruct),
            (SOURCE_SET_DARWIN_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_DARWIN_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_DARWIN}"])] |> collectDataModelStruct),
            (SOURCE_SET_WINDOWS_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_WINDOWS_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_WINDOWS}"])] |> collectDataModelStruct),
            (SOURCE_SET_ANDROID_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_ANDROID_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_LINUX}", "${ENV_ANDROID}"])] |> collectDataModelStruct),
            (SOURCE_SET_HOS_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_HOS_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_LINUX}", "${ENV_HOS}"])] |> collectDataModelStruct),
            (SOURCE_SET_OHOS_NAME,
                [field(SOURCE_SET_SRC_DIR_TOML, "${SOURCE_SET_OHOS_NAME}"),
                    field(SOURCE_SET_CONDITION_TOML, ["${OS_LINUX}", "${ENV_OHOS}"])] |> collectDataModelStruct)
        ]
    )

    private func visitField<T>(dms: DataModelStruct, name: String, visitor: (DataModel) -> T): T {
        return fakeVisitField(dms.get(name), name, visitor)
    }

    // Visit some DataModel as if it was a field member. (Hell of a crutch)
    private func fakeVisitField<T>(dm: DataModel, name: String, visitor: (DataModel) -> T): T {
        let oldCtx = ctx
        ctx = ctx.forward(name)
        let res = visitor(dm)
        ctx = oldCtx
        return res
    }

    private func visitOptionalField<T>(dms: DataModelStruct, name: String, visitor: (DataModel) -> T): ?T {
        match (dms.get(name)) {
            case _: DataModelNull => return None
            case dm =>
                let oldCtx = ctx
                ctx = ctx.forward(name)
                let res = visitor(dm)
                ctx = oldCtx
                return res
        }
    }

    private func checkTable(dm: DataModel): ?DataModelStruct {
        match (dm) {
            case data: DataModelStruct => data
            case _ =>
                errors.add("Field `${ctx.fieldFqName}` should be table")
                return None
        }
    }

    // An entry point to parsing of `cjpm.toml`. Currently only [source-set] declaration is visited
    protected open func visitRoot(dm: DataModel): SourceSetsConfiguration {
        let dms: DataModelStruct = match (dm) {
            case data: DataModelStruct => data
            case _ => throw CliFailError("Field `${ctx.fieldFqName}` should be table")
        }

        assertion(
            {=> !(dms.get("package") is DataModelNull)},
            {
                => "Source sets are supported only for packages. Please, make sure [package] field is always present in `cjpm.toml`"
            }
        )
        // Used only to update VisitorContext on the presence of `src-dir` field
        visitField(dms, "package", visitPackage)

        if (!(dms.get(SOURCE_SET_CONFIG) is DataModelNull) && !state.isExperimental) {
            throw CliFailError(
                "Error: `[source-set]` is part of experimental Cangjie Multiplatform feature, please enable `experimental = true` in cjpm.toml of this project"
            )
        }
        let res = if (let Some(sourceSetField) <- visitOptionalField(dms, SOURCE_SET_CONFIG,
            visitSourceSetsConfiguration)) {
            if (vstate.packageSrcDir.isSet) {
                errors.add("Only one of `package.${SRC_DIR}` and `[${SOURCE_SET_CONFIG}]` fields should be specified")
                Option<SourceSetsConfiguration>.None
            } else {
                sourceSetField
            }
        } else {
            SourceSetsConfiguration(vstate.packageSrcDir.value)
        }

        if (!errors.isEmpty()) {
            throw CliFailError(errors)
        }

        if (let Some(sscfg) <- res) {
            assertion(
                {=> errors.isEmpty()},
                {=> "Errors are present while didn't manage to parse DataModel"}
            )
            return sscfg
        } else {
            assertion(
                {=> !errors.isEmpty()},
                {=> "Didn't manage to parse DataModel but didn't find errors"}
            )
            throw CliFailError(errors)
        }
    }

    private func visitPackage(dm: DataModel): Unit {
        match (dm as DataModelStruct) {
            case Some(val) => match (val.get(SRC_DIR)) {
                case _: DataModelNull => ()
                case s: DataModelString =>
                    if (!s.getValue().isEmpty()) {
                        vstate.packageSrcDir.modify(s.getValue())
                    }
                case _ => errors.add("Field `${ctx.fieldFqName}.${SRC_DIR}` should be string literal")
            }
            case None => errors.add("Field `${ctx.fieldFqName}` should be a table")
        }
    }

    protected open func visitSourceSetsConfiguration(dm: DataModel): ?SourceSetsConfiguration {
        let dms = checkTable(dm) ?? return None

        var hasFailed = false

        if (!(dms.get(SOURCE_SET_MAIN_NAME) is DataModelNull)) {
            errors.add(
                "`${ctx.fieldFqName}.${SOURCE_SET_MAIN_NAME}` is a reserved name for root source set, consider using other name"
            )
            hasFailed = true
        }

        let names = dms.names |> filter {it: String => it != SOURCE_SET_MAIN_NAME} |> collectArray

        var maybeValid = names |>
            filterMap {
            name: String =>
            let data = visitField(dms, name, checkTable)
            // Setting defaults for unset values
            if (let Some(nestedDms) <- data) {
                let newDm: DataModelStruct = if (let Some(defaultDm) <- defaulsSourceSets.get(name)) {
                    defaultDm.merge(nestedDms)
                } else {
                    nestedDms
                }
                Field(name, newDm)
            } else {
                Option<Field>.None
            }
        } |> collectDataModelStruct

        // Something got filtered out due to an error
        hasFailed ||= (maybeValid.names.size != names.size)
        var fields = maybeValid.getFields().iterator()
        if (!maybeValid.containsAnyNot(SOURCE_SET_DEFAULTABLE)) {
            fields = fields |> concat(defaulsSourceSets.iterator() |> map {it => Field(it[0], it[1])})
        }
        maybeValid = fields |> concat([field(SOURCE_SET_SRC_DIR_TOML, ".")]) |> collectDataModelStruct

        let result = SourceSetsConfiguration(visitSourceSet(maybeValid) ?? return None)

        if (hasFailed) {
            return None
        }

        return result
    }

    // Parsing of all source set declarations unified in single function
    protected open func visitSourceSet(dm: DataModel): ?SourceSet {
        let dms = checkTable(dm) ?? return None
        if (dms.containsAnyNot(SOURCE_SET_RESERVED_KEYWORDS)) {
            return Group(visitGroupConfig(dms) ?? return None)
        } else {
            return Source(visitSourceConfig(dms) ?? return None)
        }
    }

    // Check whether source set declaration is DataModelStruct
    // 
    // Check `common/other` source set declaration not containing nested source sets and illegal fields field
    private func verifySpecialSourceSetDM(dm: DataModel): ?DataModelStruct {
        let dms = checkTable(dm) ?? return None

        if (dms.containsAnyNot(SOURCE_SET_RESERVED_KEYWORDS)) {
            errors.add("Source set `${ctx.fieldFqName}` cannot have nested source sets")
        }

        let forbidden: Array<String> = match {
            case ctx.currentName == SOURCE_SET_OTHER_NAME => [SOURCE_SET_CONDITION_TOML]
            case ctx.currentName == SOURCE_SET_COMMON_NAME => [SOURCE_SET_CONDITION_TOML]
            case _ => []
        }

        for (forbiddenName in dms.intersect(forbidden)) {
            errors.add("Illegal field set for common/other source set `${ctx.fieldFqName}`: `${forbiddenName}`")
        }

        return dms
    }

    // Parsing of source set declaration which specifies a source set containing nested source sets
    // 
    // Source set declaration is identified as grouping one if it's table contains any key, different from the reserved one.
    // 
    // NOTE: Value of such non-reserved keys are obliged to be source set declaration
    // 
    // Example syntax:
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 1. Intented syntax
    // 
    // [source-set.socketSelection]
    //   src-dir = "src/net/select"
    // 
    // // From this now on: socketSelection is interpreted as grouping source set
    // [source-set.socketSelection.epoll]
    //   src-dir = "src/net/select/epoll"
    //   condition = [ "feature.os.epoll" ]
    // 
    // [source-set.socketSelection.kqueue]
    //   src-dir = "src/net/select/kqueue"
    //   condition = [ "feature.os.kqueue" ]
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 2. Syntax, which TOML also allows
    // 
    // [source-set.socketSelection]
    //   src-dir = "src/net/select"
    //   epoll = {
    //     src-dir = "src/net/select/epoll",
    //     condition = [ "feature.os.epoll" ]
    //   }
    //   kqueue = {
    //     src-dir = "src/net/select/kqueue",
    //     condition = [ "feature.os.kqueue" ]
    //   }
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // NOTE: source set, which contains any nested source sets cannot have more than one `src-dir`
    protected open func visitGroupConfig(dms: DataModelStruct): ?GroupConfig {
        let paths = visitField(dms, SOURCE_SET_SRC_DIR_TOML, visitGroupConfigSourceDirectories)

        let conditions = visitField(dms, SOURCE_SET_CONDITION_TOML, visitConditions)

        var commonSetDms = visitOptionalField(dms, SOURCE_SET_COMMON_NAME, verifySpecialSourceSetDM) ?? Some(
            DataModelStruct())
        var otherSetDms = visitOptionalField(dms, SOURCE_SET_OTHER_NAME, verifySpecialSourceSetDM) ?? Some(
            DataModelStruct())

        let nestedSetsNames = dms.names |> filter {
            it: String => !SOURCE_SET_SPECIAL_NAMES.contains(it)
        } |> collectArray

        var hasFailed = false
        if (nestedSetsNames.size == 0) {
            errors.add(
                "Parse error in `${ctx.fieldFqName}`: 'other' and 'common' source sets can only be redefined if any other nested source set was specified"
            )
            hasFailed = true
        }

        let nestedSets = nestedSetsNames |>
            filterMap<String, (String, SourceSet)> {
            name: String => if (let Some(ss) <- visitField(dms, name, visitSourceSet)) {
                (name, ss)
            } else {
                None
            }
        } |> collectHashMap

        // Something got filtered out due to an error
        hasFailed ||= (nestedSets.size != nestedSetsNames.size)

        let pathsRes = paths ?? return None
        assertion(
            {=> pathsRes.isSingleSource},
            {
                => "For unknown reason, grouping source set didn't sanitize `src-dir` field to have single source directory: ${ctx.fieldFqName}"
            }
        )

        let (otherSet, commonSet) = generateOtherCommon(pathsRes.sourceDirs[0], otherSetDms, commonSetDms)

        if (hasFailed) {
            return None
        }

        GroupConfig(pathsRes, conditions ?? return None, commonSet ?? return None, otherSet ?? return None, nestedSets)
    }

    private func generateOtherCommon(path: String, otherDms: Option<DataModelStruct>, commonDms: Option<DataModelStruct>): (Option<SourceSet>, 
        Option<SourceSet>) {
        var defaultOther = [field(SOURCE_SET_SRC_DIR_TOML, "${path}/${SOURCE_SET_OTHER_NAME}")] |> collectDataModelStruct
        var defaultCommon = [field(SOURCE_SET_SRC_DIR_TOML, "${path}/${SOURCE_SET_COMMON_NAME}")] |>
            collectDataModelStruct

        let otherSet = if (let Some(otherSetDmsVal) <- otherDms) {
            fakeVisitField(defaultOther.merge(otherSetDmsVal), SOURCE_SET_OTHER_NAME, visitSourceSet)
        } else {
            Option<SourceSet>.None
        }
        let commonSet = if (let Some(commonSetDmsVal) <- commonDms) {
            fakeVisitField(defaultCommon.merge(commonSetDmsVal), SOURCE_SET_COMMON_NAME, visitSourceSet)
        } else {
            Option<SourceSet>.None
        }
        (otherSet, commonSet)
    }

    // Checking if our limitation for source set to have single `src-dir` applies
    private func visitGroupConfigSourceDirectories(dm: DataModel): ?SourceDirectories {
        if (let Some(dmseq) <- (dm as DataModelSeq)) {
            let arr = Array<String>.deserialize(dmseq)
            if (arr.size > 1) {
                errors.add(
                    "`${ctx.fieldFqName}` cannot have multiple source directories due to `${ctx.parentFieldFqName}` containing nested source sets"
                )
                return None
            }
        }
        visitSourceDirectories(dm)
    }

    // Parsing of source set declaration which specifies a source set NOT containing any other
    // 
    // Example syntax:
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // /** This is grouping source set **/
    // [source-set.socketSelection]
    //   src-dir = "src/net/select"
    // 
    // /** This is source set with `*.cj` files **/
    // [source-set.socketSelection.epoll]
    //   src-dir = "src/net/select/epoll"
    //   condition = [ "feature.os.epoll" ]
    // 
    // /** Also this on is source set with `.*cj` files **/
    // [source-set.socketSelection.kqueue]
    //   src-dir = "src/net/select/kqueue"
    //   condition = [ "feature.os.kqueue" ]
    // 
    // --------------------------------------------------------------------------------------------------------
    protected open func visitSourceConfig(dms: DataModelStruct): ?SourceConfig {
        let srcDirs = visitField(dms, SOURCE_SET_SRC_DIR_TOML, visitSourceDirectories)
        let conditions = visitField(dms, SOURCE_SET_CONDITION_TOML, visitConditions)

        SourceConfig(srcDirs ?? return None, conditions: conditions ?? return None)
    }

    // Parsing of `conditions` field in source set declaration which specifies enable constrains on source set
    // 
    // If source set declaration doesn't contain `condition` field - no constraints are put
    // 
    // Possible syntax:
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 1. Array literal of strings - e.g. AND (&&)
    // 
    // All features should be satisfied for source set to be enabled
    // 
    // [source-set.${source set fully qualified name}]
    //   condition = ["feature.arch.little", "feature.env.ohos", "feature.os.linux"]
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 2. Table of array literals - e.g. OR (||)
    // 
    // Source set should match with at least one array literal in the table.
    // key of `condition` table can be any alphanumeric identifier
    // 
    // [source-set.${source set fully qualified name}]
    //   condition.1     = ["feature.arch.little"]
    //   condition.alpha = ["feature.env.ohos"]
    //   condition.beta  = ["feature.os.linux", "feature.os.posix"]  // Has to satisfy both features
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // NOTE: All string literals represent valid feature identifiers
    // 
    // NOTE: This field is forbidden to be mentioned for `common/other` source sets
    protected open func visitConditions(dm: DataModel): ?Conditions {
        match (dm) {
            case dmseq: DataModelSeq => Conditions(dmseqToFeatures(dmseq) ?? return None)
            case dms: DataModelStruct => Conditions(visitConditionTable(dms) ?? return None)
            case _: DataModelNull => Conditions()
            case _ =>
                errors.add("`${ctx.fieldFqName}` has to be either an array of features or table of arrays")
                return None
        }
    }

    // Parsing of table of array literals
    private func visitConditionTable(dms: DataModelStruct): ?Array<HashSet<Feature>> {
        let result = dms.names |> filterMap {
            name: String => visitField(dms, name, dmseqToFeatures)
        } |> collectArray

        // Something got filtered out due to an error
        if (dms.names.size != result.size) {
            return None
        }

        return result
    }

    // Parsing of array literal of strings
    private func dmseqToFeatures(dm: DataModel): ?HashSet<Feature> {
        let dmseq = match (dm) {
            case data: DataModelSeq => data
            case _ =>
                errors.add("`${ctx.fieldFqName}` has to be an array of features")
                return None
        }

        let result = dmseq.getItems() |>
            filterMap {
            it: DataModel => if (let Some(dmstr) <- (it as DataModelString)) {
                if (let Some(feature) <- Feature.fromStringOp(dmstr.getValue())) {
                    feature
                } else {
                    errors.add("Error in `${ctx.fieldFqName}`: unknown feature `${dmstr.getValue()}`")
                    Option<Feature>.None
                }
            } else {
                errors.add("Error in `${ctx.fieldFqName}`: feature should be string literal")
                Option<Feature>.None
            }
        } |> collectHashSet

        // Something got filtered out
        if (result.size != dmseq.getItems().size) {
            return None
        }
        return result
    }

    // Parsing of `src-dir` field in source set declaration which specifies path to directories
    // 
    // Possible syntax:
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 1. String literal
    // 
    // Plain and simple
    // 
    // [source-set.${source set fully qualified name}]
    //   src-dir = "./src/linux/common"
    // 
    // --------------------------------------------------------------------------------------------------------
    // 
    // 2. Array literal of strings
    // 
    // All resources or specified directories will be collected as single compilation unit as if it was single directory
    // 
    // [source-set.${source set fully qualified name}]
    //   src-dir = ["./src/linux/dirA", "./src/linux/dirB"]
    // 
    // --------------------------------------------------------------------------------------------------------
    protected func visitSourceDirectories(dm: DataModel): ?SourceDirectories {
        match (dm) {
            case dms: DataModelString => SourceDirectories(dms.getValue())
            case dmseq: DataModelSeq =>
                let arr = Array<String>.deserialize(dmseq)
                if (arr.size == 0) {
                    errors.add("`${ctx.fieldFqName}` is not specified (empty array)")
                    return None
                } else {
                    SourceDirectories(arr)
                }
            case _: DataModelNull =>
                errors.add("`${ctx.fieldFqName}` is not specified")
                return None
            case _ =>
                errors.add("`${ctx.fieldFqName}` has to be either string literal or an array literal of strings")
                return None
        }
    }
}

// Structure representation of `src-dir` field of source-set, configured in `cjpm.toml` with error handling
protected struct SourceDirectories {
    public let sourceDirs: Array<String>

    init(sourceDirs: String) {
        this.sourceDirs = [sourceDirs]
    }

    init(sourceDirs: Array<String>) {
        this.sourceDirs = sourceDirs
    }

    public prop isSingleSource: Bool {
        get() {
            sourceDirs.size == 1
        }
    }
}

// Structure representation of `condition` field of source-set, configured in `cjpm.toml` with error handling
protected struct Conditions <: ToString {
    private let conditions: Array<HashSet<Feature>>

    init() {
        this.conditions = Array<HashSet<Feature>>(1) {
            _ => HashSet()
        }
    }

    init(conditions: Array<Feature>) {
        this.conditions = Array<HashSet<Feature>>(1) {
            _ => conditions |> collectHashSet
        }
    }

    init(conditions: HashSet<Feature>) {
        this.conditions = Array<HashSet<Feature>>(1) {
            _ => conditions
        }
    }

    init(conditions: Array<HashSet<Feature>>) {
        this.conditions = conditions
    }

    public func matches(features: Iterable<Feature>): Bool {
        conditions |> any {it: HashSet<Feature> => containsCount(it, features)}
    }

    private static func containsCount(target: HashSet<Feature>, features: Iterable<Feature>): Bool {
        assertion(
            {=> checkAllUnique(features)},
            {=> "Repetition occured: ${features |> collectArray}"}
        )
        (features |> filter {it: Feature => target.contains(it)} |> count) == target.size
    }

    public func toString() {
        let sb = StringBuilder()
        let iterator = conditions.iterator()
        var curr = iterator.next()
        if (let Some(condition) <- curr) {
            sb.append("[${condition |> collectString(delimiter: ", ")}]")
            curr = iterator.next()
        } else {
            sb.append("No conditions")
        }
        while (let Some(condition) <- curr) {
            sb.append(" OR");
            sb.append("\n");
            sb.append("[${condition |> collectString(delimiter: ", ")}]")
            curr = iterator.next()
        }
        sb.toString()
    }
}

// Union of possible types of source-set configuration, depending on surrounding context
protected enum SourceSet {
    | Group(GroupConfig)
    | Source(SourceConfig)

    public func matches(features: Iterable<Feature>): Bool {
        this.conditions.matches(features)
    }

    public prop conditions: Conditions {
        get() {
            match (this) {
                case Group(g) => g.conditions
                case Source(s) => s.conditions
            }
        }
    }
}

// Representation of source-sets which contain some nested source-set in them
// 
// source-set is automatically considered for grouping if non-reserved key is used in configuration
protected struct GroupConfig {
    GroupConfig(
        public let sourceSetDirs: SourceDirectories,
        public let conditions: Conditions,
        public let commonSet: SourceSet,
        public let otherSet: SourceSet,
        public let nestedSets: HashMap<String, SourceSet>
    ) {}
}

// Representation of source-sets which contain `.cj` files
// 
// source-set is automatically considered for grouping if non-reserved key is used in configuration
protected struct SourceConfig {
    SourceConfig(
        public let sourceDirs: SourceDirectories,
        public let conditions!: Conditions = Conditions()
    ) {}
}

public struct SourceSetsConfiguration {
    SourceSetsConfiguration(private let sourceSetRoot: SourceSet) {}

    public init() {
        this("${DIR_SRC}")
    }

    public init(src: String) {
        this(Source(SourceConfig(SourceDirectories(src))))
    }

    // @param path     System path to project root folder, which stores cjpm.toml
    // @param features List of features, used to filter out and select source sets
    // 
    // @return A pair of of folder groups behaving as a single folder (see. CMPSource & CopycatDir)
    public func selectSourceSets(path: String, features: ReadOnlyList<Feature>): CMPSource {
        let result = ArrayList<(String, SourceDirectories)>()
        let commonResult = ArrayList<(String, SourceDirectories)>()

        let queue = ArrayList<GroupConfig>()

        func placeSourceSet(it: SourceSet, name: String): Unit {
            match (it) {
                case Source(s) => result.add((name, s.sourceDirs))
                case Group(g) => queue.add(g)
            }
        }

        func placeCommonSourceSet(it: SourceSet, namePrefix: String): Unit {
            match (it) {
                case Source(s) => commonResult.add(("${namePrefix}.common", s.sourceDirs))
                case Group(_) => throw CliFailError("Ill-formed state, report if encountered such")
            }
        }

        func placeOtherSourceSet(it: SourceSet, namePrefix: String): Unit {
            match (it) {
                case Source(s) => result.add(("${namePrefix}.other", s.sourceDirs))
                case Group(_) => throw CliFailError("Ill-formed state, report if encountered such")
            }
        }

        // when Source - the project is regular
        // when Group  - the project is multiplatform
        placeSourceSet(this.sourceSetRoot, "package.src-dir")

        var parentSourceSet = "source-set"
        var root = true

        let errors = ArrayList<String>()

        while (queue.size > 0) {
            let front = queue.remove(at: 0)
            placeCommonSourceSet(front.commonSet, parentSourceSet)
            var selected: ?(String, SourceSet) = None
            var multiSelect = false
            for ((sourceSetName, sourceSet) in front.nestedSets) {
                if (sourceSet.matches(features)) {
                    if (let Some((selectedName, selectedSs)) <- selected) {
                        if (!multiSelect) {
                            errors.add(EnabledFeaturesPrinter(features).toString())
                            errors.add(
                                """
Multiple source sets selected on the same level: '${parentSourceSet}.${selectedName}'
Conditions: ${selectedSs.conditions}
"""
                            )
                            multiSelect = true
                        }
                        errors.add(
                            """
Multiple source sets selected on the same level: '${parentSourceSet}.${sourceSetName}'
Conditions: ${sourceSet.conditions}
"""
                        )
                    } else {
                        selected = (sourceSetName, sourceSet)
                    }
                }
            }

            if (let Some((name, cfg)) <- selected) {
                parentSourceSet += ".${name}"
                placeSourceSet(cfg, parentSourceSet)
            } else {
                if (root) {
                    errors.add(NoSourceSetWasSelectedError(features, path).toString())
                } else {
                    placeOtherSourceSet(front.otherSet, parentSourceSet)
                }
            }

            root = false
            if (errors.size != 0) {
                throw CliFailError(errors)
            }
        }

        return makeLeafCMPSource(result, commonResult, features, path: path)
    }
}

private func makeLeafCMPSource(
    iter: Iterable<(String, SourceDirectories)>,
    iterCommon: Iterable<(String, SourceDirectories)>,
    features: ReadOnlyList<Feature>,
    path!: String = "."
): CMPSource {
    let configPath = Path(path).join(CONFIG_FILE_NAME).toString()
    let errors = ArrayList<String>()
    let commonDirs: Array<(String, Path)> = iterCommon |>
        flatMap { it: (String, SourceDirectories) => it[1].sourceDirs |> map {srcDir => (it[0], srcDir)}} |>
        filterMap { it: (String, String) =>
        let (sourceSetName, pathStr) = it
        if (let Some(err) <- safeCheck_valueError(pathStr, configPath)) {
            errors.add(err)
            return Option<(String, Path)>.None
        }
        let basePath = Path("${path}/${pathStr}")
        try {
            let path = canonicalize(basePath)
            if (FileInfo(path).isDirectory()) {
                (sourceSetName, path)
            } else {
                Option<(String, Path)>.None
            }
        } catch (_: FSException) {
            (sourceSetName, basePath) // Non existant path to common should not be an issue, really
        }
    } |> collectArray
    let platformDirs: Array<(String, Path)> = iter |>
        flatMap {it: (String, SourceDirectories) => it[1].sourceDirs |> map {srcDir => (it[0], srcDir)}} |>
        map { it: (String, String) =>
        let (sourceSetName, pathStr) = it
        if (let Some(err) <- safeCheck_valueError(pathStr, configPath)) {
            errors.add(err)
            return (sourceSetName, Path(pathStr))
        }
        let srcDirPath = getPath(path, pathStr)
        if (!directoryExists(srcDirPath)) {
            errors.add("Error: path of src directory '${pathStr}' in ${configPath} does not exist")
            return (sourceSetName, Path(srcDirPath))
        }
        let curDirPath = getCanonicalPath(path)
        if (!srcDirPath.startsWith(curDirPath)) {
            errors.add(
                "Error: src file directory ${pathStr} must be contained in module directory. " +
                    "Please check field 'src-dir' in ${configPath}")
            return (sourceSetName, Path(srcDirPath))
        }
        (sourceSetName, Path(srcDirPath))
    } |> collectArray

    if (errors.size != 0) {
        errors.add(SourceLocationsPrinter(CMPSource(commonDirs, platformDirs), features).toString())
        throw CliFailError(errors)
    }
    CMPSource(commonDirs, platformDirs)
}

public enum CopycatInode {
    | F(FileInfo)
    | D(CopycatDir)
}

// Behaves like single directory while storing an Array<Path> of directories under the hood
// 
// @Field paths: Array<Path>
// 
// Example:
// 
// [CopycatDirRoot]
// ├── dirA
// ├── dirB
// │   └── pkg2
// │       └── pkg3
// └── dirC
//     ├── pkg1
//     └── pkg2
// 
// Let's say the set of directories {dirA, dirB, dirC} forms CopycatDir
// 
// Then, after entering directory `pkg2` we get:
// 
// [CopycatDirRoot]
// ├── dirB/pkg2
// │   └── pkg3
// └── dirC/pkg2
// 
// And can access files of all of the following directories
// As if they are located in the same directory
// 
// Note: files are still different inspite having same names
public struct CopycatDir <: Iterable<CopycatInode> & Serializable<CopycatDir> {
    public static const empty: CopycatDir = CopycatDir()

    CopycatDir(public let paths: Array<Path>) {}

    private const init() {
        this.paths = Array()
    }

    public init(paths: Array<String>) {
        this(paths |> map {it: String => Path(it)} |> collectArray)
    }

    public prop actualPaths: Array<String> {
        get() {
            paths |> map {it: Path => it.toString()} |> collectArray
        }
    }

    private static func groupPathsByFilename(paths: Iterable<Path>): HashMap<String, ArrayList<Path>> {
        let dirMap: HashMap<String, ArrayList<Path>> = HashMap()
        paths |> flatMap {
            it: Path => Directory.readFrom(it) |> filter {info => info.isDirectory()}
        } |>
            forEach {
            it: FileInfo =>
            let dirname = it.path.fileName
            if (!dirMap.contains(dirname)) {
                dirMap[dirname] = ArrayList<Path>()
            }
            dirMap[dirname].add(it.path)
        }
        dirMap
    }

    public func iterator(): Iterator<CopycatInode> {
        [this.files() |> map {it: FileInfo => F(it)}, this.dirs() |> map {it: CopycatDir => D(it)}] |> flatten
    }

    public func files(): Iterator<FileInfo> {
        paths |> flatMap {it: Path => tryReadDirectory(it) |> filter {info => info.isRegular()}}
    }

    public func dirs(): Iterator<CopycatDir> {
        let dirMap = groupPathsByFilename(paths)
        dirMap.values() |> map {it: ArrayList<Path> => CopycatDir(it.toArray())}
    }

    public func serialize(): DataModel {
        let result: ArrayList<DataModel> = paths |> map<Path, DataModel> {
            it: Path => DataModelString(it.toString())
        } |> collectArrayList

        DataModelSeq(result)
    }

    public static func deserialize(dm: DataModel): CopycatDir {
        let paths: Array<Path> = match (dm) {
            case dmseq: DataModelSeq => Array<String>.deserialize(dmseq) |> map {it: String => Path(it)} |> collectArray
            case dmstr: DataModelString => [Path(dmstr.getValue())]
            case _ => throw DataModelException("this data is not DataModelSeq and not DataModelString")
        }

        CopycatDir(paths)
    }
}

public struct CMPSource <: Serializable<CMPSource> {
    public static const empty = CMPSource()

    let _commonPaths: Array<(String, Path)>
    let _platformPaths: Array<(String, Path)>
    let _commonSource: CopycatDir
    let _platformSource: CopycatDir

    private const init() {
        this._commonPaths = Array()
        this._platformPaths = Array()
        this._commonSource = CopycatDir.empty
        this._platformSource = CopycatDir.empty
    }

    init(commonPaths: Array<(String, Path)>, platformPaths: Array<(String, Path)>) {
        this._commonPaths = commonPaths
        this._platformPaths = platformPaths
        this._commonSource = CopycatDir(commonPaths |> map {it: (String, Path) => it[1]} |> collectArray)
        this._platformSource = CopycatDir(platformPaths |> map {it: (String, Path) => it[1]} |> collectArray)
    }

    public prop commonPaths: Array<Path> {
        get() {
            let result = _commonSource.paths |>
                filterMap {
                it: Path => try {
                    canonicalize(it)
                } catch (_: FSException) {
                    None
                }
            } |> collectArray
            if (isMultiplatform && result.size == 0) {
                let sb = StringBuilder()
                sb.append("No common source directory exists in any of selected source sets, please specify one.\n")
                sb.append("Common sources:\n")
                sb.append(SourcesPrinter(_commonPaths).toString())
                throw CliFailError(sb.toString())
            }
            result
        }
    }
    public prop platformPaths: Array<Path> {
        get() {
            _platformSource.paths
        }
    }

    public prop allPaths: Array<Path> {
        get() {
            commonPaths |> concat(platformPaths) |> collectArray
        }
    }

    public prop isMultiplatform: Bool {
        get() {
            !_commonSource.paths.isEmpty() && !_platformSource.paths.isEmpty()
        }
    }

    public func serialize(): DataModel {
        let dmseqCommon = _commonPaths |>
            map {
            it: (String, Path) => DataModelStruct().add(field("_0", it[0])).add(field("_1", it[1].toString()))
        } |> collectDataModelSeq
        let dmseqPlatform = _platformPaths |>
            map {
            it: (String, Path) => DataModelStruct().add(field("_0", it[0])).add(field("_1", it[1].toString()))
        } |> collectDataModelSeq
        DataModelStruct().add(Field("commonPaths", dmseqCommon)).add(Field("platformPaths", dmseqPlatform))
    }

    public static func deserialize(dm: DataModel): CMPSource {
        match (dm) {
            case dms: DataModelStruct =>
                let dmseqCommon = ((dms
                    .get("commonPaths") as DataModelSeq) ?? throw DataModelException("this data is not DataModelSeq"))
                    .getItems() |>
                    map {
                    it: DataModel =>
                    let values = (it as DataModelStruct) ?? throw DataModelException("this data is not DataModelStruct")
                    let fst = (values.get("_0") as DataModelString)?.getValue() ?? throw DataModelException(
                        "this data is not DataModelString")
                    let snd = (values.get("_1") as DataModelString)?.getValue() ?? throw DataModelException(
                        "this data is not DataModelString")
                    (fst, Path(snd))
                } |> collectArray
                let dmseqPlatform = ((dms
                    .get("platformPaths") as DataModelSeq) ?? throw DataModelException("this data is not DataModelSeq"))
                    .getItems() |>
                    map {
                    it: DataModel =>
                    let values = (it as DataModelStruct) ?? throw DataModelException("this data is not DataModelStruct")
                    let fst = (values.get("_0") as DataModelString)?.getValue() ?? throw DataModelException(
                        "this data is not DataModelString")
                    let snd = (values.get("_1") as DataModelString)?.getValue() ?? throw DataModelException(
                        "this data is not DataModelString")
                    (fst, Path(snd))
                } |> collectArray
                CMPSource(dmseqCommon, dmseqPlatform)
            case _ => throw DataModelException("this data is not DataModelStruct")
        }
    }

    public func dirs(): Iterator<CMPSource> {
        let dirMap: HashMap<String, (ArrayList<(String, Path)>, ArrayList<(String, Path)>)> = HashMap()
        _commonPaths |>
            filterMap {
            it: (String, Path) => try {
                (it[0], canonicalize(it[1]))
            } catch (_: FSException) {
                None
            }
        } |>
            flatMap {
            it: (String, Path) => tryReadDirectory(it[1]) |> filter {info => info.isDirectory()} |>
                map {it2: FileInfo => (it[0], it2)}
        } |>
            forEach {
            it: (String, FileInfo) =>
            let dirname = it[1].path.fileName
            if (!dirMap.contains(dirname)) {
                dirMap[dirname] = (ArrayList<(String, Path)>(), ArrayList<(String, Path)>())
            }
            let (commonP, _) = dirMap[dirname]
            commonP.add((it[0], it[1].path))
        }

        _platformPaths |>
            flatMap {
            it: (String, Path) => tryReadDirectory(it[1]) |> filter {info => info.isDirectory()} |>
                map {it2: FileInfo => (it[0], it2)}
        } |>
            forEach {
            it: (String, FileInfo) =>
            let dirname = it[1].path.fileName
            if (!dirMap.contains(dirname)) {
                dirMap[dirname] = (ArrayList<(String, Path)>(), ArrayList<(String, Path)>())
            }
            let (_, platformP) = dirMap[dirname]
            platformP.add((it[0], it[1].path))
        }

        dirMap.values() |>
            map {
            it: (ArrayList<(String, Path)>, ArrayList<(String, Path)>) =>
            let (commonP, platformP) = it
            CMPSource(commonP.toArray(), platformP.toArray())
        }
    }

    public func files(): Iterator<FileInfo> {
        CopycatDir(allPaths).files()
    }
}

public struct SourceLocationsPrinter <: ToString {
    public SourceLocationsPrinter(
        private let source: CMPSource,
        private let features: Iterable<Feature>,
        private let project!: ?String = None
    ) {}

    public func toString() {
        let sb = StringBuilder()
        if (let Some(projectName) <- project) {
            sb.append("Source code locations for the multiplatform dependency `${projectName}`")
        } else {
            sb.append("Source code locations for the current multiplatform project.")
        }
        sb.append('\n')
        sb.append('\n')
        sb.append("Enabled features: ");
        sb.append(features |> collectString(delimiter: ", "));
        sb.append('\n')
        sb.append("Common sources:");
        sb.append('\n')
        sb.append("${SourcesPrinter(source._commonPaths)}");
        sb.append('\n')
        sb.append("Specific sources:");
        sb.append('\n')
        sb.append("${SourcesPrinter(source._platformPaths)}");
        sb.append('\n')
        sb.toString()
    }
}

private struct SourcesPrinter <: ToString {
    SourcesPrinter(private let source: Array<(String, Path)>) {}

    public func toString() {
        let maxSize: Int64 = (source |> map {it: (String, Path) => it[1].toString().size} |> max) ?? 0
        source |> map {it: (String, Path) => "  ${it[1].toString().padEnd(maxSize)}  [${it[0]}]"} |> collectString(
            delimiter: "\n")
    }
}

private struct EnabledFeaturesPrinter <: ToString {
    EnabledFeaturesPrinter(let features: ReadOnlyList<Feature>) {}

    public func toString() {
        "Enabled features: ${features |> collectString(delimiter: ", ")}"
    }
}

private struct NoSourceSetWasSelectedError <: ToString {
    NoSourceSetWasSelectedError(
        let features: ReadOnlyList<Feature>,
        let path: String
    ) {}

    public func toString() {
        let sb = StringBuilder()
        sb.append(EnabledFeaturesPrinter(features).toString());
        sb.append('\n')
        sb.append("No source set specified in ${Path(path).join(CONFIG_FILE_NAME).toString()} was selected");
        sb.append('\n')
        sb.toString()
    }
}

private func tryReadDirectory(path: Path) {
    if (!directoryExists(path.toString())) {
        throw CliFailError("Error: directory '${path}' does not exist or path is too long")
    }
    Directory.readFrom(path)
}
