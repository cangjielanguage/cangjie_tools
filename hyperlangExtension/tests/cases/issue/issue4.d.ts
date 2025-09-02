//简单元组类型
export type BasicTuple = [number, string, boolean];

//元组类型的可选类型
export type OptionalElementTuple = [number, string?, boolean?];

//异构元组类型
export type HeterogeneousTuple = [string, number, boolean, Date];

//元组的剩余元素
export type RestTuple = [string, ...number[]];

//包含固定长度的元组
export type FixedLengthTuple = [number, string, boolean];

//元组与数组的结合
export type TupleArray = [number, string][];

//多维元组
export type TwoDimensionalTuple = [[number, string], [boolean, Date]];

//元组与联合类型结合
export type UnionTuple = [string | number, boolean, Date];