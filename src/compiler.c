// pl/0 compiler with code generation
#define _CRT_SECURE_NO_WARNINGS
#define NDEBUG
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "compiler.h"

void error(long n) {
  long i;

  printf(" ****");
  for (i = 1; i <= cc - 1; i++) {
    printf(" ");
  }
  printf("^%2d\n", n);
  err++;
}

void getch() {
  if (cc == ll) {
    if (feof(infile)) {
      printf("************************************\n");
      printf("      program incomplete\n");
      printf("************************************\n");
      system("pause");
      exit(1);
    }
    ll = 0;
    cc = 0;
    printf("%5d ", cx);
    while ((!feof(infile)) && ((ch = (char)(getc(infile))) != '\n')) {
      printf("%c", ch);
      ++ll;
      line[ll] = ch;
    }
    printf("\n");
    ++ll;
    line[ll] =
        ' '; // 将每一行最后的'\n'换为' ', line[0] 不放字符，line[ll] 放空格
  }
  assert(line[0] == '\0' && line[ll] == ' ');
  ++cc;
  ch = line[cc];
}

/*
** 为区别实数 0.5 和数组下标界 0..5，必须向前查看两个字符
** 返回前面第二个字符是不是'.'(不影响 cc)
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
** 返回 type 是否为数组类型
*/
long is_arr(TypeRecord const *const type) {
  assert(type != type_table);
  if (type != NULL) {
    assert(type->dope_vec.elem_type != type_table);
  }
  assert(type_table < type &&
         type <= type_table + type_tx); // 类型名未定义，外部应检测过
  return type != NULL && type->dope_vec.elem_type != NULL;
}

