// pl/0 compiler with code generation
#define _CRT_SECURE_NO_WARNINGS
#define NDEBUG
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pl0.h"

long g_line_num = 0;

void error(long n) {
  long i;

  printf("line %ld:\n", g_line_num);
  for (i = 1; i < ll; ++i) { // print err line in src code
    printf("%c", line[i]);
  }
  printf("\n");

  //	printf(" ****");
  //	for (i = 1; i<=cc-1; i++) {	// print position of err
  //		printf(" ");
  //	}
  for (i = 0; i < cc - 2; i++) { // print position of err
    printf(" ");
  }
  printf("^%2ld\n", n);
  err++;
}

void getch() {
  if (cc == ll) {
    if (feof(infile)) {
      printf("************************************\n");
      printf("      program incomplete\n");
      printf("************************************\n");
      //	system("pause");
      exit(1);
    }
    ll = 0;
    cc = 0;
    //	printf("%5ld ", cx);
    while ((!feof(infile)) && ((ch = (char)(getc(infile))) != '\n')) {
      // printf("%c",ch);
      ++ll;
      line[ll] = ch;
    }
    // printf("\n");
    ++ll;
    line[ll] =
        ' '; // ��ÿһ������'\n'��Ϊ' ', line[0] �����ַ���line[ll] �ſո�
    ++g_line_num;
  }
  assert(line[0] == '\0' && line[ll] == ' ');
  ++cc;
  ch = line[cc];
}

/*
 ** Ϊ����ʵ�� 0.5 �������±�� 0..5��������ǰ�鿴�����ַ�
 ** ����ǰ��ڶ����ַ��ǲ���'.'(��Ӱ�� cc)
 */
long next_is_period() {
  assert(line[cc] == '.');
  if (cc == ll) {
    return 0;
  }
  assert((cc + 1) <= ll);
  return line[cc + 1] == '.';
}

/*
 ** ���� type �Ƿ�Ϊ��������
 */
long is_arr(TypeRecord const *const type) {
  assert(type != type_table);
  if (type != NULL) {
    assert(type->dope_vec.elem_type != type_table);
  }
  assert(type_table < type &&
         type <= type_table + type_tx); // ������δ���壬�ⲿӦ����
  return type != NULL && type->dope_vec.elem_type != NULL;
}

/*
 ** ��ι���������ά���β���
 ** �������ͷŶ�̬���ɵ��β���
 */
void free_para_link(Record *begin, Record *end) {
  for (; begin != end; ++begin) {
    if ((begin->kind == PROC || begin->kind == FUNC) &&
        begin->para_link != NULL) {
      Record *p = begin->para_link; // p ָ���һ��������begin
                                    // ָ�������ͷ��㣨ͷ���Ƕ�̬���ɣ�
      Record *pre = NULL;           // ��סǰһ����㣬�����ͷ�
      assert(begin->para_num > 0);
      for (;;) {
        if (p->para_link == NULL) {
          free(p);
          p = NULL;
          break;
        } else {
          pre = p;
          p = p->para_link;
          free(pre);
          pre = NULL;
        }
      }
    }
  }
}

void free_break_link(BreakNode *head) {
  BreakNode *pre = NULL;
  BreakNode *p = NULL;
  if (NULL == head) {
    return;
  }
  p = head;
  for (;;) {
    if (p->next == NULL) {
      free(p);
      p = NULL;
      break;
    } else {
      pre = p;
      p = p->next;
      free(pre);
      pre = NULL;
    }
  }
}

/*
 ** concat list src to list dst
 */
BreakNode *list_cat(BreakNode *const dst, BreakNode *const src) {
  BreakNode *p = NULL;
  if (NULL == dst) {
    return src;
  }
  if (NULL == src) {
    return dst;
  }
  for (p = dst; p->next != NULL; p = p->next) {
  }
  p->next = src;

  return dst;
}

/*
 ** �ʷ�����
 ** ���ñ�����֮ǰ�� getch()
 */
DataType getsym() {
  long i, j, k;
  float real_num = 0; // 32λ������ num ���͡�����ջ���ʹ�С���
  float weight = 0;   // ���ڼ���ʵ����С������
  DataType sym_type =
      NULL; // �� sym �� number ʱ����������ʵ�������������򷵻�Ϊ NULL

LABEL_INIT:
  while (ch == ' ' || ch == '\t') {
    getch();
  }
  if (isalpha(ch)) { // identified or reserved
    assert(a[al] == '\0');
    k = 0; // k ��¼��ʶ���ĳ���
    do {
      if (k < al) {
        a[k] = ch; // ����ʶ�����ȴ���al������Ľ���ʧ
        ++k;
      } else {
        error(54); // ��ʶ��̫��
      }
      getch();
    } while (isalpha(ch) || isdigit(ch));
    if (k >= kk) {
      kk =
          k; // kk
             // ��ȫ�ֱ�������ʼ��al����ʶ����󳤶ȣ�����ִ�е����˵���˴α�ʶ������>=�ϴα�ʶ������
    } else {
      do {
        --kk;
        a[kk] = ' ';
      } while (k < kk);
    }
    strcpy(g_id, a);
    i = 0;
    j = norw - 1;
    do { // ���ֲ��� id �Ƿ�Ϊ�����֣����� bsearch()
      k = (i + j) / 2;
      if (strcmp(g_id, word[k]) <= 0) {
        j = k - 1;
      }
      if (strcmp(g_id, word[k]) >= 0) {
        i = k + 1;
      }
    } while (i <= j);
    if (i - 1 > j) {
      sym = wsym[k];
    } else {
      sym = ident;
    }
  } else if (isdigit(ch)) { // number
    k = 0;
    g_num = 0;
    sym = number;
    do {
      g_num = g_num * 10 + (ch - '0');
      ++k;
      getch();
    } while (isdigit(ch));
    sym_type = INTEGER;
    if ('.' == ch && !next_is_period()) {
      getch();
      assert(ch != '.');
      sym_type = REAL;
      real_num = (float)g_num;
      weight = 10;
      if (isdigit(ch)) {
        do {
          real_num += (ch - '0') / weight;
          weight *= 10;
          getch();
        } while (isdigit(ch));
        assert(sizeof(long) == sizeof(float));
        memcpy(&g_num, &real_num, sizeof(long));
      } else {
        error(36);
      }
    }
    if (k > nmax) {
      error(31);
    }
  } else if (ch == ':') {
    getch();
    if (ch == '=') {
      sym = becomes;
      getch();
    } else {
      sym = colon;
    }
  } else if (ch == '<') {
    getch();
    if (ch == '=') {
      sym = leq;
      getch();
    } else if (ch == '>') {
      sym = neq;
      getch();
    } else {
      sym = lss;
    }
  } else if (ch == '>') {
    getch();
    if (ch == '=') {
      sym = geq;
      getch();
    } else {
      sym = gtr;
    }
  } else if (ch == '/') {
    getch();
    if (ch == '*') {
    LABEL_COMMENT1:
      do {
        getch();
      } while (ch != '*');
    LABEL_COMMENT2:
      getch();
      if (ch == '/') {
        getch(); // ע������ת֮ǰ getch()
        goto LABEL_INIT;
      } else if (ch == '*') {
        goto LABEL_COMMENT2;
      } else {
        goto LABEL_COMMENT1;
      }
    } else {
      sym = slash; // ����б�ܺ��Ѿ� getch()�����ﲻ���� getch()
    }
  } else if (ch == '.') {
    getch();
    if (ch == '.') {
      sym = rangesym;
      getch();
    } else {
      sym = period;
    }
  } else {
    sym = ssym[(unsigned char)ch];
    getch();
  }

  return sym_type;
}

