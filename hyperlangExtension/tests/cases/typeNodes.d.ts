// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// These interfaces are used for testing type syntax of the typescript compiler.
// Properties are named after the syntax kinds as in tsc.
interface BasicTypes {
	numberKeyword: number;
	stringKeyword: string;
	booleanKeyword: boolean;
	bigintKeyword: bigint;
	objectKeyword: object;
	symbolKeyword: symbol;
	voidKeyword: void;
	undefinedKeyword: undefined;
	anyKeyword: any;
	unknownKeyword: unknown;
	neverKeyword: never;
}

interface FunctionTypes {
	functionType1: () => void;
	functionType10: <V>(arg: V) => void;
	functionType11: <V, W extends string>(arg: V) => W;
	functionType12: <V = object, W extends string = '123' | '345'>(arg: V) => W; // type parameter default values are unsupported
	functionType20: (...a: number[]) => void;
	// functionType21: (...a?: number[]) => void; // illegal
	functionType30: (a?: number) => void;
}

interface ComplexTypes<T, U extends string> {
	typeReference10: U;
	typeReference20: ComplexTypes<T, U>;
	typeReference21: Promise<T>;
	typeReference30: Pick<ComplexTypes<T, U>, 'literalType1'>;

	literalType1: 123; // numeric literal
	literalType2: "abc"; // string literal
	literalType3: null; // null is treated as literal type rather than a bare null keyword

	arrayType1: number[];
	arrayType2: U[];

	unionType: number | U;
	intersectionType: object & Record<U, T>;

	// parenthesized types are sometimes necessary in program text, but never in ast
	parenthesizedType1: (any);
	parenthesizedType2: (U | number)[];

	tupleType: [number, number, string];

	typeLiteral10: { x: number; y: U; };
	typeLiteral20: { [p: number]: string; [p: symbol]: T };
	typeLiteral30: { (): void; (number): string };

	typeOperator1: keyof ComplexTypes<T, U>;
	typeOperator10: readonly number[];

	// type queries are evaluated by type inference, resulting in the actual type
	typeQuery: typeof setTimeout;

	// mapped types could be evaluated to another type by type inference
	mappedType10: { [k in keyof ComplexTypes<T, U>]: number };
	mappedType20: { [k in keyof Promise<U>]: number };
}

interface NestedTypes {
	nested10: {
		x: { value: number; unit: string; } | number;
		y: { value: number; unit: string; } | number;
	};

	nested20: ComplexTypes<Record<string, (x: number) => Promise<string>>, string>;

	nested21: ComplexTypes<Record<string, <T>(x: T) => Promise<string>>, string>;
	 
	nested22: ComplexTypes<Record<string, <T>(x?: T) => Promise<[string, string]>>, string>;
}

interface Optionals {
	optionalField1?: number;
	optionalParam10: (a: number, b?: string) => void;
}

interface InferredTypes {
	// this is evaluated to never type
	inferred1: number & string;
	inferred10: ReturnType<typeof setTimeout>;
}

interface UnsupportedTypes {
	// the following types are hard to support, or can not easily be handled
	unsupported10: { [123]: string; }; // literal type with a property whose name is an expression (numeric literal)
	unsupported20: { ['234']: number; }; // literal type with a property whose name is an expression (string literal)
	unsupported30: { [Symbol.toStringTag]: number }; // literal type with a property whose name is an expression (property access expression)
}

interface ArrayTypesI {
    arr10: Array<Error>;
    arr11?: Array<Error>;
    arr20: number[];
    arr21?: number[];
}

declare class ArrayTypesC {
    arr10: Array<Error>;
    arr11?: Array<Error>;
    arr20: number[];
    arr21?: number[];
}

declare function returnType10(): any;
