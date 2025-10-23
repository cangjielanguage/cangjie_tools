type TB230<V = string, W extends 'abc' | 'def' = 'abc'> = (arg: V) => W;
type TB300 = Promise<number>;
type TB310 = Promise<boolean>[];