/*
 ** ָ���ʽ
 ** x:	������
 ** y:	Ƕ�ײ��
 ** z:	��ַ
 */
void gen(enum fct x, long y, long z) {
  if (cx > cxmax) {
    printf("program too long\n");
    // system("pause");
    exit(1);
  }
  code[cx].f = x;
  code[cx].l = y;
  code[cx].a = z;
  ++cx;
}

/*
 ** ��� sym(global) �Ƿ��� s1 �У����������� s2 ����
 ** s1:	��Ҫ�ķ��ż�
 ** s2:	�����õķ��ż�
 ** n:	�����
 */
void test(SymbolType s1, SymbolType s2, long n) {
  if (!(sym & s1)) {
    error(n);
    s1 = s1 | s2;
    while (!(sym & s1)) {
      getsym();
    }
  }
}

/*
 ** ������Ӧ���е��±�λ�ã����Ҳ��� id �򷵻�0
 */
long position(void *which_table, char *id) { // find identifier id in table
  long i = 0;

  if (which_table == (void *)table) {
    strcpy(table[0].name, id); // �ڱ�
    i = tx;
    while (strcmp(table[i].name, id) != 0) {
      --i;
    }
  } else if (which_table == (void *)type_table) {
    strcpy(type_table[0].name, id);
    i = type_tx;
    while (strcmp(type_table[i].name, id) != 0) {
      --i;
    }
  } else {
    i = 0;
    assert(!"����ķ��ű�");
  }

  return i;
}

void enter(char const *const sym_id, enum object k,
           DataType down_type) { // enter object into table
  long i;
  TypeRecord *ptrcd = NULL;
  // ͬһ���в������ض��壬��1�� tx �ĸ����ŵ� level �ǵ�����
  for (i = 1; i <= tx; ++i) {
    if (strcmp(table[i].name, sym_id) == 0 && table[i].level == lev) {
      error(40);
      return;
    }
  }
  for (i = 4; i <= type_tx; ++i) {
    if (strcmp(type_table[i].name, sym_id) == 0 && type_table[i].level == lev) {
      error(40);
      return;
    }
  }

  switch (k) {
  case CONSTANT:
    if (g_num > amax) {
      error(31);
      g_num = 0;
    }
    strcpy(table[++tx].name, sym_id);
    table[tx].kind = k;
    table[tx].val = g_num;
    table[tx].type = down_type;
    table[tx].para_link = NULL;
    table[tx].para_num = 0;
    break;

  case VARIABLE:
    // var a, b: real; ��������ֻ�е� real ����ʱ��֪�� a, b
    // �����ͣ��������͡���ַ�� dx ��������д TODO:
    strcpy(table[++tx].name, sym_id);
    table[tx].kind = k;
    table[tx].level = lev;
    table[tx].para_link = NULL;
    table[tx].para_num = 0;
    break;

  case PROC:
    strcpy(table[++tx].name, sym_id);
    table[tx].kind = k;
    table[tx].level = lev;
    table[tx].para_link = NULL;
    table[tx].para_num = 0;
    break;

  case FUNC:
    strcpy(table[++tx].name, sym_id);
    table[tx].kind = k;
    table[tx].level = lev;
    table[tx].para_link = NULL;
    table[tx].para_num = 0;
    // ���͵�ɨ�赽����ͷ�Ľ�β��֪����������� func_dec() �л���
    break;

  case TAG:
    strcpy(type_table[++type_tx].name, sym_id);
    type_table[type_tx].dope_vec =
        g_dope_vec; // g_dope_vec �����½��� type_expr() �л��
    type_table[type_tx].dope_vec.n = type_table[type_tx].dope_vec.upper_bound -
                                     type_table[type_tx].dope_vec.lower_bound;
    type_table[type_tx].size = type_table[type_tx].dope_vec.elem_type->size *
                               type_table[type_tx].dope_vec.n;
    type_table[type_tx].level = lev;
    for (ptrcd = &type_table[type_tx],
        ptrcd->dope_vec.const_offset = ptrcd->dope_vec.lower_bound;
         is_arr(ptrcd->dope_vec.elem_type); ptrcd = ptrcd->dope_vec.elem_type) {
      ptrcd->dope_vec.const_offset =
          ptrcd->dope_vec.const_offset * ptrcd->dope_vec.elem_type->dope_vec.n +
          ptrcd->dope_vec.elem_type->dope_vec.lower_bound;
    }

    break;

  default:
    assert(!"δ���������");
    break;
  }
}

void constdeclaration() {
  DataType sym_type = NULL;

  if (sym == ident) {
    getsym();
    if (sym == eql || sym == becomes) {
      if (sym == becomes) {
        error(1);
      }
      sym_type = getsym();
      if (sym == number) {
        enter(g_id, CONSTANT, sym_type);
        getsym();
      } else {
        error(2);
      }
    } else {
      error(3);
    }
  } else {
    error(4);
  }
}

void vardeclaration() {
  if (sym == ident) {
    enter(g_id, VARIABLE, NULL); // ������ block() �м�����ű�
    getsym();
  } else {
    error(4);
  }
}

