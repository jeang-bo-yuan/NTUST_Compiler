# Type

## Array

Example：

```d
// 1 維
int arr[10];

// 2 維
int arr[10][20];
```

最高支援 15 維陣列。每一維的大小必需是常數。

## Const

const variable 必須在定義時給定初始值，且定義後不能再修改。

唯一不用給初始值的情況是：const function parameter。函數的參數會在呼叫時才賦值，不過const function parameter同樣在賦值後不能再被修改。

## 初始值

global non-const variable 和 const variable 的初始值一定要是「編譯時期常數」。

local non-const variable 的初始值可以是任意 expression。


# 運算

## Operator

- 所有二元運算子，左右運算元要有***相同型別***
- ***所有*** 代表 int, float, double, string, bool 及其陣列型別

Operator | 接受型別 | 備註
---|---|---|
`= ` | ***所有*** | 左運算元要是 lvalue
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
prefix `++`  | int | 運算元要是 lvalue
prefix `--`  | int | 運算元要是 lvalue
postfix `++` | int | 運算元要是 lvalue
postfix `--` | int | 運算元要是 lvalue

# Statement

```
expression;

print expression;
println expression;

read lvalue;

return;
return expression;

{
    statements...
}
```

控制結構

```
if (bool_expression)
    ...
else
    ...

while (bool_expression)
    ...

for (expression ; bool_expression ; expression)
    ...

foreach (identifier : int_expression .. int_expression)
    ...
```
