# 语法设计

## 终结符

- ID 标识符
- NUM 数字
- STR 字符串
- BOOL 布尔值

## 非终结符

### 语句

- [x] SS : {S}
- [x] S : ECHO "(" VAR ")" | ASS | VAL
- [x] ASS : ID | ID "=" VAL
- [ ] VAL : STR | E | BOOL | FuncDef | Funcall

### 计算表达式

- [x] E : T "+" E | T "-" E | T
- [x] T : F "*" T | F "/" T | F
- [x] F : "-" P | P "**" P | P
- [x] P : NUM | ID | "(" E ")"

### 块

- [ ] Block : "{" SS "}"

### 列表

- [ ] Args : "(" VAL{"," VAL} ")" | "(" ")"
- [ ] Params : "(" ID{"," ID} ")" | "(" ")"

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

### 函数

- [ ] FuncDef : "func" ID Params BLOCK
- [ ] FunCall : "func" ID Args