void list_code(long cx0) { // list code generated for this block
  long i;

  for (i = cx0; i < cx; i++) {
    printf("%10ld%5s%3ld%15ld\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
  }
}

DataType expr(SymbolType fsys, DataType down_type);

/*
 ** IdentRef -> ident['['Exp']'{'['Exp']'}]
 ** ���� IdentRef ������
 ** pprcd:	ָ��ؼ���Ϊ ident �ļ�¼��ָ���ָ�룬���ڻص�λ��
 */
DataType ident_ref(SymbolType fsys, Record **pprcd) {
  DataType ident_ref_type = NULL;
  DataType sym_type = NULL;
  DataType subscript_type = NULL;
  long dim_num = 0;

  assert(pprcd != NULL);
  *pprcd = NULL;
  if (sym != ident) {
    error(59);
    return NULL;
  }
  *pprcd = &table[position(table, g_id)];
  if (*pprcd == table) {
    error(11);
    return NULL;
  }
  ident_ref_type = (*pprcd)->type;
  getsym();
  if (sym != lbracket) {
    return ident_ref_type; // ��������ֱ�ӷ���
  }
  do {
    ++dim_num;
    if (!is_arr(ident_ref_type)) {
      error(56); // �������ŵ���������
      return NULL;
    }

    sym_type = getsym();
    if (dim_num >= 2) {
      gen(lit, 0, (is_arr(ident_ref_type) ? ident_ref_type->dope_vec.n : 1));
      gen(opr, 0, 4);
    }
    subscript_type = expr(fsys | rbracket, sym_type); // ���±�ѹ��ջ
    if (subscript_type == INTEGER) {
      if (sym == rbracket) {
        gen(lit, 0, ident_ref_type->dope_vec.lower_bound);
        gen(lit, 0, ident_ref_type->dope_vec.upper_bound);
        gen(ckb, 0, 0);
        /*
           e.g. a[0..4], a[2]
           data stack:
           4
           0
           2
           */
        if (dim_num >= 2) {
          gen(opr, 0, 2);
        }
        ident_ref_type = ident_ref_type->dope_vec.elem_type;
        getsym();
      } else {
        error(51);
      }
    } else {
      error(53);
    }
  } while (sym == lbracket);

  return ident_ref_type;
}

DataType simp_exp(SymbolType, DataType);
DataType expr(SymbolType, DataType);
void actual_para_l(SymbolType fsys, DataType down_type, Record const *prcd);

/*
 ** fsys:	follow symbol set of factor
 */
DataType factor(SymbolType fsys, DataType down_type) {
  DataType sym_type = NULL;
  Record *prcd = NULL;
  DataType factor_type = NULL;

  test(facbegsys, fsys, 24);
  while (sym & facbegsys) { // ΪʲôҪѭ����
    if (sym == ident) {
      prcd = &table[position(table, g_id)];
      if (table == prcd) {
        error(11);
      } else if (prcd->kind == FUNC) {
        // ���������ܵ��� ident_ref���Դ�����ִ��
        // ������ǰ�鿴�������ţ�ǰ���Ǻ������ܷ������飩
        // Factor -> ident[(ActParal)]
        factor_type = prcd->type;
        getsym();
        if (lparen == sym) {
          sym_type = getsym();
          actual_para_l(fsys | rparen, sym_type, prcd);
          gen(rva, 0, prcd->para_num); // ��תʵ�Σ�ģ��ʵ�ֲ�������ѹջ
          gen(cal, lev - prcd->level, prcd->addr);
          gen(ppa, 0,
              prcd->para_num); // ջ��ָ��ָ�������ѹջǰ��λ�ã���Ӧʵ��ѹջ��
          if (rparen == sym) {
            getsym();
          } else {
            error(33);
          }
        } else {
          gen(cal, lev - prcd->level, prcd->addr);
        }
        gen(opr, 0, 37);
      } else {
        // Factor -> IdentRef
        factor_type = ident_ref(fsys, &prcd);
        if (factor_type != NULL && factor_type != type_table && prcd != NULL &&
            prcd != table) {
          switch (prcd->kind) {
          case CONSTANT:
            gen(lit, 0, prcd->val);
            break;

          case VARIABLE:
            if (is_arr(prcd->type)) {
              gen(lda, lev - prcd->level,
                  prcd->addr - prcd->type->dope_vec.const_offset);
            } else {
              gen(lod, lev - prcd->level, prcd->addr);
            }
            break;

          case PROC:
            error(21);
            break;

          default:
            assert(!"δ���������");
            break;
          }
        } else {
          error(57);
        }
      }
    } else if (sym == number) {
      factor_type = down_type; // �� factor() ���� getsym() �õ��� sym(number)
                               // �������� down_type
      if (g_num > amax) {
        error(31);
        g_num = 0;
      }
      gen(lit, 0, g_num);
      sym_type = getsym();
      assert(sym_type == NULL);
    } else if (sym == lparen) {
      sym_type = getsym();
      factor_type = expr((rparen | fsys), sym_type);
      if (sym == rparen) {
        sym_type = getsym();
        assert(sym_type == NULL);
      } else {
        error(22);
      }
    } else if (sym == notsym) {
      sym_type = getsym();
      factor_type = factor(fsys, sym_type);
      if (factor_type != BOOLEAN) {
        error(44);
        factor_type = BOOLEAN; // ���������Ƿ���ʣ�
      }
      gen(opr, 0, 34);
    } else if (sym == oddsym) {
      factor_type = BOOLEAN;
      getsym();
      if (sym == lparen) {
        DataType simp_exp_type;
        sym_type = getsym();
        simp_exp_type = simp_exp(rparen | fsys, sym_type);
        if (simp_exp_type == INTEGER) {
          if (sym == rparen) {
            sym_type = getsym();
            assert(sym_type == NULL);
            gen(opr, 0, 6);
          } else {
            error(22);
          }
        } else {
          error(43);
        }
      } else {
        error(22);
      }
    } else if (sym == truesym || sym == falsesym) {
      factor_type = BOOLEAN;
      gen(lit, 0, (sym == truesym ? TRUE_VALUE : FALSE_VALUE));
      sym_type = getsym();
      assert(sym_type == NULL);
    } else {
      assert(!"�Ƿ��� factor ��ʼ��");
    }
    test(fsys, lparen, 23); // ���ȼ����� facbegsys?
  }

  //	assert(factor_type != NULL);
  return factor_type;
}

DataType term(SymbolType fsys, DataType down_type) {
  SymbolType mulop;
  DataType sym_type = NULL;
  DataType factor_type1 = NULL;
  DataType factor_type2 = NULL;

  factor_type1 =
      factor(fsys | times | slash | divsym | modsym | andsym, down_type);
  while (sym == times || sym == slash || sym == divsym || sym == modsym ||
         sym == andsym) {
    long cx_temp = cx;
    if (sym == andsym) {
      gen(jp0, 0, 0);
    }

    mulop = sym;
    sym_type = getsym();
    factor_type2 =
        factor(fsys | times | slash | divsym | modsym | andsym, sym_type);

    if (factor_type1 == INTEGER && factor_type2 == INTEGER) {
      if (mulop == times) {
        gen(opr, 0, 4);
      } else if (mulop == divsym) {
        gen(opr, 0, 5);
      } else if (mulop == slash) {
        error(42);
      } else if (mulop == modsym) {
        gen(opr, 0, 35);
      } else {
        error(41);
      }
    } else if (factor_type1 == REAL && factor_type2 == REAL) {
      if (mulop == times) {
        gen(opr, 0, 21);
      } else if (mulop == slash) {
        gen(opr, 0, 22);
      } else {
        error(41);
      }
    } else if (factor_type1 == INTEGER && factor_type2 == REAL) {
      gen(opr, 0, 31);
      if (mulop == times) {
        gen(opr, 0, 21);
      } else if (mulop == slash) {
        gen(opr, 0, 22);
      } else {
        error(41);
      }
      factor_type1 = REAL;
    } else if (factor_type1 == REAL && factor_type2 == INTEGER) {
      gen(opr, 0, 30);
      if (mulop == times) {
        gen(opr, 0, 21);
      } else if (mulop == slash) {
        gen(opr, 0, 22);
      } else {
        error(41);
      }
    } else if (factor_type1 == BOOLEAN && factor_type2 == BOOLEAN) {
      if (mulop == andsym) {
        gen(opr, 0, 33);
        code[cx_temp].a = cx;
      } else {
        error(41);
      }
    } else {
      assert(!"�Ƿ�����");
    }
  }

  //	assert(NULL != factor_type1);
  return factor_type1;
}

/*
 ** down_type:	���� simp_exp() ǰ getsym() �ķ���ֵ����������ֵ
 */
DataType simp_exp(SymbolType fsys, DataType down_type) {
  SymbolType addop = 0;
  DataType sym_type = NULL; // ��¼�ֲ��� getsym() �ķ���ֵ�������ӹ���ʱ����ȥ
  DataType term_type1 = NULL;
  DataType term_type2 = NULL;

  if (sym == plus || sym == minus) {
    addop = sym;
    sym_type = getsym();
    term_type1 = term(fsys | plus | minus | orsym, sym_type);
    if (addop == minus) {
      gen(opr, 0, 1);
    }
  } else {
    term_type1 = term(fsys | plus | minus | orsym, down_type);
  }
  while (sym == plus || sym == minus || sym == orsym) {
    long cx_temp = cx;
    if (sym == orsym) {
      gen(jp1, 0, 0); // ��·
    }
    addop = sym;
    sym_type = getsym();
    term_type2 = term(fsys | plus | minus | orsym, sym_type);
    // Ӧ�� 3 or 2.0 �� true + true �����ı��ʽ�����ͼ��
    if (term_type1 == INTEGER && term_type2 == INTEGER) {
      if (addop == plus) {
        gen(opr, 0, 2);
      } else if (addop == minus) {
        gen(opr, 0, 3);
      } else {
        error(41);
      }
    } else if (term_type1 == REAL && term_type2 == REAL) {
      if (addop == plus) {
        gen(opr, 0, 19);
      } else if (addop == minus) {
        gen(opr, 0, 20);
      } else {
        error(41);
      }
    } else if (term_type1 == INTEGER && term_type2 == REAL) {
      gen(opr, 0, 31); // ��ʱ��ջ����INTEGER,ջ����REAL
      if (addop == plus) {
        gen(opr, 0, 19);
      } else if (addop == minus) {
        gen(opr, 0, 20);
      } else {
        error(41);
      }
      term_type1 = REAL;
    } else if (term_type1 == REAL && term_type2 == INTEGER) {
      gen(opr, 0, 30);
      if (addop == plus) {
        gen(opr, 0, 19);
      } else if (addop == minus) {
        gen(opr, 0, 20);
      } else {
        error(41);
      }
    } else if (term_type1 == BOOLEAN && term_type2 == BOOLEAN) {
      if (addop == orsym) {
        gen(opr, 0, 32);
        code[cx_temp].a = cx; // �����·
      } else {
        error(41);
      }
    } else {
      error(41);
    }
  }

  return term_type1; // ��ʾʽ�����;��������������
}

DataType expr(SymbolType fsys, DataType down_type) {
  SymbolType relop;
  DataType sym_type = NULL;
  DataType expr_type1 = NULL;
  DataType expr_type2 = NULL;

  expr_type1 = simp_exp(fsys | eql | neq | lss | gtr | leq | geq, down_type);
  if (sym & (eql | neq | lss | gtr | leq | geq)) {
    relop = sym;
    sym_type = getsym();
    expr_type2 = simp_exp(fsys, sym_type);

    switch (relop) {
    case eql:
      if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
      } else if (expr_type1 != expr_type2) {
        error(41);
      }
      gen(opr, 0, 8);
      break;

    case neq:
      if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
      } else if (expr_type1 != expr_type2) {
        error(41);
      }
      gen(opr, 0, 9);
      break;

    case lss:
      if (expr_type1 == INTEGER && expr_type2 == INTEGER) {
        gen(opr, 0, 10);
      } else if (expr_type1 == REAL && expr_type1 == REAL) {
        gen(opr, 0, 26);
      } else if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
        gen(opr, 0, 26);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
        gen(opr, 0, 26);
      } else {
        error(41); // ������ false < true �����ı��ʽ
      }
      break;

    case geq:
      if (expr_type1 == INTEGER && expr_type2 == INTEGER) {
        gen(opr, 0, 11);
      } else if (expr_type1 == REAL && expr_type2 == REAL) {
        gen(opr, 0, 27);
      } else if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
        gen(opr, 0, 27);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
        gen(opr, 0, 27);
      } else {
        error(41);
      }
      break;

    case gtr:
      if (expr_type1 == INTEGER && expr_type2 == INTEGER) {
        gen(opr, 0, 12);
      } else if (expr_type1 == REAL && expr_type2 == REAL) {
        gen(opr, 0, 28);
      } else if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
        gen(opr, 0, 28);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
        gen(opr, 0, 28);
      } else {
        error(41);
      }
      break;

    case leq:
      if (expr_type1 == INTEGER && expr_type2 == INTEGER) {
        gen(opr, 0, 13);
      } else if (expr_type1 == REAL && expr_type2 == REAL) {
        gen(opr, 0, 29);
      } else if (expr_type1 == INTEGER && expr_type2 == REAL) {
        gen(opr, 0, 31);
        gen(opr, 0, 29);
      } else if (expr_type1 == REAL && expr_type2 == INTEGER) {
        gen(opr, 0, 30);
        gen(opr, 0, 29);
      } else {
        error(41);
      }
      break;

    default:
      assert(!"δ������߼�����");
      break;
    }
    expr_type1 = BOOLEAN;
  }

  return expr_type1;
}

