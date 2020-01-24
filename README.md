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
- [x] S : ECHO "(" VAR ")" | ASS | VAL | FuncDef | Funcall
- [x] ASS : ID | ID "=" VAL
- [x] VAL : STR | E | BOOL | Funcall

### 计算表达式

- [x] E : T "+" E | T "-" E | T
- [x] T : F "*" T | F "/" T | F
- [x] F : "-" P | P "**" P | P
- [x] P : NUM | ID | "(" E ")"

### 块

- [x] Block : "{" SS "}" | "{" "}"

### 列表

- [x] Args : "(" VAL{"," VAL} ")" | "(" ")"
- [x] Params : "(" ID{"," ID} ")" | "(" ")"

### 条件

- [ ] Condition :  BOOLEXPR {LogicalOp BOOLEXPR}
- [ ] BOOLEXPR :  BOOL | E ComparisonOp E
- [ ] ComparisonOp : "==" | ">" | "<" | ">=" | "<="
- [ ] LogicalOp : "&&" | "||" | "!"

### 循环分支结构

- [ ] If : "if" "(" Condition ")" Block ElseIfList
- [ ] ElseIfList ：$ | Else | ElseIf ElseIfList
- [ ] ElseIf : "elif" "(" Condition ")"  Block
- [ ] Else : "else" Block
- [ ] Lable : ID ":"
- [ ] Goto : "goto" : id

### 函数

- [x] FuncDef : "func" ID Params BLOCK
- [x] FunCall : ID Args
