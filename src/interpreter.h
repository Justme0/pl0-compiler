#ifndef INTERPRETER_H__
#define INTERPRETER_H__

#define cxmax 2000 // size of code array

enum fct {
  lit,
  opr,
  lod,
  sto,
  cal,
  Int,
  jmp,
  jpc, // 栈顶为0时转，跳转后栈顶值就没用了
  jp0, // 栈顶为0时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要
  jp1, // 栈顶为1时转，此时 Boolean 表达式的值就是栈顶值，跳转后还需要
  lda, // load for a[i]
  sta, // store for a[i]
  ckb, // check bounds, 单独作为一条指令可能不合适
  ppa, // pop arguments, 函数调用完成后栈顶指针恢复到参数压栈前的位置，address
       // 域为参数个数
  rva  // reverse arguments(真正实现时可能不用这样)，address 域为参数个数
};
//#define INSTRUCTION_NUM			15
#define INSTRUCTION_FUN_LEN 3
typedef struct {
  enum fct f; // function code
  long l;     // level
  long a;     // displacement address
              // 	long src_line;	// 出现 RTE 时指出源程序位置
} instruction;
/*
lit 0, a : load constant a
opr 0, a : execute operation a
lod l, a : load variable l, a
sto l, a : store variable l, a
cal l, a : call procedure a at level l
Int 0, a : increment t-register by a
jmp 0, a : jump to a
jpc 0, a : jump conditional to a when stack top is 0, and --t
jp0 0, a : jump conditional to a when stack top is 0
jp1 0, a : jump conditional to a when stack top is 1
*/

// the following array space for interpreter
#define stacksize 50000
long s[stacksize]; // datastore

#define MAX_FILE_NAME_LEN 80
char infilename[MAX_FILE_NAME_LEN];

#endif