/*
 ** ActParal -> Exp{, Exp}
 ** prcd ָ����ű��еĺ�������¼
 */
void actual_para_l(SymbolType fsys, DataType down_type, Record const *prcd) {
  assert(NULL != prcd);
  for (;;) {
    DataType arg_type = expr(fsys | comma, down_type);
    if (prcd->para_link == NULL) {
      error(60); // ʵ����Ŀ̫��
    } else {
      prcd = prcd->para_link;
      if (prcd->type == REAL && arg_type == INTEGER) {
        gen(opr, 0, 30);
      } else if (prcd->type != arg_type) {
        error(62);
      }
      // gen(sto, lev - prcd->level, prcd->addr);	// callee
      // ֪��ʵ��λ�ã���ʱȥȡ����
    }
    if (comma == sym) {
      getsym();
    } else {
      if (NULL != prcd->para_link) {
        error(61); // ʵ����Ŀ̫��
      }
      break;
    }
  }
}

BreakNode *statement(SymbolType fsys) {
  DataType sym_type = NULL;
  DataType ident_ref_type = NULL;
  DataType expr_type = NULL;
  Record *prcd = NULL;
  long cx1 = 0;
  long cx2 = 0;
  BreakNode *pbn = NULL;

  if (sym == ident) {
    ident_ref_type = ident_ref(fsys | lbracket, &prcd);
    if (ident_ref_type != NULL && ident_ref_type != type_table &&
        prcd != NULL && prcd != table) {
      if (prcd->kind != VARIABLE && prcd->kind != FUNC) {
        error(58);
      }
    } else {
      error(57);
    }
    if (sym == becomes) {
      sym_type = getsym();
    } else {
      error(13);
    }
    expr_type = expr(fsys, sym_type);
    if (ident_ref_type == REAL && expr_type == INTEGER) {
      gen(opr, 0, 30);
    } else if (ident_ref_type != expr_type) {
      error(39);
    }
    if (prcd->kind == FUNC) {
      gen(opr, 0, 36); // ������ֵ�浽�Ĵ�����
      gen(opr, 0, 0);
    } else if (is_arr(prcd->type)) {
      // ���鳣�����ֱ���ʱ���
      gen(sta, lev - prcd->level,
          (prcd->addr - prcd->type->dope_vec.const_offset));
    } else {
      gen(sto, lev - prcd->level, prcd->addr);
    }
  } else if (sym == callsym) {
    getsym();
    if (sym != ident) {
      error(14);
    } else {
      // Stmt     -> call ident[(ActParal)]
      // ActParal -> Exp{, Exp}
      prcd = &table[position(table, g_id)]; // ָ��������¼
      assert(NULL != prcd);
      getsym();
      if (prcd == table) {
        error(11);
      } else if (prcd->kind == PROC) {
        if (lparen == sym) {
          sym_type = getsym();
          actual_para_l(fsys | rparen, sym_type, prcd);
          gen(rva, 0, prcd->para_num); // ��תʵ�Σ�ģ��ʵ�ֲ�������ѹջ
          gen(cal, lev - prcd->level, prcd->addr);
          gen(ppa, 0,
              prcd->para_num); // ջ��ָ��ָ�������ѹջǰ��λ�ã���Ӧʵ��ѹջ��
          if (rparen == sym) {
            getsym();
          } else {
            error(33);
          }
        } else {
          gen(cal, lev - prcd->level, prcd->addr);
        }
      } else {
        error(15);
      }
    }
  } else if (sym == ifsym) {
    sym_type = getsym();
    expr_type = expr(fsys | thensym | dosym, sym_type); // why dosym?
    if (BOOLEAN != expr_type) {
      error(45);
    }
    if (sym == thensym) {
      getsym();
    } else {
      error(16);
    }
    cx1 = cx;
    gen(jpc, 0, 0);
    pbn = statement(fsys | elsesym);
    if (sym == elsesym) {
      BreakNode *pbn2 = NULL;
      getsym();
      code[cx1].a = cx + 1; // cx ���� jmp
      cx1 = cx;
      gen(jmp, 0, 0);
      pbn2 = statement(fsys);
      pbn = list_cat(pbn, pbn2);
      code[cx1].a = cx; // ǰ���޸��� cx1
    } else {
      code[cx1].a = cx;
    }
  } else if (sym == beginsym) {
    getsym();
    pbn = statement(fsys | semicolon | endsym);
    while (sym == semicolon || (sym & statbegsys)) {
      BreakNode *pbn2 = NULL;
      if (sym == semicolon) {
        getsym();
      } else {
        error(10);
      }
      pbn2 = statement(fsys | semicolon | endsym);
      pbn = list_cat(pbn, pbn2); // �� pbn2 �ӵ� pbn ��β����pbn ԭ�������� NULL
    }
    if (sym == endsym) {
      getsym();
    } else {
      error(17);
    }
  } else if (sym == whilesym) {
    cx1 = cx;
    sym_type = getsym();
    expr_type = expr(fsys | dosym, sym_type);
    if (BOOLEAN != expr_type) {
      error(45);
    }
    cx2 = cx;
    gen(jpc, 0, 0); // cx2 ��¼�˲�������
    if (sym == dosym) {
      getsym();
    } else {
      error(18);
    }
    ++g_loop_depth;
    pbn = statement(fsys);
    --g_loop_depth;
    gen(jmp, 0, cx1); // ������������ cx1
    code[cx2].a = cx; // ���������������������� cx(cx ��ѭ����ĵ�һ��ָ��)
    for (; pbn != NULL; pbn = pbn->next) {
      code[pbn->code_loc].a = cx; // ���� break ���ĳ��ڵ�ַ
    }
    free_break_link(pbn);
    pbn = NULL;
  } else if (sym == breaksym) {
    if (g_loop_depth <= 0) {
      error(63);
    } else {
      pbn = (BreakNode *)malloc(sizeof(BreakNode));
      if (NULL == pbn) {
        printf("stack overflow");
        // system("pause");
        exit(1);
      }
      pbn->code_loc = cx; // ��ס�Լ���ָ���ַ������ѭ��������Ļ���
      pbn->next = NULL;
      gen(jmp, 0, 0);
    }
    getsym();
  } else if (sym == readsym) {
    getsym();
    if (sym != lparen) {
      error(34);
    } else {
      do {
        getsym();
        ident_ref_type = ident_ref(fsys | comma, &prcd);
        // e.g. arr[3]: ident_ref_type(of arr[3]) is real, prcd->type (of arr)
        // is Vector
        // r: ident_ref_type is real, prcd->type is also real
        if (ident_ref_type != NULL && ident_ref_type != type_table &&
            prcd != NULL && prcd != table) {
          if (prcd->kind == VARIABLE) {
            if (INTEGER == ident_ref_type) {
              gen(opr, 0, 16); // ����
            } else if (REAL == ident_ref_type) {
              gen(opr, 0, 23);
            } else {
              error(46);
            }
            if (is_arr(prcd->type)) {
              gen(sta, lev - prcd->level,
                  (prcd->addr - prcd->type->dope_vec.const_offset));
            } else {
              gen(sto, lev - prcd->level, prcd->addr); // ��������
            }
          } else {
            error(58);
          }
        } else {
          error(57);
        }
      } while (sym == comma);
    }
    if (sym != rparen) {
      error(33); // if �Ƿ�Ӧ��д������� else �У�TODO:
      while (!(sym & fsys)) {
        getsym(); // �����ﲹ���Ƿ���ʣ�����ͬ�� TODO:
      }
    } else {
      getsym();
    }
    gen(opr, 0, 24);
  } else if (sym == writesym) {
    getsym();
    if (sym != lparen) {
      error(34);
    } else {
      do {
        sym_type = getsym();
        expr_type = expr(fsys | comma | rparen, sym_type);
        if (INTEGER == expr_type) {
          gen(opr, 0, 14);
        } else if (REAL == expr_type) {
          gen(opr, 0, 17);
        } else {
          error(46);
        }
      } while (sym == comma);
    }
    if (rparen != sym) {
      error(33);
      while (!(sym & fsys)) {
        getsym();
      }
    } else {
      getsym();
    }
    gen(opr, 0, 15);
  }
  test(fsys, 0, 19);

  return pbn;
}