/*
** 层次管理导致另外维护形参链
** 本函数释放动态生成的形参链
*/
void free_para_link(Record *begin, Record *end) {
  for (; begin != end; ++begin) {
    if ((begin->kind == PROC || begin->kind == FUNC) &&
        begin->para_link != NULL) {
      Record *p = begin->para_link; // p 指向第一个参数，begin
                                    // 指向链表的头结点（头结点非动态生成）
      Record *pre = NULL;           // 记住前一个结点，用于释放
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
** 词法分析
** 调用本函数之前已 getch()
*/
DataType getsym() {
  long i, j, k;
  float real_num = 0; // 32位，须与 num 类型、运行栈类型大小相等
  float weight = 0;   // 用于计算实数的小数部分
  DataType sym_type =
      NULL; // 当 sym 是 number 时区别整数与实数，其他符号则返回为 NULL

LABEL_INIT:
  while (ch == ' ' || ch == '\t') {
    getch();
  }
  if (isalpha(ch)) { // identified or reserved
    assert(a[al] == '\0');
    k = 0; // k 记录标识符的长度
    do {
      if (k < al) {
        a[k] = ch; // 若标识符长度大于al，后面的将丢失
        ++k;
      } else {
        error(54); // 标识符太长
      }
      getch();
    } while (isalpha(ch) || isdigit(ch));
    if (k >= kk) {
      kk =
          k; // kk
             // 是全局变量，初始是al（标识符最大长度），若执行到这里，说明此次标识符长度>=上次标识符长度
    } else {
      do {
        --kk;
        a[kk] = ' ';
      } while (k < kk);
    }
    strcpy(g_id, a);
    i = 0;
    j = norw - 1;
    do { // 二分查找 id 是否为保留字，可用 bsearch()
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
        getch(); // 注意在跳转之前 getch()
        goto LABEL_INIT;
      } else if (ch == '*') {
        goto LABEL_COMMENT2;
      } else {
        goto LABEL_COMMENT1;
      }
    } else {
      sym = slash; // 出现斜杠后，已经 getch()，这里不能再 getch()
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
** 指令格式
** x:	操作码
** y:	嵌套层次
** z:	地址
*/
void gen(enum fct x, long y, long z) {
  if (cx > cxmax) {
    printf("program too long\n");
    system("pause");
    exit(1);
  }
  code[cx].f = x;
  code[cx].l = y;
  code[cx].a = z;
  ++cx;
}

/*
** 检测 sym(global) 是否在 s1 中，若不在则用 s2 补救
** s1:	需要的符号集
** s2:	补救用的符号集
** n:	错误号
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
** 返回相应表中的下标位置，若找不到 id 则返回0
*/
long position(void *which_table, char *id) { // find identifier id in table
  long i = 0;

  if (which_table == (void *)table) {
    strcpy(table[0].name, id); // 哨兵
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
    assert(!"错误的符号表");
  }

  return i;
}

void enter(char const *const sym_id, enum object k,
           DataType down_type) { // enter object into table
  long i;
  TypeRecord *ptrcd = NULL;
  // 同一层中不允许重定义，从1到 tx 的各符号的 level 是递增的
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
    // var a, b: real; 这种声明只有当 real 出现时才知道 a, b
    // 的类型，所以类型、地址和 dx 在外面填写 TODO:
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
    // 类型等扫描到函数头的结尾才知道，在外面的 func_dec() 中回填
    break;

  case TAG:
    strcpy(type_table[++type_tx].name, sym_id);
    type_table[type_tx].dope_vec =
        g_dope_vec; // g_dope_vec 的上下界在 type_expr() 中获得
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
    assert(!"未定义的类型");
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
    enter(g_id, VARIABLE, NULL); // 类型在 block() 中加入符号表
    getsym();
  } else {
    error(4);
  }
}

// void list_code(long cx0) {	// list code generated for this block
// 	long i;
//
// 	for(i = cx0; i < cx; i++) {
// 		printf("%10d%5s%3d%15d\n", i, mnemonic[code[i].f], code[i].l,
// code[i].a);
// 	}
// }

/*
** 将 IR 输出到 fout
*/
void list_code(FILE *fout, instruction const *const first,
               instruction const *const last) {
  instruction const *ite = first;
  for (; ite != last; ++ite) {
    fprintf(fout, "%5s%3d%15d\n", mnemonic[ite->f], ite->l, ite->a);
  }
}

DataType expr(SymbolType fsys, DataType down_type);

/*
** IdentRef -> ident['['Exp']'{'['Exp']'}]
** 返回 IdentRef 的类型
** pprcd:	指向关键字为 ident 的记录的指针的指针，用于回调位置
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
    return ident_ref_type; // 非数组则直接返回
  }
  do {
    ++dim_num;
    if (!is_arr(ident_ref_type)) {
      error(56); // 有左括号但不是数组
      return NULL;
    }

    sym_type = getsym();
    if (dim_num >= 2) {
      gen(lit, 0, (is_arr(ident_ref_type) ? ident_ref_type->dope_vec.n : 1));
      gen(opr, 0, 4);
    }
    subscript_type = expr(fsys | rbracket, sym_type); // 将下标压入栈
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
  while (sym & facbegsys) { // 为什么要循环？
    if (sym == ident) {
      prcd = &table[position(table, g_id)];
      if (table == prcd) {
        error(11);
      } else if (prcd->kind == FUNC) {
        // 函数名不能调用 ident_ref，以此区分执行
        // 避免向前查看两个符号（前提是函数不能返回数组）
        // Factor -> ident[(ActParal)]
        factor_type = prcd->type;
        getsym();
        if (lparen == sym) {
          sym_type = getsym();
          actual_para_l(fsys | rparen, sym_type, prcd);
          gen(rva, 0, prcd->para_num); // 逆转实参，模拟实现参数逆序压栈
          gen(cal, lev - prcd->level, prcd->addr);
          gen(ppa, 0,
              prcd->para_num); // 栈顶指针恢复到参数压栈前的位置（对应实参压栈）
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
            assert(!"未定义的类型");
            break;
          }
        } else {
          error(57);
        }
      }
    } else if (sym == number) {
      factor_type = down_type; // 在 factor() 外面 getsym() 得到的 sym(number)
                               // 的类型是 down_type
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
        factor_type = BOOLEAN; // 这样补救是否合适？
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
      assert(!"非法的 factor 开始符");
    }
    test(fsys, lparen, 23); // 补救集不是 facbegsys?
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
      assert(!"非法类型");
    }
  }

  //	assert(NULL != factor_type1);
  return factor_type1;
}

/*
** down_type:	调用 simp_exp() 前 getsym() 的返回值，用于字面值
*/
DataType simp_exp(SymbolType fsys, DataType down_type) {
  SymbolType addop = 0;
  DataType sym_type = NULL; // 记录局部的 getsym() 的返回值，调用子过程时传下去
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
      gen(jp1, 0, 0); // 短路
    }
    addop = sym;
    sym_type = getsym();
    term_type2 = term(fsys | plus | minus | orsym, sym_type);
    // 应对 3 or 2.0 和 true + true 这样的表达式做类型检查
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
      gen(opr, 0, 31); // 此时次栈顶是INTEGER,栈顶是REAL
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
        code[cx_temp].a = cx; // 回填短路
      } else {
        error(41);
      }
    } else {
      error(41);
    }
  }

  return term_type1; // 表示式的类型就是最后的项的类型
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
        error(41); // 不允许 false < true 这样的表达式
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
      assert(!"未定义的逻辑操作");
      break;
    }
    expr_type1 = BOOLEAN;
  }

  return expr_type1;
}

