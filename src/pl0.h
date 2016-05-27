#include <stdio.h>
#include <limits.h>
#include <float.h>

typedef unsigned long long SymbolType;

#define norw 29   // no. of reserved words
#define txmax 100 // length of identifier table
#define nmax 14   // max. no. of digits in numbers
#define al 10     // length of identifiers
//#define amax       2047           // maximum address 为什么地址最大是2047？
#define amax LONG_MAX // TODO: 将浮点也存在 long 型的 num 中
#define MAX_INTEGER LONG_MAX
#define MAX_FLOAT FLT_MAX
#define levmax 3   // maximum depth of block nesting
#define cxmax 2000 // size of code array

#define nul 0x1
#define ident 0x2
#define number 0x4
#define plus 0x8
#define minus 0x10
#define times 0x20
#define slash 0x40
#define oddsym 0x80
#define eql 0x100
#define neq 0x200
#define lss 0x400
#define leq 0x800
#define gtr 0x1000
#define geq 0x2000
#define lparen 0x4000
#define rparen 0x8000
#define comma 0x10000
#define semicolon 0x20000
#define period 0x40000
#define becomes 0x80000
#define colon 0x100000
#define endsym 0x200000
#define ifsym 0x400000
#define thensym 0x800000
#define whilesym 0x1000000
#define dosym 0x2000000
#define callsym 0x4000000
#define constsym 0x8000000
#define varsym 0x10000000
#define procsym 0x20000000
#define typesym 0x40000000
#define arraysym 0x80000000
#define ofsym 0x100000000
#define integersym 0x200000000
#define realsym 0x400000000
#define booleansym 0x800000000
#define funcsym 0x1000000000
#define elsesym 0x2000000000
#define writesym 0x4000000000
#define readsym 0x8000000000
#define breaksym 0x10000000000
#define orsym 0x20000000000
#define andsym 0x40000000000
#define notsym 0x80000000000
#define divsym 0x100000000000
#define modsym 0x200000000000
#define truesym 0x400000000000
#define falsesym 0x800000000000
#define beginsym 0x1000000000000
#define lbracket 0x2000000000000
#define rbracket 0x4000000000000
#define rangesym 0x8000000000000

enum object { CONSTANT, VARIABLE, PROC, FUNC, TAG };

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

#define INSTRUCTION_NUM 15
#define INSTRUCTION_FUN_LEN 3
char mnemonic[INSTRUCTION_NUM][INSTRUCTION_FUN_LEN + 1]; // 虚拟机代码指令名称

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

char ch;           // last character read
SymbolType sym;    // last symbol read
char g_id[al + 1]; // last identifier read

#define TRUE_VALUE 0x00000001
#define FALSE_VALUE 0x00000000
long g_num; // last number read
long cc;    // character count, 刚刚读的字符的下标（非下一位置）
long ll;    // line length
long kk;    // 记录较大的标识符长度，初始为标识符最大长度
long err;   // 编译错误的数目
long cx;    // code allocation index，指向下一位置

#define LINE_MAX_LEN 100
char line[LINE_MAX_LEN]; // 缓存一行代码
char a[al + 1];
instruction code[cxmax + 1];
char word[norw][al + 1]; // list of reserved word names
SymbolType wsym[norw];   // internal representation of reserved words
SymbolType ssym[256];    // list of special symbols(hashtable)
SymbolType declbegsys, statbegsys,
    facbegsys; // declaration, statement, factor begin symbol set

typedef struct TypeRecord *DataType;

typedef struct DopeVec {
  DataType elem_type;
  long lower_bound;
  long upper_bound; // [lower_bound, upper_bound)
  long n;           // n = upper_bound - lower_bound
  long const_offset;
} DopeVec;
DopeVec g_dope_vec; // 供 enter()

typedef struct TypeRecord {
  char name[al + 1];
  DopeVec dope_vec; // 若要扩展成多维数组，应该用指针
  long size;        // 单位是 32 bit
  long level;
} TypeRecord; // 是否有 level （类型定义的作用域）？

TypeRecord type_table[txmax + 1]; // 0号作顺序查找时的哨兵
long type_tx;                     // 非下一位置

#define INTEGER (type_table + 1)
#define REAL (type_table + 2)
#define BOOLEAN (type_table + 3)

typedef struct Record {
  char name[al + 1];
  enum object kind;
  DataType type;
  long val;
  long level;
  long addr;
  struct Record *
      para_link; // 对于带参的函数名，该域指向第一个参数；对于形参，该域指向下一个参数（末参数该域为
                 // NULL）
  long para_num;
} Record;
Record
    table[txmax + 1]; // symbol table(0号位置作为 position() 中顺序查找的哨兵)

char infilename[80];
FILE *infile;

// the following variables for block
long lev; // current depth of block nesting
long tx;  // current table index(非下一位置)

// the following array space for interpreter
#define stacksize 50000
long s[stacksize]; // datastore

typedef struct BreakNode {
  long code_loc;
  struct BreakNode *next;
} BreakNode;

long g_loop_depth = 0;
