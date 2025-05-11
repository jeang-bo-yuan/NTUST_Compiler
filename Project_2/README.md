# Feature

## Array

Example：

```d
// 1 維
int arr[10];

// 2 維
int arr[10][20];
```

最高支援 15 維陣列。每一維的大小必需是常數。

## Operator

- 所有二元運算子，左右運算元要有***相同型別***
- ***所有*** 代表 int, float, double, string, bool 及其陣列型別
- 除非特別標註，否則都支援「常數」的 identifier 出現在 expression

Operator | 接受型別 | 備註
---|---|---|
`= ` | ***所有*** | 左運算元要是「非常數」的 Identifier
LOGIC ||
`\|\|` | bool |
`&&` | bool |
`! ` | bool |
COMPARE ||
`< ` | int, float, double, string |
`<=` | int, float, double, string |
`==` | ***所有*** |
`>=` | int, float, double, string |
`> ` | int, float, double, string |
`!=` | ***所有*** |
ARITHMETIC ||
`+`  | int, float, double, string | 對字串而言為字串串接
`-`  | int, float, double
`*`  | int, float, double
`/`  | int, float, double
`%`  | int
unary `+` | int, float, double
unary `-` | int, float, double
INCR && DECR ||
prefix `++`  | int | 運算元要是「非常數」的 Identifier
prefix `--`  | int | 運算元要是「非常數」的 Identifier
postfix `++` | int | 運算元要是「非常數」的 Identifier
postfix `--` | int | 運算元要是「非常數」的 Identifier