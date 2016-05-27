#include <stdio.h>
#include <limits.h>
#include <float.h>

typedef unsigned long long SymbolType;

#define norw 29   // no. of reserved words
#define txmax 100 // length of identifier table
#define nmax 14   // max. no. of digits in numbers
#define al 10     // length of identifiers
//#define amax       2047           // maximum address Ϊʲô��ַ�����2047��
#define amax LONG_MAX // TODO: ������Ҳ���� long �͵� num ��
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
  jpc, // ջ��Ϊ0ʱת����ת��ջ��ֵ��û����
  jp0, // ջ��Ϊ0ʱת����ʱ Boolean ���ʽ��ֵ����ջ��ֵ����ת����Ҫ
  jp1, // ջ��Ϊ1ʱת����ʱ Boolean ���ʽ��ֵ����ջ��ֵ����ת����Ҫ
  lda, // load for a[i]
  sta, // store for a[i]
  ckb, // check bounds, ������Ϊһ��ָ����ܲ�����
  ppa, // pop arguments, ����������ɺ�ջ��ָ��ָ�������ѹջǰ��λ�ã�address
       // ��Ϊ��������
  rva  // reverse arguments(����ʵ��ʱ���ܲ�������)��address ��Ϊ��������
};

#define INSTRUCTION_NUM 15
#define INSTRUCTION_FUN_LEN 3
char mnemonic[INSTRUCTION_NUM][INSTRUCTION_FUN_LEN +
                               1]; // ���������ָ������(fct2str)

typedef struct {
  enum fct f; // function code
  long l;     // level
  long a;     // displacement address
  // 	long src_line;	// ���� RTE ʱָ��Դ����λ��
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
long cc;    // character count, �ոն����ַ����±꣨����һλ�ã�
long ll;    // line length
long kk;    // ��¼�ϴ�ı�ʶ�����ȣ���ʼΪ��ʶ����󳤶�
long err;   // ����������Ŀ
long cx;    // code allocation index��ָ����һλ��

#define LINE_MAX_LEN 100
char line[LINE_MAX_LEN]; // ����һ�д���
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
DopeVec g_dope_vec; // �� enter()

typedef struct TypeRecord {
  char name[al + 1];
  DopeVec dope_vec; // ��Ҫ��չ�ɶ�ά���飬Ӧ����ָ��
  long size;        // ��λ�� 32 bit
  long level;
} TypeRecord; // �Ƿ��� level �����Ͷ���������򣩣�

TypeRecord type_table[txmax + 1]; // 0����˳�����ʱ���ڱ�
long type_tx;                     // ����һλ��

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
      para_link; // ���ڴ��εĺ�����������ָ���һ�������������βΣ�����ָ����һ��������ĩ��������Ϊ
                 // NULL��
  long para_num;
} Record;
Record
    table[txmax + 1]; // symbol table(0��λ����Ϊ position() ��˳����ҵ��ڱ�)

#define MAX_FILE_NAME_LEN 80
char infilename[MAX_FILE_NAME_LEN];
FILE *infile;

// the following variables for block
long lev; // current depth of block nesting
long tx;  // current table index(����һλ��)

// the following array space for interpreter
#define stacksize 50000
long s[stacksize]; // datastore

typedef struct BreakNode {
  long code_loc;
  struct BreakNode *next;
} BreakNode;

long g_loop_depth = 0;