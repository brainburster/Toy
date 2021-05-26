# Toy

尝试不利用第三方工具，写一个玩具语言的纯手工解释器

## wasm版本

  <https://brainburster.github.io/Toy/wasm/toy.html>

## 终结符

- ID 标识符
- NUM 数字
- STR 字符串
- BOOL 布尔值

## 非终结符

### 语句

- [x] SS : {S}
- [x] S : ECHO "(" BE ")" | ASS | BE | FuncDef | Funcall | If | Loop | Ret
- [x] ASS : ID "=" VAL

### 计算表达式

- [x] VAL : STR | E | BOOL | Funcall | AT | ARRAY
- [x] OP : "+" | "-" | "*" | "/" | "**"
- [x] E : T { OP T }
- [x] T : -p | p
- [x] P : NUM | ID | "(" BE ")"

### 块

- [x] Block : "{" SS "}" | "{" "}"

### 列表

- [x] Args : "(" VAL{"," VAL} ")" | "(" ")"
- [x] Params : "(" ID{"," ID} ")" | "(" ")"

### 数组

- [x] Array : "[" VAL{"," VAL} ")" | "[" "]"
- [x] At : ID "[" E "]"

### 布尔表达式

- [x] BE : BT "&&" BE | BT
- [x] BT : BF "||" BT | BF
- [x] BF : "!" BP | BP
- [x] BP : VAL [ ComOp VAL ]
- [x] ComOp : "==" | ">" | "<" | ">=" | "<=" | "!="

### 循环分支结构

- [x] If : "if" "(" BE ")" Block [ ElseIfList ]
- [x] ElseIfList ：Else | ElseIf ElseIfList
- [x] ElseIf : "elif" "(" BE ")"  Block [ ElseIfList ]
- [x] Else : "else" Block
- [x] Loop : "loop" "(" BE ")" Block
- [x] Ret : "return" {E};

### 函数

- [x] FuncDef : "func" ID Params BLOCK
- [x] FunCall : ID Args

## 例子

![例子](/example.png)
