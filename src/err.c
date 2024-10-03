#include "err.h"

#include <assert.h>

#define kMaxErrNum 100
const char* err_msg[kMaxErrNum];
const char* err_msg_chinese[kMaxErrNum];

void initErrNum() {
  err_msg[1] = "Should be '=' instead of ':='";
  err_msg_chinese[1] = "应为=而不是:=";
  err_msg[2] = "A number should follow '='";
  err_msg_chinese[2] = "=后应为数";
  err_msg[3] = "An identifier should be followed by '='";
  err_msg_chinese[3] = "标识符后应为=";
  err_msg[4] =
      "'const', 'type', 'var', 'procedure', 'function' should be followed by "
      "an identifier";
  err_msg_chinese[4] = "const, type, var, procedure, function 后应为标识符";
  err_msg[5] = "Missing comma or semicolon";
  err_msg_chinese[5] = "遗漏逗号或分号";
  err_msg[6] = "Incorrect token after procedure declaration";
  err_msg_chinese[6] = "过程声明后的记号不正确";
  err_msg[7] = "Should be a statement";
  err_msg_chinese[7] = "应为语句";
  err_msg[8] = "Incorrect token after the statement part within a subprogram";
  err_msg_chinese[8] = "分程序内的语句部分后的记号不正确";
  err_msg[9] = "Should be a period";
  err_msg_chinese[9] = "应为句号";
  err_msg[10] = "Missing semicolon between statements";
  err_msg_chinese[10] = "语句之间漏分号";
  err_msg[11] = "Identifier not declared";
  err_msg_chinese[11] = "标识符未声明";
  err_msg[13] = "Should be the assignment operator ':='";
  err_msg_chinese[13] = "应为赋值运算符:=";
  err_msg[14] = "'call' should be followed by an identifier";
  err_msg_chinese[14] = "call后应为标识符";
  err_msg[15] = "'call' can only be followed by a procedure name";
  err_msg_chinese[15] = "call 后只能是过程名";
  err_msg[16] = "Should be 'then'";
  err_msg_chinese[16] = "应为then";
  err_msg[17] = "Should be a semicolon or 'end'";
  err_msg_chinese[17] = "应为分号或end";
  err_msg[18] = "Should be 'do'";
  err_msg_chinese[18] = "应为do";
  err_msg[19] = "Incorrect token after statement";
  err_msg_chinese[19] = "语句后的记号不正确";
  err_msg[21] = "Procedure identifiers cannot be used within an expression";
  err_msg_chinese[21] = "表达式内不可有过程标识符";
  err_msg[22] = "Missing parenthesis";
  err_msg_chinese[22] = "遗漏括号";
  err_msg[23] = "This token cannot follow a factor";
  err_msg_chinese[23] = "因子后不可为此记号";
  err_msg[24] = "Expression cannot start with this token";
  err_msg_chinese[24] = "表达式不能以此记号开始";
  err_msg[30] = "This number is too large";
  err_msg_chinese[30] = "这个数太大";
  err_msg[31] = "This constant or address offset is too large";
  err_msg_chinese[31] = "这个常数或地址偏移太大";
  err_msg[32] = "Program nesting level is too deep";
  err_msg_chinese[32] = "程序嵌套层次太多";
  err_msg[33] = "Format error, should be a right parenthesis";
  err_msg_chinese[33] = "格式错误，应是右括号";
  err_msg[34] = "Format error, should be a left parenthesis";
  err_msg_chinese[34] = "格式错误，应是左括号";
  err_msg[35] = "In 'read()', there should be a declared variable name";
  err_msg_chinese[35] = "read()中应是声明过的变量名";
  err_msg[36] =
      "A real number's decimal point must be followed by a non-empty string of "
      "digits";
  err_msg_chinese[36] = "实数的小数点后必须含非空数字串";
  err_msg[37] = "Missing colon when defining a variable or function";
  err_msg_chinese[37] = "定义变量或函数时缺少冒号";
  err_msg[38] = "Illegal type";
  err_msg_chinese[38] = "非法类型";
  err_msg[39] = "Type mismatch";
  err_msg_chinese[39] = "类型不匹配";
  err_msg[40] = "Redefinition";
  err_msg_chinese[40] = "重定义";
  err_msg[41] = "Operation not defined for this type of operand";
  err_msg_chinese[41] = "对该类型的操作数的操作未定义";
  err_msg[42] = "Integer division should use 'div'";
  err_msg_chinese[42] = "整数除法应该用 div";
  err_msg[43] = "The operand type of 'odd' should be 'integer'";
  err_msg_chinese[43] = "odd 的操作数类型应是 integer";
  err_msg[44] = "The operand type of 'not' should be 'Boolean'";
  err_msg_chinese[44] = "not 的操作数类型应是 Boolean";
  err_msg[45] =
      "The test condition of 'if/while' should be 'Boolean' (can be indicated "
      "after evaluating the expression, the indication position is offset)";
  err_msg_chinese[45] =
      "if/while的测试条件的类型应是 "
      "Boolean（在表达式计算后才能指出，指示位置有偏移）";
  err_msg_chinese[46] = "只能输入输出整型或实型数据";
  err_msg[46] = "Can only input/output integer or real data";
  err_msg_chinese[47] = "应是关键字 array，只允许给数组类型命名";
  err_msg[47] =
      "Should be the keyword 'array', only allowed for naming array types";
  err_msg_chinese[48] = "格式错误，应是左中括号";
  err_msg[48] = "Format error, should be a left square bracket";
  err_msg_chinese[49] = "应是无符号整数,作为数组的下标界限";
  err_msg[49] = "Should be an unsigned integer, as the array index boundary";
  err_msg_chinese[50] = "应是..";
  err_msg[50] = "Should be '..'";
  err_msg_chinese[51] = "格式错误，应是右中括号";
  err_msg[51] = "Format error, should be a right square bracket";
  err_msg_chinese[52] = "应是 of";
  err_msg[52] = "Should be 'of'";
  err_msg_chinese[53] = "数组下标应是整数";
  err_msg[53] = "Array index should be an integer";
  err_msg_chinese[54] = "标识符太长（最多为al=10个字符）";
  err_msg[54] = "Identifier too long (maximum of 10 characters)";
  err_msg_chinese[55] = "应是类型标识符";
  err_msg[55] = "Should be a type identifier";
  err_msg_chinese[56] = "非数组类型";
  err_msg[56] = "Non-array type";
  err_msg_chinese[57] = "标识符引用有误";
  err_msg[57] = "Incorrect identifier reference";
  err_msg_chinese[58] = "只能修改变量或返回函数值";
  err_msg[58] = "Can only modify variables or return function values";
  err_msg_chinese[59] = "应是标识符";
  err_msg[59] = "Should be an identifier";
  err_msg_chinese[60] = "实参数目太多";
  err_msg[60] = "Too many actual parameters";
  err_msg_chinese[61] = "实参数目太少";
  err_msg[61] = "Too few actual parameters";
  err_msg_chinese[62] = "实参与形参类型不匹配";
  err_msg[62] = "Actual parameter type does not match formal parameter type";
  err_msg_chinese[63] = "break 语句没有处于任何 while 语句中";
  err_msg[63] = "'break' statement not within any 'while' statement";
  err_msg_chinese[64] = "函数的参数类型只能是 integer, real 或 Boolean";
  err_msg[64] =
      "Function's parameter type can only be 'integer', 'real', or 'Boolean'";
  err_msg_chinese[65] = "函数的返回值类型能是 integer, real 或 Boolean";
  err_msg[65] =
      "Function's return value type can be 'integer', 'real', or 'Boolean'";
}

const char* getErrString(int errNum) {
  assert(0 <= errNum && errNum < kMaxErrNum);

  return err_msg_chinese[errNum];
}