/*
 ** ������ű�
 ** tx0:	��ʼ���ű��±�
 */
void list_table(long tx0) {
  long i = 0;
  Record rcd;
  printf("TABLE:------------------------------------------------\n");
  if (tx0 + 1 > tx) {
    printf("NULL\n");
  }
  for (i = tx0 + 1; i <= tx; ++i) {
    rcd = table[i];
    switch (rcd.kind) {
    case CONSTANT:
      printf("%-2ld CONSTANT %s val = %ld type = %s\n", i, rcd.name, rcd.val,
             rcd.type->name);
      break;

    case VARIABLE:
      printf("%-2ld VARIABLE %s lev = %ld addr = %ld type = %s\n", i, rcd.name,
             rcd.level, rcd.addr, rcd.type->name);
      break;

    case PROC:
      printf("%-2ld PROC     %s lev = %ld addr = %ld para_num = %ld\n", i,
             rcd.name, rcd.level, rcd.addr, rcd.para_num);
      break;

    case FUNC:
      printf("%-2ld FUNC     %s lev = %ld addr = %ld ret_type = %s para_num = "
             "%ld\n",
             i, rcd.name, rcd.level, rcd.addr, rcd.type->name, rcd.para_num);
      break;

    default:
      assert(!"δ���������");
      break;
    }
  }
  printf("------------------------------------------------------\n");
}

