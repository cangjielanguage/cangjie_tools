interface Item<T> {
    id: number;
    name: string;
    category: T;
}

interface Item1<T, U> {
    id: number;
    name: U;
    category: T;
}