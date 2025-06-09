# 額外功能

1. prefix ++, --
2. 有 double（沒f後綴）和 float（有f後綴）
3. break 和 continue
4. foreach

# Usage

```shell
# compile
make

# execute (read from stdin)
./parser

#execute (read from file)
./parser file
```

# Type

## Const

const variable 必須在定義時給定初始值，且定義後不能再修改。

唯一不用給初始值的情況是：const function parameter。函數的參數會在呼叫時才賦值，不過const function parameter同樣在賦值後不能再被修改。

## 初始值

global non-const variable 和 const variable 的初始值一定要是「編譯時期常數」。

local non-const variable 的初始值可以是任意 expression。


# 運算

## Operator

- 所有二元運算子，左右運算元要有***相同型別***
- ***所有*** 代表 int, float, double, ~~string~~, bool 及~~其陣列型別~~
> string 和 陣列型別被簡化掉了

Operator | 接受型別 | 備註
---|---|---|
`= ` | ***所有*** | 左運算元要是 lvalue
LOGIC ||
`\|\|` | bool |
`&&` | bool |
`! ` | bool |
COMPARE ||
`< ` | int, float, double, ~~string~~ |
`<=` | int, float, double, ~~string~~ |
`==` | ***所有*** |
`>=` | int, float, double, ~~string~~ |
`> ` | int, float, double, ~~string~~ |
`!=` | ***所有*** |
ARITHMETIC ||
`+`  | int, float, double, ~~string~~ | 對字串而言為字串串接
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

// Note: update_expression 在迴圈執行一輪後執行。即使有 continue 也會執行
for (init_expression ; bool_expression ; update_expression)
    ...

// Note: 先評估 I1 的值再評估 I2（只評估一次）
//       然後讓 i = 「I1 的結果」後，逐漸使 i 加／減1直到 i 變 「I2的結果」
//       foreach 是閉區間，也就是 i 是 I1 和 I2 的值都會進入迴圈
foreach (i : I1 .. I2)
    ...
```

在 while, for, foreach 內可以用 break 和 continue
