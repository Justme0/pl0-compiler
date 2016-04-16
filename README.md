# 示例
本编译器可解释执行下面的程序（二分查找）

``` c
/* bsearch.pl0 */

type Arr = array[0..9] of integer; /* typedef an array */
var a: Arr; /* define a global variable */

/*
** Find value in [first, last).
** If found, return its index; otherwise return last index.
*/
function bsearch(first: integer; last: integer; value: integer): integer;
var i: integer; /* define local variables */
    ret: integer;
begin
    ret := last;
    while (first < last) do begin
        i := first + (last - first) div 2;
        if (a[i] = value) then begin
            bsearch := i;   /* similar to return */
        end else if (a[i] < value) then begin first := i + 1;
        end else begin
            last := i;
        end;
    end;

    bsearch := ret;
end;

/* main() */
begin
    a[0] := -3;
    a[1] := -2;
    a[2] := 0;
    a[3] := 1;
    a[4] := 1;
    a[5] := 7;
    a[6] := 8;
    a[7] := 11;
    a[8] := 22;
    a[9] := 27;

    write(bsearch(0, 10, 22));  /* 8 */
    write(bsearch(0, 10, -1));  /* 10 */
end.
```

# src 文件夹内容

| 程序名 | 说明 |
| --- | --- |
| compiler.c | 将源程序编译成中间代码，输出到文件，扩展名是 pl0c |
| interpreter.c | 解释执行中间代码，结果输出到屏幕 |
| pl0.c | 合并两者，输入源程序，输出结果到屏幕 |

# test 文件夹内容

若干 PL/0 程序，作为测试程序。

# doc 文件夹内容
文档说明

# 实现

* 数据栈中的每个单元是32位，下面是各种类型的数据的表示

| 类型 | 表示 |
| --- | --- |
| integer | 用32位有符号数补码表示 |
| real | 用 IEEE 754 标准，32位表示 |
| Boolean | 为32位，true的内存表示为0x00000001，false为0x00000000 |
| 地址（下标） | 用32位有符号数补码表示（应是正数） |

* 约定 odd，mod 只能对 integer 操作；对 Boolean 的关系操作只有 eql 与 neq，不能比较大小

# 编译错误编号及含义

| 错误编号 | 含义 |
| --- | --- |
| 1 | 应为=而不是:= |
| 2 | =后应为数 |
| 3 | 标识符后应为= |
| 4 | const, type, var, procedure, function 后应为标识符 |
| 5 | 遗漏逗号或分号 |
| 6 | 过程声明后的记号不正确 |
| 7 | 应为语句 |
| 8 | 分程序内的语句部分后的记号不正确 |
| 9 | 应为句号 |
| 10 | 语句之间漏分号 |
| 11 | 标识符未声明 |
| ~~12~~ | ~~不可向常量或过程赋值~~ |
| 13 | 应为赋值运算符:= |
| 14 | call后应为标识符 |
| 15 | call 后只能是过程名 |
| 16 | 应为then |
| 17 | 应为分号或end |
| 18 | 应为do |
| 19 | 语句后的记号不正确 |
| ~~20~~ | ~~应为关系运算符~~ |
| 21 | 表达式内不可有过程标识符 |
| 22 | 遗漏括号 |
| 23 | 因子后不可为此记号 |
| 24 | 表达式不能以此记号开始 |
| 30 | 这个数太大 |
| 31 | 这个常数或地址偏移太大 |
| 32 | 程序嵌套层次太多 |
| 33 | 格式错误，应是右括号 |
| 34 | 格式错误，应是左括号 |
| 35 | read()中应是声明过的变量名 |
| 36 | 实数的小数点后必须含非空数字串 |
| 37 | 定义变量或函数时缺少冒号 |
| 38 | 非法类型 |
| 39 | 类型不匹配 |
| 40 | 重定义 |
| 41 | 对该类型的操作数的操作未定义 |
| 42 | 整数除法应该用 div |
| 43 | odd 的操作数类型应是 integer |
| 44 | not 的操作数类型应是 Boolean |
| 45 | if/while的测试条件的类型应是 Boolean（在表达式计算后才能指出，指示位置有偏移） |
| 46 | 只能输入输出整型或实型数据 |
| 47 | 应是关键字 array，只允许给数组类型命名 |
| 48 | 格式错误，应是左中括号 |
| 49 | 应是无符号整数,作为数组的下标界限 |
| 50 | 应是.. |
| 51 | 格式错误，应是右中括号 |
| 52 | 应是 of |
| 53 | 数组下标应是整数 |
| 54 | 标识符太长（最多为al=10个字符） |
| 55 | 应是类型标识符 |
| 56 | 非数组类型 |
| 57 | 标识符引用有误 |
| 58 | 只能修改变量或返回函数值 |
| 59 | 应是标识符 |
| 60 | 实参数目太多 |
| 61 | 实参数目太少 |
| 62 | 实参与形参类型不匹配 |
| 63 | break 语句没有处于任何 while 语句中 |
| 64 | 函数的参数类型只能是 integer, real 或 Boolean |
| 65 | 函数的返回值类型能是 integer, real 或 Boolean |

# 新增指令含义

| 指令 | 含义 |
| --- | --- |
| jp0 | 栈顶为0时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要 |
| jp1 | 栈顶为1时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要 |
| lda | load for a[i] |
| sta | store for a[i] |
| ckb | check bounds, 单独作为一条指令可能不合适 |
| ppa | pop arguments, 函数调用完成后栈顶指针恢复到参数压栈前的位置，address 域为参数个数 |
| rva | reverse arguments（真正实现时可能不用这样），address 域为参数个数 |

# opr 指令的 address 域含义

| address | 含义 |
| --- | --- |
| 0 | 返回 |
| 1 | integer 取相反数 |
| 2 | integer +（--t, s[t] = s[t] op s[t+1]，下面同理） |
| 3 | integer - |
| 4 | integer * |
| 5 | integer div |
| 6 | integer odd |
| 8 | integer == <br /> or real ==（对实数的比较，只有内存表示相同时才认为相等，TODO: 此时应给个警告) <br /> or Boolean == |
| 9 | integer != <br /> or real != <br /> or Boolean != |
| 10 | integer < |
| 11 | integer >= |
| 12 | integer > |
| 13 | integer <= |
| 14 | 输出 integer |
| 15 | 输出换行 |
| 16 | 输入 integer |
| 17 | 输出 real |
| 18 | real 取相反数 |
| 19 | real + |
| 20 | real - |
| 21 | real * |
| 22 | real / |
| 23 | 输入 real |
| 24 | 读掉剩余字符（忽略当前输入行剩余字符，下一个读语句接受的数据另起一行） |
| 26 | real < |
| 27 | real >= |
| 28 | real > |
| 29 | real <= |
| 30 | convert integer(s[t]) to real |
| 31 | convert integer(s[t-1]) to real |
| 32 | Boolean or |
| 33 | Boolean and |
| 34 | Boolean not |
| 35 | integer mod |
| 36 | 将栈顶值存到寄存器 |
| 37 | 从寄存器取值到栈顶 |
