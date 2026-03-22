#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,      // ==
  /* TODO: Add more token types */
  TK_NEQ,     // !=
  TK_AND,     // &&
  TK_NUM,     // decimal number
  TK_HEX,     // hex number
  TK_REG,     // register like $eax
  TK_DEREF,   // pointer dereference (unary *)
  TK_NEG,     // negation (unary -)
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +",                   TK_NOTYPE},  // spaces
  {"==",                   TK_EQ},      // equal (before = )
  {"!=",                   TK_NEQ},     // not equal
  {"&&",                   TK_AND},     // logical and
  {"\\+",                  '+'},        // plus
  {"-",                    '-'},        // minus
  {"\\*",                  '*'},        // multiply or deref
  {"/",                    '/'},        // divide
  {"\\(",                  '('},        // left paren
  {"\\)",                  ')'},        // right paren
  {"0[xX][0-9a-fA-F]+",   TK_HEX},    // hex number
  {"[0-9]+",               TK_NUM},    // decimal number
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG}, // register
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          //default: TODO();
          case TK_NOTYPE:
            break; // skip spaces
          case TK_NUM:
          case TK_HEX:
          case TK_REG:
            if (nr_token >= 32) {
              printf("Expression too long\n");
              return false;
            }
            if (substr_len >= 32) {
              printf("Token string too long\n");
              return false;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          default:
            if (nr_token >= 32) {
              printf("Expression too long\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/* 检查 tokens[p..q] 是否是一个有效的括号包裹的表达式 */
static bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;
  int cnt = 0;
  int i;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') {
      cnt--;
      if (cnt == 0 && i < q) return false; // outer '(' matched before q
    }
  }
  return cnt == 0;
}

/* 给出token优先级*/
static int get_precedence(int type) {
  switch (type) {
    case TK_AND: return 1;
    case TK_EQ:
    case TK_NEQ: return 2;
    case '+': return 3;
    case '-': return 3;
    case '*': return 4;
    case '/': return 4;
    default:  return -1;
  }
}

static int dominant_operator(int p, int q) {
  int op = -1;
  int op_prec = 100;
  int paren_depth = 0;
  int i;

  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') { paren_depth++; continue; }
    if (tokens[i].type == ')') { paren_depth--; continue; }
    if (paren_depth > 0) continue;

    // unary operators are handled separately
    if (tokens[i].type == TK_NEG || tokens[i].type == TK_DEREF) continue;

    int prec = get_precedence(tokens[i].type);
    if (prec < 0) continue;

    // choose the rightmost operator among the same precedence
    // to keep left associativity in recursive split
    if (prec <= op_prec) {
      op = i;
      op_prec = prec;
    }
  }

  return op;
}

static uint32_t eval(int p, int q, bool *success);

static uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    printf("Empty expression\n");
    *success = false;
    return 0;
  }

  else if (p == q) {
    /* Single token */
    *success = true;
    switch (tokens[p].type) {
      case TK_NUM:
        return (uint32_t)strtoul(tokens[p].str, NULL, 10);
      case TK_HEX:
        return (uint32_t)strtoul(tokens[p].str, NULL, 16);
      case TK_REG: {
        const char *name = tokens[p].str + 1; /* skip '$' */
        int i;
        if (strcmp(name, "eip") == 0) return cpu.eip;
        for (i = 0; i < 8; i++) {
          if (strcmp(name, regsl[i]) == 0) return reg_l(i);
        }
        printf("Unknown register: %s\n", tokens[p].str);
        *success = false;
        return 0;
      }
      default:
        printf("Unexpected token\n");
        *success = false;
        return 0;
    }
  }

  else if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  }

  else {
    // Unary operators at position p
    if (tokens[p].type == TK_NEG) {
      uint32_t val = eval(p + 1, q, success);
      if (!*success) return 0;
      return (uint32_t)(-(int32_t)val);
    }
    if (tokens[p].type == TK_DEREF) {
      uint32_t addr = eval(p + 1, q, success);
      if (!*success) return 0;
      return vaddr_read(addr, 4);
    }

    int op = dominant_operator(p, q);
    if (op == -1) {
      printf("No valid operator found\n");
      *success = false;
      return 0;
    }

    uint32_t val1 = eval(p, op - 1, success);
    if (!*success) return 0;
    uint32_t val2 = eval(op + 1, q, success);
    if (!*success) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/':
        if (val2 == 0) {
          printf("Division by zero\n");
          *success = false;
          return 0;
        }
        return val1 / val2;
      case TK_EQ:  return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default:
        *success = false;
        return 0;
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  if (nr_token == 0) {
    *success = false;
    return 0;
  }
  
  int i;
  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-' || tokens[i].type == '*') {
      bool is_unary = (i == 0) ||
        (tokens[i-1].type != TK_NUM  &&
         tokens[i-1].type != TK_HEX  &&
         tokens[i-1].type != TK_REG  &&
         tokens[i-1].type != ')');
      if (is_unary) {
        tokens[i].type = (tokens[i].type == '*') ? TK_DEREF : TK_NEG;
      }
    }
  }

  *success = true;
  return eval(0, nr_token - 1, success);
}
