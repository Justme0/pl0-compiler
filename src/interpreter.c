#define _CRT_SECURE_NO_WARNINGS
#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "interpreter.h"

typedef struct Pair {
	char *key;
	enum fct value;
} Pair;

Pair fct_arr[] = {
	"cal", cal,
	"ckb", ckb,
	"int", Int,
	"jmp", jmp,
	"jp0", jp0,
	"jp1", jp1,
	"jpc", jpc,
	"lda", lda,
	"lit", lit,
	"lod", lod,
	"opr", opr,
	"ppa", ppa,
	"rva", rva,
	"sta", sta,
	"sto", sto
};

static int cmp(const void *key, const void *pair) {
	return strcmp((char *)key, ((Pair *)pair)->key);
}

/*
** s 是 fct 的字符串形式
*/
enum fct str2fct(char *s) {
	Pair *p = (Pair *)bsearch(
		s,
		fct_arr,
		sizeof fct_arr / sizeof *fct_arr,
		sizeof *fct_arr,
		cmp);
	assert(NULL != p);
	return p->value;
}

long base(long b, long l) {
	long b1 = b;
	while (l>0) {	// find base l levels down
		b1 = s[b1];
		--l;
	}
	return b1;
}

void interpret(instruction *code) {
	long pre_p = 0;	// 记录 pc
	long p = 0;		// program count
	long b = 1;		// base register
	long t = 0;		// topstack register(指向栈顶，非下一位置，s[0]不用)
	instruction i;	// instruction register	float fa = 0;	// 为操作浮点数
	long registr = 0;	// 放函数返回值

	long first = 0;	// for reverse arguments
	long last = 0;

	float fa = .0f;
	float fb = .0f;
	size_t fsize = sizeof(float);
	assert(sizeof(long) == sizeof(float));

	printf("start PL/0\n");
	s[1] = 0;
	s[2] = 0;
	s[3] = 0;
	do {
		i = code[p];
		pre_p = p;
		++p;
		switch(i.f) {
		case lit:
			++t;
			s[t] = i.a;	// a 表示字面值常量
			break;
		case opr:
			switch(i.a) { 	// operator
			case 0:	// return
				t = b-1;
				p = s[t+3];		// RA
				b = s[t+2];		// DL
				break;
			case 1:
				s[t] = -s[t];
				break;
			case 2:
				--t;
				s[t] += s[t+1];
				break;
			case 3:
				--t;
				s[t] -= s[t+1];
				break;
			case 4:
				--t;
				s[t] *= s[t+1];
				break;
			case 5:
				--t;
				s[t] /= s[t+1];
				break;
			case 6:
				s[t] %= 2;
				break;
			case 8:
				--t;
				s[t] = (s[t]==s[t+1]);
				assert(s[t] == 1 || s[t] == 0);
				break;
			case 9:
				--t;
				s[t] = (s[t]!=s[t+1]);
				assert(s[t] == 1 || s[t] == 0);
				break;
			case 10:
				--t;
				s[t] = (s[t]<s[t+1]);
				break;
			case 11:
				--t;
				s[t] = (s[t]>=s[t+1]);
				break;
			case 12:
				--t;
				s[t] = (s[t]>s[t+1]);
				break;
			case 13:
				--t;
				s[t] = (s[t]<=s[t+1]);
				break;

			case 14:
				printf("%ld ", s[t]);	// 暂时简单处理，直接输出一个空格 TODO:
				--t;
				break;

			case 15:
				printf("\n");
				break;

			case 16:
				//printf("请输入数据：");
				++t;
				scanf("%ld", &s[t]);	// 增加类型后，应区别输入格式字符串
				break;

			case 17:
				memcpy(&fa, &s[t], fsize);
				printf("%.2f ", fa);	// 默认保留2位小数
				break;

			case 18:
				memcpy(&fa, &s[t], fsize);
				fa = -fa;
				memcpy(&s[t], &fa, fsize);
				break;

			case 19:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				fa += fb;
				memcpy(&s[t], &fa, fsize);
				break;

			case 20:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				fa -= fb;
				memcpy(&s[t], &fa, fsize);
				break;

			case 21:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				fa *= fb;
				memcpy(&s[t], &fa, fsize);
				break;

			case 22:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				fa /= fb;
				memcpy(&s[t], &fa, fsize);
				break;

			case 23:
				++t;
				scanf("%f", &fa);
				memcpy(&s[t], &fa, fsize);
				break;

			case 24:
				scanf("%*[^\n]%*c");	// or scanf("%*[^\n]");	// 下次读入数据时 '\n' 会忽略
				break;

			case 26:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				s[t] = (fa < fb);
				break;

			case 27:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				s[t] = (fa >= fb);
				break;

			case 28:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				s[t] = (fa > fb);
				break;

			case 29:
				--t;
				memcpy(&fa, &s[t], fsize);
				memcpy(&fb, &s[t + 1], fsize);
				s[t] = (fa <= fb);
				break;

			case 30:
				fa = (float)(s[t]);	// INTEGER 和 REAL 都是32位，将 INTEGER 隐式转换成 REAL，可能丢失信息，下面同理
				memcpy(&s[t], &fa, fsize);
				break;

			case 31:
				fa = (float)(s[t - 1]);
				memcpy(&s[t - 1], &fa, fsize);
				break;

			case 32:
				--t;
				assert(s[t] == 1 || s[t] == 0);
				s[t] = s[t] || s[t + 1];
				assert(s[t] == 1 || s[t] == 0);
				break;

			case 33:
				--t;
				s[t] = s[t] && s[t + 1];
				break;

			case 34:
				s[t] = !s[t];
				break;

			case 35:
				--t;
				s[t] %= s[t + 1];
				break;

			case 36:
				registr = s[t--];
				break;

			case 37:
				s[++t] = registr;
				break;

			default:
				assert(!"未定义的操作指令");
				break;
			}
			break;

		case lod:
			++t;
			s[t] = s[base(b,i.l)+i.a];
			break;

		case sto:
			s[base(b,i.l)+i.a] = s[t];
			//	printf("%10d\n", s[t]);
			--t;
			break;

		case lda:
			s[t] = s[base(b, i.l) + i.a + s[t]];
			break;

		case sta:	// store array element
			s[base(b, i.l) + i.a + s[t - 1]] = s[t];
			t -= 2;
			break;

		case cal:		// generate new block mark
			s[t+1] = base(b,i.l);	// SL
			s[t+2] = b;				// DL
			s[t+3] = p;				// RA
			b = t+1;				// base 在 SL 的位置，都是 t+1
			p = i.a;				// callee 的入口地址
			break;

		case Int:
			t += i.a;
			break;

		case jmp:
			p = i.a;
			break;
		case jpc:
			if (s[t]==0) {
				p = i.a;
			}
			--t;
			break;

		case jp0:
			if (s[t] == 0) {
				p = i.a;
			}
			break;

		case jp1:
			if (s[t] != 0) {
				p = i.a;
			}
			break;

		case ckb:
			if (!(s[t-1] <= s[t-2] && s[t-2] < s[t])) {
				printf("ArrayIndexOutOfBoundsException at instructon %ld\n", pre_p);
				system("pause");
				exit(1);
			}
			t -= 2;
			break;

		case ppa:
			t -= i.a;
			break;

		case rva:
			first = t - i.a + 1;
			last = t + 1;
			if (last - first >= 2) {
				for (; first < (last--); ++first) {
					long tmp = s[first];
					s[first] = s[last];
					s[last] = tmp;
				}
			}

			break;

		default:
			assert(!"未定义的操作指令");
			break;
		}
	} while (p!=0);
	printf("end PL/0\n");
}

int main(int argc, char **argv) {
	instruction code[cxmax+1];
	long i = 0;
	char s_fct[INSTRUCTION_FUN_LEN + 1];
	FILE *fin = NULL;

	printf("Please input intermediate representation file name:\n");
	scanf("%s", infilename);
	fin = fopen(infilename, "r");
	if (fin == NULL) {
		printf("Can't open file %s.\n", infilename);
		system("pause");
		exit(1);
	}
	for (i = 0; i <= cxmax && EOF != fscanf(fin, "%s", s_fct); ++i) {
		code[i].f = str2fct(s_fct);
		fscanf(fin, "%ld%ld", &(code[i].l), &(code[i].a));
	}
	if (i > cxmax) {
		printf("program too long\n");
		system("pause");
		exit(1);
	}
	
	interpret(code);

	fclose(fin);
	system("pause");
	return 0;
}