/*
** ActParal -> Exp{, Exp}
** prcd 指向符号表中的函数名记录
*/
void actual_para_l(SymbolType fsys, DataType down_type, Record const *prcd) {
  assert(NULL != prcd);
  for (;;) {
    DataType arg_type = expr(fsys | comma, down_type);
    if (prcd->para_link == NULL) {
      error(60); // 实参数目太多
    } else {
      prcd = prcd->para_link;
      if (prcd->type == REAL && arg_type == INTEGER) {
        gen(opr, 0, 30);
      } else if (prcd->type != arg_type) {
        error(62);
      }
      // gen(sto, lev - prcd->level, prcd->addr);	// callee
      // 知道实参位置，用时去取即可
    }
    if (comma == sym) {
      getsym();
    } else {
      if (NULL != prcd->para_link) {
        error(61); // 实参数目太少
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
      gen(opr, 0, 36); // 将返回值存到寄存器中
      gen(opr, 0, 0);
    } else if (is_arr(prcd->type)) {
      // 数组常量部分编译时算好
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
      prcd = &table[position(table, g_id)]; // 指向函数名记录
      assert(NULL != prcd);
      getsym();
      if (prcd == table) {
        error(11);
      } else if (prcd->kind == PROC) {
        if (lparen == sym) {
          sym_type = getsym();
          actual_para_l(fsys | rparen, sym_type, prcd);
          gen(rva, 0, prcd->para_num); // 逆转实参，模拟实现参数逆序压栈
          gen(cal, lev - prcd->level, prcd->addr);
          gen(ppa, 0,
              prcd->para_num); // 栈顶指针恢复到参数压栈前的位置（对应实参压栈）
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
      code[cx1].a = cx + 1; // cx 处放 jmp
      cx1 = cx;
      gen(jmp, 0, 0);
      pbn2 = statement(fsys);
      pbn = list_cat(pbn, pbn2);
      code[cx1].a = cx; // 前面修改了 cx1
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
      pbn = list_cat(pbn, pbn2); // 将 pbn2 接到 pbn 的尾部，pbn 原来可能是 NULL
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
    gen(jpc, 0, 0); // cx2 记录了测试条件
    if (sym == dosym) {
      getsym();
    } else {
      error(18);
    }
    ++g_loop_depth;
    pbn = statement(fsys);
    --g_loop_depth;
    gen(jmp, 0, cx1); // 跳到测试条件 cx1
    code[cx2].a = cx; // 若测试条件不满足则跳到 cx(cx 是循环外的第一条指令)
    for (; pbn != NULL; pbn = pbn->next) {
      code[pbn->code_loc].a = cx; // 回填 break 语句的出口地址
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
        system("pause");
        exit(1);
      }
      pbn->code_loc = cx; // 记住自己的指令地址，用于循环结束后的回填
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
              gen(opr, 0, 16); // 输入
            } else if (REAL == ident_ref_type) {
              gen(opr, 0, 23);
            } else {
              error(46);
            }
            if (is_arr(prcd->type)) {
              gen(sta, lev - prcd->level,
                  (prcd->addr - prcd->type->dope_vec.const_offset));
            } else {
              gen(sto, lev - prcd->level, prcd->addr); // 基本类型
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
      error(33); // if 是否应该写在上面的 else 中？TODO:
      while (!(sym & fsys)) {
        getsym(); // 在这里补救是否合适？下面同理 TODO:
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
** 输出符号表
** tx0:	初始符号表下标
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
      printf("%-2d CONSTANT %s val = %ld type = %s\n", i, rcd.name, rcd.val,
             rcd.type->name);
      break;

    case VARIABLE:
      printf("%-2d VARIABLE %s lev = %ld addr = %ld type = %s\n", i, rcd.name,
             rcd.level, rcd.addr, rcd.type->name);
      break;

    case PROC:
      printf("%-2d PROC     %s lev = %ld addr = %ld para_num = %ld\n", i,
             rcd.name, rcd.level, rcd.addr, rcd.para_num);
      break;

    case FUNC:
      printf("%-2d FUNC     %s lev = %ld addr = %ld ret_type = %s para_num = "
             "%ld\n",
             i, rcd.name, rcd.level, rcd.addr, rcd.type->name, rcd.para_num);
      break;

    default:
      assert(!"未定义的类型");
      break;
    }
  }
  printf("------------------------------------------------------\n");
}

/*
** 根据符号得到类型
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
    assert(!"未定义的类型");
    return NULL;
    break;

  default:
    assert(!"未定义的类型");
    return NULL;
    break;
  }
}

/*
** 这里仅限类型定义数组，改变 g_dope_vec。若要扩展，应重构。
** 只能给数组类型命名，故
** TypeExp -> array'['number..number']' of (integer | real | Boolean | ident)
*/
void type_expr() {
  long i = 0;
  DataType sym_type = NULL;
  // 待完善出错处理 TODO:

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
    strcpy(temp_tag, g_id); // 先保存用户定义的类型名
    getsym();
    if (sym == eql || sym == becomes) {
      if (sym == becomes) {
        error(1);
      }
      getsym();
      type_expr(); // 这里仅限数组，将修改 g_dope_vec。若要扩展，应重构
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
  Record *prcd = &table[tx]; // prcd 初始指向函数名记录，用于操纵形参链
  Record *const pfunc = prcd;
  pfunc->para_num = 0;
  assert(NULL == prcd->para_link);

  for (;;) {
    vardeclaration(); // 使 ++tx
    if (sym == colon) {
      getsym();
    } else {
      error(37); // 是否应该补救？
    }
    if (sym == integersym || sym == realsym || sym == booleansym) {
      table[tx].type = sym_to_type(sym);
      ++(pfunc->para_num);
      table[tx].addr = -(pfunc->para_num); // 实参逆序压栈
      // 			++dx;	// 参数压栈是 caller 的责任，不占
      // callee 的空间
      getsym();

      // 此时形成一个参数结点，将其加入形参链表，尾插入
      prcd->para_link = (Record *)malloc(sizeof(Record));
      if (NULL == prcd->para_link) {
        printf("stack overflow");
        system("pause");
        exit(1);
      }
      *(prcd->para_link) = table[tx]; // 浅拷贝
      prcd = prcd->para_link;
      assert(prcd->para_link == NULL);
      prcd->para_link = NULL;
    } else {
      error(64);
    }
    if (sym == semicolon) {
      getsym();
    } else {
      // printf("%s有%ld个参数\n", pfunc->name, pfunc->para_num);
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
  ++lev; // 将该过程的形参当成其局部变量
  tmp_tx = tx;
  table[tx].addr = cx; // 保存 jmp 的 cx
  if (sym == lparen) {
    getsym();
    formal_para_l(); // 将改变 tx
    if (sym == rparen) {
      getsym();
    } else {
      error(33);
    }
  }
  // function 特有
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
  tx = tmp_tx; // 形参在符号表中也将销毁，但保存在形参链中
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
      } while (sym == ident); // 可定义为 const a = 1, b = 2; c = 3;
    }
    if (sym == typesym) {
      getsym();
      do {
        type_define();
      } while (sym == ident);
    }
    if (sym == varsym) {
      getsym(); // sym 应是 ident
      do {
        long tx1 =
            tx + 1; // tx1 指向将要定义的变量在符号表中的位置，用于反填类型信息
        vardeclaration();
        while (sym == comma) {
          getsym();
          vardeclaration();
        }
        if (sym == colon) {
          getsym();
        } else {
          error(37); // 是否应该补救？
        }
        if (sym & (integersym | realsym | booleansym |
                   ident)) { // 应检查 ident 是否为数组类型 TODO:
          long i;
          for (i = tx1; i <= tx; ++i) {
            table[i].type =
                sym_to_type(sym); // 类型最好加在 vardeclaration() 中，待重构
            table[i].addr = dx;
            dx += table[i].type->size;
            if (NULL == table[i].type) {
              assert(!"未定义的类型");
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
  code[table[tx0].addr].a = cx; // 开始生成当前过程代码
  table[tx0].addr = cx;         // 反填过程体的入口地址
  cx0 = cx;
  gen(Int, 0, dx); // 分配内存数据段，3个单元加局部变量个数
  //	list_table(tx0);	// 输出符号表 (tx0, tx]
  pbn = statement(fsys | semicolon |
                  endsym); // 每个后跟集都包含上层后跟集，以便补救
  if (NULL != pbn) {
    error(63); // break 语句没有处于任何 while 语句中
  }
  gen(opr, 0, 0); // 退出过程时释放内存数据段
  test(fsys, 0, 8);
  //	list_code(cx0);		// [cx0, cx)
}

/*
** 初始化全局变量
*/
void init() {
  long i;
  for (i = 0; i < 256; i++) {
    ssym[i] = nul;
  }

  // reserved words name(保持有序已便二分查找)
  strcpy(word[0], "Boolean   "); // 首字母大写
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
  ssym['+'] = plus;
  ssym['-'] = minus;
  ssym['*'] = times;
  ssym['/'] = slash;
  ssym['('] = lparen;
  ssym[')'] = rparen;
  ssym['='] = eql;
  ssym[','] = comma;
  ssym['.'] = period;
  ssym[';'] = semicolon;
  ssym[':'] = colon;
  ssym['['] = lbracket;
  ssym[']'] = rbracket;

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

  // 设置声明、语句、因子的开始符号集
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
  // 其他域已在全局区初始化为0
  strcpy(type_table[1].name, "integer   ");
  type_table[1].size = 1;

  strcpy(type_table[2].name, "real      ");
  type_table[2].size = 1;

  strcpy(type_table[3].name, "Boolean   ");
  type_table[3].size = 1;
}

int main() {
  init();
  printf("please input source program file name: ");
  scanf("%s", infilename);
  // sprintf(infilename, "C:\\Users\\Administrator\\Desktop\\pl0ext_doc\\test
  // programs\\PreTest%ld.pl0", i);
  // strcpy(infilename, "myTest.pl0");
  printf("\n");
  if ((infile = fopen(infilename, "r")) == NULL) {
    printf("File %s can't be opened.\n", infilename);
    system("pause");
    exit(1);
  }

  getsym();
  table[tx].addr = cx;
  block(declbegsys | statbegsys | period, tx); // Why declbegsys and statbegsys?
  free_para_link(table + 1, table + tx + 1);
  if (sym != period) {
    error(9);
  }
  if (err > 0) {
    printf("%ld error%s in PL/0 program\n", err, (err == 1 ? "" : "s"));
  } else {
    FILE *outfile = NULL;
    char outfilename[MAX_FILE_NAME_LEN];
    sprintf(outfilename, "%s%c", infilename, 'c'); // IR 扩展名为 pl0c
    outfile = fopen(outfilename, "w");
    list_code(outfile, code, code + cx);
    printf("Output IR to file %s.\n", outfilename);
    fclose(outfile);
  }
  fclose(infile);

  system("pause");
  return 0;
}