/*
 ** ���ݷ��ŵõ�����
 */
DataType sym_to_type(SymbolType sym_type) {
  long i = 0;

  switch (sym_type) {
  case integersym:
    return type_table + 1;
    break;

  case realsym:
    return type_table + 2;
    break;

  case booleansym:
    return type_table + 3;
    break;

  case ident:
    for (i = 4; i <= type_tx; ++i) {
      if (0 == strcmp(type_table[i].name, g_id)) {
        return type_table + i;
      }
    }
    assert(!"δ���������");
    return NULL;
    break;

  default:
    assert(!"δ���������");
    return NULL;
    break;
  }
}

/*
 ** ����������Ͷ������飬�ı� g_dope_vec����Ҫ��չ��Ӧ�ع���
 ** ֻ�ܸ�����������������
 ** TypeExp -> array'['number..number']' of (integer | real | Boolean | ident)
 */
void type_expr() {
  long i = 0;
  DataType sym_type = NULL;
  // �����Ƴ����� TODO:

  if (sym != arraysym) {
    error(47);
    return;
  }
  getsym();
  if (sym != lbracket) {
    error(48);
    return;
  }

  sym_type = getsym();
  if (sym != number) {
    error(49);
    return;
  }
  if (sym_type == INTEGER) {
    g_dope_vec.lower_bound = g_num;
  } else {
    error(53);
  }

  getsym();
  if (sym != rangesym) {
    error(50);
    return;
  }

  sym_type = getsym();
  if (sym != number) {
    error(49);
    return;
  }
  if (sym_type == INTEGER) {
    g_dope_vec.upper_bound = g_num + 1;
  } else {
    error(53);
  }

  getsym();
  if (sym != rbracket) {
    error(51);
    return;
  }
  getsym();
  if (sym != ofsym) {
    error(52);
    return;
  }
  getsym();
  i = position(type_table, g_id);
  getsym();
  if (i == 0) {
    error(11);
  } else {
    g_dope_vec.elem_type = type_table + i;
  }
}

/*
 ** TypeDef -> ident = TypeExp;
 */
void type_define() {
  if (sym == ident) {
    char temp_tag[al + 1];
    strcpy(temp_tag, g_id); // �ȱ����û������������
    getsym();
    if (sym == eql || sym == becomes) {
      if (sym == becomes) {
        error(1);
      }
      getsym();
      type_expr(); // ����������飬���޸� g_dope_vec����Ҫ��չ��Ӧ�ع�
      enter(temp_tag, TAG, NULL);
      if (sym == semicolon) {
        getsym();
      } else {
        error(5);
      }
    } else {
      error(3);
    }
  } else {
    error(4);
  }
}

/*
 ** ForParal -> ident: Type{; ident: Type}
 */
void formal_para_l() {
  Record *prcd = &table[tx]; // prcd ��ʼָ��������¼�����ڲ����β���
  Record *const pfunc = prcd;
  pfunc->para_num = 0;
  assert(NULL == prcd->para_link);

  for (;;) {
    vardeclaration(); // ʹ ++tx
    if (sym == colon) {
      getsym();
    } else {
      error(37); // �Ƿ�Ӧ�ò��ȣ�
    }
    if (sym == integersym || sym == realsym || sym == booleansym) {
      table[tx].type = sym_to_type(sym);
      ++(pfunc->para_num);
      table[tx].addr = -(pfunc->para_num); // ʵ������ѹջ
      // 			++dx;	// ����ѹջ�� caller �����Σ���ռ
      // callee �Ŀռ�
      getsym();

      // ��ʱ�γ�һ��������㣬��������β�����β����
      prcd->para_link = (Record *)malloc(sizeof(Record));
      if (NULL == prcd->para_link) {
        printf("stack overflow");
        // system("pause");
        exit(1);
      }
      *(prcd->para_link) = table[tx]; // ǳ����
      prcd = prcd->para_link;
      assert(prcd->para_link == NULL);
      prcd->para_link = NULL;
    } else {
      error(64);
    }
    if (sym == semicolon) {
      getsym();
    } else {
      // printf("%s��%ld������\n", pfunc->name, pfunc->para_num);
      break;
    }
  }
}

void block(SymbolType fsys, long tx0);

/*
 ** FuncDec -> procedure ident [(ForParal)]      ; Block; |
 **            function  ident [(ForParal)]: Type; Block;
 **
 ** pdx:		pointer to data relative index
 */
