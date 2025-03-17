#include "nemu.h"
#include <stdlib.h>
#include <string.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_NUM,
  TK_IDENT,
  TK_DEREF,
  TK_HEX_NUM,
  TK_REG,
  // '+' = 260,
  // '-' = 261,
  // '*' = 262,
  // '/' = 263,
  // '(' = 264,
  // ')' = 265

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"0x[0-9a-fA-F]+", TK_HEX_NUM},  // 匹配十六进制数
  {"[0-9]+", TK_NUM},       // 匹配整数
  {"[a-zA-Z_][a-zA-Z0-9_]*", TK_IDENT}, // 匹配标识符
  {"\\$[a-zA-Z_][a-zA-Z0-9_]*", TK_REG}, // 匹配寄存器变量
  {"\\-", '-'},             // 匹配减号
  {"\\*", '*'},             // 匹配乘号
  {"/", '/'},               // 匹配除号
  {"\\(", '('},             // 匹配左括号
  {"\\)", ')'},             // 匹配右括号
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
          case TK_NOTYPE:
            break;
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            int len = substr_len < 31 ? substr_len : 31;
            strncpy(tokens[nr_token].str, substr_start, len);
            tokens[nr_token].str[len] = '\0';
            nr_token++;
            break;
          case TK_IDENT:
            tokens[nr_token].type = TK_IDENT;
            len = substr_len < 31 ? substr_len : 31;
            strncpy(tokens[nr_token].str, substr_start, len);
            tokens[nr_token].str[len] = '\0';
            nr_token++;
            break;
          case TK_HEX_NUM:
            tokens[nr_token].type = TK_HEX_NUM;
            len = substr_len < 31 ? substr_len : 31;
            strncpy(tokens[nr_token].str, substr_start, len);
            tokens[nr_token].str[len] = '\0';
            nr_token++;
            break;
          case TK_REG:
            tokens[nr_token].type = TK_REG;
            len = substr_len < 31 ? substr_len : 31;
            strncpy(tokens[nr_token].str, substr_start+1, len);
            tokens[nr_token].str[len] = '\0';
            nr_token++;
            break;
          case '+':
            tokens[nr_token].type = '+';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case '-':
            tokens[nr_token].type = '-';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case '*':
            tokens[nr_token].type = '*';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case '/':
            tokens[nr_token].type = '/';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case '(':
            tokens[nr_token].type = '(';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          case ')':
            tokens[nr_token].type = ')';
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          default: 
            printf("Unknown token type: %d\n", rules[i].token_type);
            return false;
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

bool check_parentheses(int p, int q){
  if(tokens[p].type != '(' || tokens[q].type != ')'){
    return false;
  }
  int balance = 0;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '('){
      balance++;
    }
    if(tokens[i].type == ')'){
      balance--;
      if(balance == 0 && i != q){
        printf("the left '(' and the right ')' are not matched!\n");
        return false;
      }
    }
  }

  return (balance == 0);
}

int find_op(int p, int q){
  int balance = 0;
  int op = -1;
  bool is_low = false;
  bool is_mid = false;
  bool is_eq = false;
  for(int i = p; i <= q; i++){
    
    if(tokens[i].type == '('){
      balance++;
    }
    else if(tokens[i].type == ')'){
      balance--;
    }
    else if(balance == 0){
      switch(tokens[i].type){
        case TK_EQ:
          op = i;
          is_eq = true;
          break;
        case '+':
          if(is_eq){
            break;
          }
          is_low = true;
          op = i;
          break;
        case '-':
          if(is_eq){
            break;
          }
          is_low = true;
          op = i;
          break;
        case '*':
          if(is_low || is_eq){
            break;
          }
          op = i;
          is_mid = true;
          break;
        case '/':
          if(is_low || is_eq){
            break;
          }
          op = i;
          is_mid = true;
          break;
        case TK_DEREF:
          if(is_low || is_mid || is_eq){
            break;
          }
          op = i;
          break;
        default:
          break;
      }
    }
  }

  return op;
}


uint32_t eval(int p, int q){
  
  if(p > q){
    printf("p:%d,q:%d\n",p,q);
    printf("bad expression!\n");
    return 0;
  }
  else if(p == q){
    if(tokens[p].type == TK_NUM){
      return atoi(tokens[p].str);
    }
    else if(tokens[p].type == TK_HEX_NUM){
      return strtol(tokens[p].str, NULL, 16);
    }
    else if(tokens[p].type == TK_REG){
      if(strcmp(tokens[p].str,"eax") == 0){
        return vaddr_read(cpu.eax, 4);
        // printf("right,%x\n",cpu.eax);
      }
      else if(strcmp(tokens[p].str,"ecx") == 0){
        return vaddr_read(cpu.ecx, 4);
      }
      else if(strcmp(tokens[p].str,"edx") == 0){
        return vaddr_read(cpu.edx, 4);
      }
      else if(strcmp(tokens[p].str,"ebx") == 0){
        return vaddr_read(cpu.ebx, 4);
      }
      else if(strcmp(tokens[p].str,"esp") == 0){
        return vaddr_read(cpu.esp, 4);
      }
      else if(strcmp(tokens[p].str,"ebp") == 0){
        return vaddr_read(cpu.ebp, 4);
      }
      else if(strcmp(tokens[p].str,"esi") == 0){
        return vaddr_read(cpu.esi, 4);
      }
      else if(strcmp(tokens[p].str,"edi") == 0){
        return vaddr_read(cpu.edi, 4);
      }
      else if(strcmp(tokens[p].str,"eip") == 0){
        return cpu.eip;
      }
    }
    else{
      printf("invalid type of tokens!\n");
      return 0;
    }
  }
  else if(check_parentheses(p, q)){
    return eval(p+1, q-1);
  }
  else{
    printf("$$$$\n");
    int op = find_op(p, q);
    if(op == -1){
      printf("no op found!\n");
      return 0;
    }
    if(tokens[op].type == TK_DEREF){
      
      uint32_t addr = eval(op+1, q);
      // uint32_t value = vaddr_read(addr, 4);
      // printf("0x%08x: 0x%08x\n", addr, value);
      
      return vaddr_read(addr, 4);
    }
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    switch (tokens[op].type){
    case TK_EQ:
      return val1 == val2;
      break;
    case '+':
    printf("%u\n",val1);
    printf("%u\n",val2);
      return val1 + val2;
      break;
    case '-':
      return val1 - val2;
      break;
    case '*':
      return val1 * val2;
      break;
    case '/':
      if(val2 == 0){
        printf("val2 can not be '0'!\n");
        return 0;
      }
      return val1 / val2;
      break;
    default:
      printf("invalid op!\n");
      assert(0);
      break;
    }
    printf("##$$\n");
  }
  printf("####\n");
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i = 0; i < nr_token; i++){
    if (tokens[i].type == '*' && 
        (i == 0 || 
         tokens[i-1].type == '+' || 
         tokens[i-1].type == '-' || 
         tokens[i-1].type == '*' || 
         tokens[i-1].type == '/')) {
        tokens[i].type = TK_DEREF;
    }
  }

  uint32_t result = eval(0, nr_token-1);
  *success = true;
  return result;
}
