# Toy

尝试不利用第三方工具，写一个玩具语言的纯手工解释器

## 终结符

- ID 标识符
- NUM 数字
- STR 字符串
- BOOL 布尔值

## 非终结符

### 语句

- [x] SS : {S}
- [x] S : ECHO "(" BE ")" | ASS | BE | FuncDef | Funcall | If
- [x] ASS : ID | ID "=" VAL
- [x] VAL : STR | E | Funcall

### 计算表达式

- [x] E : T "+" E | T "-" E | T
- [x] T : F "*" T | F "/" T | F
- [x] F : "-" P | P "**" P | P
- [x] P : NUM | ID | "(" BE ")"

### 块

- [x] Block : "{" SS "}" | "{" "}"

### 列表

- [x] Args : "(" VAL{"," VAL} ")" | "(" ")"
- [x] Params : "(" ID{"," ID} ")" | "(" ")"

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
- [ ] Lable : ID ":"
- [ ] Goto : "goto" : ID

### 函数

- [x] FuncDef : "func" ID Params BLOCK
- [x] FunCall : ID Args