void func_dec(SymbolType fsys) {
  long tmp_tx =
      0; // save current table index before processing nested procedures
  long is_func = 0;
  assert(sym == procsym || sym == funcsym);
  is_func = (sym == funcsym);
  getsym();
  if (sym == ident) {
    enter(g_id, (is_func ? FUNC : PROC), NULL);
    getsym();
  } else {
    error(4);
  }
  ++lev; // ���ù��̵��βε�����ֲ�����
  tmp_tx = tx;
  table[tx].addr = cx; // ���� jmp �� cx
  if (sym == lparen) {
    getsym();
    formal_para_l(); // ���ı� tx
    if (sym == rparen) {
      getsym();
    } else {
      error(33);
    }
  }
  // function ����
  if (is_func) {
    if (sym == colon) {
      getsym();
    } else {
      error(37);
    }
    if (sym == integersym || sym == realsym || sym == booleansym) {
      assert(table[tmp_tx].kind == FUNC);
      table[tmp_tx].type = sym_to_type(sym);
      getsym();
    } else {
      error(65);
    }
  }
  if (sym == semicolon) {
    getsym();
  } else {
    error(5);
  }
  block(fsys | semicolon, tmp_tx);
  free_para_link(table + tmp_tx + 1, table + tx + 1);
  tx = tmp_tx; // �β��ڷ��ű���Ҳ�����٣����������β�����
  --lev;
  if (sym == semicolon) {
    getsym();
    test(statbegsys | ident | procsym | funcsym, fsys, 6);
  } else {
    error(5);
  }
}

void block(SymbolType fsys, long tx0) {
  BreakNode *pbn = NULL;
  long cx0; // initial code index

  long dx = 3;
  gen(jmp, 0, 0);
  if (lev > levmax) {
    error(32);
  }
  do {
    if (sym == constsym) {
      getsym();
      do {
        constdeclaration();
        while (sym == comma) {
          getsym();
          constdeclaration();
        }
        if (sym == semicolon) {
          getsym();
        } else {
          error(5);
        }
      } while (sym == ident); // �ɶ���Ϊ const a = 1, b = 2; c = 3;
    }
    if (sym == typesym) {
      getsym();
      do {
        type_define();
      } while (sym == ident);
    }
    if (sym == varsym) {
      getsym(); // sym Ӧ�� ident
      do {
        long tx1 =
            tx + 1; // tx1 ָ��Ҫ����ı����ڷ��ű��е�λ�ã����ڷ���������Ϣ
        vardeclaration();
        while (sym == comma) {
          getsym();
          vardeclaration();
        }
        if (sym == colon) {
          getsym();
        } else {
          error(37); // �Ƿ�Ӧ�ò��ȣ�
        }
        if (sym & (integersym | realsym | booleansym |
                   ident)) { // Ӧ��� ident �Ƿ�Ϊ�������� TODO:
          long i;
          for (i = tx1; i <= tx; ++i) {
            table[i].type =
                sym_to_type(sym); // ������ü��� vardeclaration() �У����ع�
            table[i].addr = dx;
            dx += table[i].type->size;
            if (NULL == table[i].type) {
              assert(!"δ���������");
            }
          }
          getsym();
        } else {
          error(38);
        }
        if (sym == semicolon) {
          getsym();
        } else {
          error(5);
        }
      } while (sym == ident);
    }
    while (sym == procsym || sym == funcsym) {
      func_dec(fsys);
    }
    test(statbegsys | ident, declbegsys, 7);
  } while (sym & declbegsys);

  assert(code[table[tx0].addr].f == jmp);
  code[table[tx0].addr].a = cx; // ��ʼ���ɵ�ǰ���̴���
  table[tx0].addr = cx;         // ������������ڵ�ַ
  cx0 = cx;
  gen(Int, 0, dx); // �����ڴ����ݶΣ�3����Ԫ�Ӿֲ���������
  //	list_table(tx0);	// ������ű� (tx0, tx]
  pbn = statement(fsys | semicolon |
                  endsym); // ÿ��������������ϲ��������Ա㲹��
  if (NULL != pbn) {
    error(63); // break ���û�д����κ� while �����
  }
  gen(opr, 0, 0); // �˳�����ʱ�ͷ��ڴ����ݶ�
  test(fsys, 0, 8);
  //	list_code(cx0);		// [cx0, cx)
}

long base(long b, long l) {
  long b1 = b;
  while (l > 0) { // find base l levels down
    b1 = s[b1];
    --l;
  }
  return b1;
}

