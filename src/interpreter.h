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
//#define INSTRUCTION_NUM			15
#define INSTRUCTION_FUN_LEN 3
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

// the following array space for interpreter
#define stacksize 50000
long s[stacksize]; // datastore

#define MAX_FILE_NAME_LEN 80
char infilename[MAX_FILE_NAME_LEN];

#endif