void interpret() {
  long pre_p = 0;   // ��¼ pc
  long p = 0;       // program count
  long b = 1;       // base register
  long t = 0;       // topstack register(ָ��ջ��������һλ�ã�s[0]����)
  instruction i;    // instruction register	float fa = 0;	// Ϊ����������
  long registr = 0; // �ź�������ֵ

  long first = 0; // for reverse arguments
  long last = 0;

  float fa = .0f;
  float fb = .0f;
  size_t fsize = sizeof(float);
  assert(sizeof(long) == sizeof(float));

  // printf("start PL/0\n");
  s[1] = 0;
  s[2] = 0;
  s[3] = 0;
  do {
    i = code[p];
    pre_p = p;
    ++p;
    switch (i.f) {
    case lit:
      ++t;
      s[t] = i.a; // a ��ʾ����ֵ����
      break;
    case opr:
      switch (i.a) { // operator
      case 0:        // return
        t = b - 1;
        p = s[t + 3]; // RA
        b = s[t + 2]; // DL
        break;
      case 1:
        s[t] = -s[t];
        break;
      case 2:
        --t;
        s[t] += s[t + 1];
        break;
      case 3:
        --t;
        s[t] -= s[t + 1];
        break;
      case 4:
        --t;
        s[t] *= s[t + 1];
        break;
      case 5:
        --t;
        s[t] /= s[t + 1];
        break;
      case 6:
        s[t] %= 2;
        break;
      case 8:
        --t;
        s[t] = (s[t] == s[t + 1]);
        assert(s[t] == 1 || s[t] == 0);
        break;
      case 9:
        --t;
        s[t] = (s[t] != s[t + 1]);
        assert(s[t] == 1 || s[t] == 0);
        break;
      case 10:
        --t;
        s[t] = (s[t] < s[t + 1]);
        break;
      case 11:
        --t;
        s[t] = (s[t] >= s[t + 1]);
        break;
      case 12:
        --t;
        s[t] = (s[t] > s[t + 1]);
        break;
      case 13:
        --t;
        s[t] = (s[t] <= s[t + 1]);
        break;

      case 14:
        printf("%ld ", s[t]); // ��ʱ�򵥴���ֱ�����һ���ո� TODO:
        --t;
        break;

      case 15:
        printf("\n");
        break;

      case 16:
        // printf("���������ݣ�");
        ++t;
        scanf("%ld", &s[t]); // �������ͺ�Ӧ���������ʽ�ַ���
        break;

      case 17:
        memcpy(&fa, &s[t], fsize);
        printf("%.2f ", fa); // Ĭ�ϱ���2λС��
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
        scanf("%*[^\n]%*c"); // or scanf("%*[^\n]");	// �´ζ�������ʱ
                             // '\n' �����
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
        fa = (float)(s[t]); // INTEGER �� REAL ����32λ���� INTEGER ��ʽת����
                            // REAL�����ܶ�ʧ��Ϣ������ͬ��
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
        assert(!"δ����Ĳ���ָ��");
        break;
      }
      break;

    case lod:
      ++t;
      s[t] = s[base(b, i.l) + i.a];
      break;

    case sto:
      s[base(b, i.l) + i.a] = s[t];
      //	printf("%10d\n", s[t]);
      --t;
      break;

    case lda:
      s[t] = s[base(b, i.l) + i.a + s[t]];
      break;

    case sta: // store array element
      s[base(b, i.l) + i.a + s[t - 1]] = s[t];
      t -= 2;
      break;

    case cal:                  // generate new block mark
      s[t + 1] = base(b, i.l); // SL
      s[t + 2] = b;            // DL
      s[t + 3] = p;            // RA
      b = t + 1;               // base �� SL ��λ�ã����� t+1
      p = i.a;                 // callee ����ڵ�ַ
      break;

    case Int:
      t += i.a;
      break;

    case jmp:
      p = i.a;
      break;
    case jpc:
      if (s[t] == 0) {
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
      if (!(s[t - 1] <= s[t - 2] && s[t - 2] < s[t])) {
        printf("ArrayIndexOutOfBoundsException at instructon %ld\n", pre_p);
        // system("pause");
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
      assert(!"δ����Ĳ���ָ��");
      break;
    }
  } while (p != 0);
  // printf("end PL/0\n");
}

/*
 ** ��ʼ��ȫ�ֱ���
 */
void init() {
  long i;
  for (i = 0; i < 256; i++) {
    ssym[i] = nul;
  }

  // reserved words name(���������ѱ���ֲ���)
  strcpy(word[0], "Boolean   "); // ����ĸ��д
  strcpy(word[1], "and       ");
  strcpy(word[2], "array     ");
  strcpy(word[3], "begin     ");
  strcpy(word[4], "break     ");
  strcpy(word[5], "call      ");
  strcpy(word[6], "const     ");
  strcpy(word[7], "div       ");
  strcpy(word[8], "do        ");
  strcpy(word[9], "else      ");
  strcpy(word[10], "end       ");
  strcpy(word[11], "false     ");
  strcpy(word[12], "function  ");
  strcpy(word[13], "if        ");
  strcpy(word[14], "integer   ");
  strcpy(word[15], "mod       ");
  strcpy(word[16], "not       ");
  strcpy(word[17], "odd       ");
  strcpy(word[18], "of        ");
  strcpy(word[19], "or        ");
  strcpy(word[20], "procedure ");
  strcpy(word[21], "read      ");
  strcpy(word[22], "real      ");
  strcpy(word[23], "then      ");
  strcpy(word[24], "true      ");
  strcpy(word[25], "type      ");
  strcpy(word[26], "var       ");
  strcpy(word[27], "while     ");
  strcpy(word[28], "write     ");

  // reserved words symbol
  wsym[0] = booleansym;
  wsym[1] = andsym;
  wsym[2] = arraysym;
  wsym[3] = beginsym;
  wsym[4] = breaksym;
  wsym[5] = callsym;
  wsym[6] = constsym;
  wsym[7] = divsym;
  wsym[8] = dosym;
  wsym[9] = elsesym;
  wsym[10] = endsym;
  wsym[11] = falsesym;
  wsym[12] = funcsym;
  wsym[13] = ifsym;
  wsym[14] = integersym;
  wsym[15] = modsym;
  wsym[16] = notsym;
  wsym[17] = oddsym;
  wsym[18] = ofsym;
  wsym[19] = orsym;
  wsym[20] = procsym;
  wsym[21] = readsym;
  wsym[22] = realsym;
  wsym[23] = thensym;
  wsym[24] = truesym;
  wsym[25] = typesym;
  wsym[26] = varsym;
  wsym[27] = whilesym;
  wsym[28] = writesym;

  // special symbol(single character)
  ssym[(unsigned)'+'] = plus;
  ssym[(unsigned)'-'] = minus;
  ssym[(unsigned)'*'] = times;
  ssym[(unsigned)'/'] = slash;
  ssym[(unsigned)'('] = lparen;
  ssym[(unsigned)')'] = rparen;
  ssym[(unsigned)'='] = eql;
  ssym[(unsigned)','] = comma;
  ssym[(unsigned)'.'] = period;
  ssym[(unsigned)';'] = semicolon;
  ssym[(unsigned)':'] = colon;
  ssym[(unsigned)'['] = lbracket;
  ssym[(unsigned)']'] = rbracket;

  // instruction name
  strcpy(mnemonic[lit], "lit");
  strcpy(mnemonic[opr], "opr");
  strcpy(mnemonic[lod], "lod");
  strcpy(mnemonic[sto], "sto");
  strcpy(mnemonic[cal], "cal");
  strcpy(mnemonic[Int], "int");
  strcpy(mnemonic[jmp], "jmp");
  strcpy(mnemonic[jpc], "jpc");
  strcpy(mnemonic[jp0], "jp0");
  strcpy(mnemonic[jp1], "jp1");
  strcpy(mnemonic[sta], "sta");
  strcpy(mnemonic[lda], "lda");
  strcpy(mnemonic[ckb], "ckb");
  strcpy(mnemonic[ppa], "ppa");
  strcpy(mnemonic[rva], "rva");

  // ������������䡢���ӵĿ�ʼ���ż�
  declbegsys = constsym | varsym | procsym | funcsym;
  statbegsys = beginsym | callsym | ifsym | whilesym | readsym | writesym;
  facbegsys = ident | number | lparen | truesym | falsesym | notsym | oddsym;

  g_num = 0;
  g_loop_depth = 0;
  err = 0;
  cc = 0;
  cx = 0;
  ll = 0;
  ch = ' ';
  kk = al;
  lev = 0;
  tx = 0;

  type_tx = 3;
  // ����������ȫ������ʼ��Ϊ0
  strcpy(type_table[1].name, "integer   ");
  type_table[1].size = 1;

  strcpy(type_table[2].name, "real      ");
  type_table[2].size = 1;

  strcpy(type_table[3].name, "Boolean   ");
  type_table[3].size = 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("pl0: fatal error: no input files\ncompilation terminated.");
    return 1;
  }
  init();
  //	printf("please input source program file name: ");
  //	sprintf(infilename, "C:\\Users\\Administrator\\Desktop\\pl0ext_doc\\test
  //programs\\PreTest%ld.pl0", i);
  //	scanf("%s", infilename);
  //	printf("\n");

  strcpy(infilename, argv[1]);
  if ((infile = fopen(infilename, "r")) == NULL) {
    printf("File %s can't be opened.\n", infilename);
    // system("pause");
    exit(1);
  }

  getsym();
  table[tx].addr = cx;
  block(declbegsys | statbegsys | period, tx); // Why declbegsys and statbegsys?
  free_para_link(table + 1, table + tx + 1);
  if (sym != period) {
    error(9);
  }
  if (err == 0) {
    interpret();
  } else {
    printf("%ld error%s in PL/0 program\n", err, (err == 1 ? "" : "s"));
  }
  fclose(infile);

  return 0;
}
