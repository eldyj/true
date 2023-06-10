#include <stdio.h>
#define TSTACK_S 16384
#define TSTR_S 64
typedef long tunit_t;

typedef enum {
  ADIG,
  ASTR
} atomk_t;

typedef struct {
  atomk_t k;
  tunit_t d;
  char s[TSTR_S];
} atom_t;

static atom_t stk[TSTACK_S] = {0};
int stki = 0;
int stri = -1;
int esc = 0;
char tmp[TSTR_S] = {0};
char tmp2[TSTR_S];

short eval_s(const char *s);
short eval_f(const char *const s);

short
eval(const char c)
{
  if (stki >= TSTACK_S) {
    fputs("stack overflow", stderr);
    return 1;
  }

  if (stri > -1) {
    if (c == '\\' && !esc) {
      stk[stki++].s[stri] = 0;
      stri = -1;
      return 0;
    }

    if (c == '%' && !esc) {
      esc = 1;
      return 0;
    }

    stk[stki].s[stri++] = c;
    return esc = 0;
  }

  if (esc)
    return esc = 0;

  if (c >= '0' && c <= '9') {
    stk[stki].k = ADIG;
    stk[stki++].d = c-'0';
    return 0;
  }

  switch (c) {
    case '+': {
      --stki;
      stk[stki-1].d += stk[stki].d;
      break;
    }
    
    case '-': {
      --stki;
      stk[stki-1].d -= stk[stki].d;
      break;
    }
    
    case '*': {
      --stki;
      stk[stki-1].d *= stk[stki].d;
      break;
    }
    
    case '/': {
      --stki;
      stk[stki-1].d /= stk[stki].d;
      break;
    }
    
    case '%': {
      --stki;
      stk[stki-1].d %= stk[stki].d;
      break;
    }
    
    case '|': {
      --stki;
      stk[stki-1].d |= stk[stki].d;
      break;
    }
    
    case '&': {
      --stki;stk[stki-1].d &= stk[stki].d;
      break;
    }
    
    case '^': {
      --stki;
      stk[stki-1].d ^= stk[stki].d;
      break;
    }
    
    case '}': {
      --stki;
      stk[stki-1].d >>= stk[stki].d;
      break;
    }
    
    case '{': {
      --stki;
      stk[stki-1].d <<= stk[stki].d;
      break;
    }

    case '>': {
      --stki;
      stk[stki-1].d = stk[stki-1].d > stk[stki].d;
      break;
    }

    case '(': {
      eval('>');
      eval('!');
      break;
    }
    
    case '<': {
      --stki;
      stk[stki-1].d = stk[stki-1].d < stk[stki].d;
      break;
    }

    case ')': {
      eval('<');
      eval('!');
      break;
    }
    
    case '=': {
      --stki;
      stk[stki-1].d = stk[stki-1].d == stk[stki].d;
      break;
    }
    
    case '~': {
      stk[stki-1].d = ~stk[stki-1].d;
      break;
    }
    
    case '!': {
      stk[stki-1].d = !stk[stki-1].d;
      break;
    }

    case '$': {
      --stki;
      break;
    }

    case '\\': {
      stk[stki].k = ASTR;
      stri = 0;
      break;
    }

    case ':': {
      eval(']');
      eval(';');
      break;
    }

    case ';': {
      puts(stk[--stki].s);
      break;
    }

    case ']': {
      stk[stki-1].k = ASTR;
      snprintf(stk[stki-1].s, TSTR_S-1, "%ld", stk[stki-1].d);
      break;
    }

    case '[': {
      stk[stki-1].k = ADIG;
      stk[stki-1].d = atoi(stk[stki-1].s);
      break;
    }

    case '.': {
      eval(',');
      eval('[');
      break;
    }

    case ',': {
      stk[stki].k = ASTR;
      scanf("%s", stk[stki++].s);
      break;
    }

    case '@': {
      stk[stki].k = stk[stki-1].k;
      stk[stki].d = stk[stki-1].d;
      strncpy(stk[stki].s, stk[stki-1].s, TSTR_S-1);
      ++stki;
      break;
    }

    case ' ':
    case '\n':
    case '\t': {
      break;
    }

    case '#': {
      eval('=');
      eval('!');
      break;
    }

    case '_': {
      strncpy(tmp, stk[--stki].s, TSTR_S-1);
    }

    case 'e': {
      return eval_s(tmp);
      break;
    }

    case 'i': {
      printf("%u\n", stki);
      break;
    }

    case '?': {
      esc = stk[--stki].d;
      break;
    }

    case 's': {
      stk[stki].d = ASTR;
      strncpy(stk[stki++].s, tmp, TSTR_S-1);
      break;
    }

    case 'r': {
      tunit_t td = stk[stki-2].d;
      atomk_t tk = stk[stki-2].k;
      strncpy(tmp2, stk[stki-2].s, TSTR_S-1);
      stk[stki-2].d = stk[stki-1].d;
      stk[stki-2].k = stk[stki-1].k;
      strncpy(stk[stki-2].s, stk[stki-1].s, TSTR_S-1);
      stk[stki-1].d = td;
      stk[stki-1].k = tk;
      strncpy(stk[stki-1].s, tmp2, TSTR_S-1);
      break;
    }

    case 'f': {
      return eval_f(stk[--stki].s);
      break;
    }

    default: {
      fprintf(stderr, "invalid char: %c\n", c);
      return 1;
    }
  }

  return 0;
}

short
eval_s(const char *str)
{
  while (*str)
    if (eval(*str++))
      return 1;

  return 0;
}

short
eval_f(const char *const str)
{
  FILE *f = fopen(str, "rb");
  
  while (1) {
    char c = fgetc(f);
    if (feof(f)) break;
    if (eval(c)) {
      fclose(f);
      return 1;
    }
  }
  
  fclose(f);
  return 0;
}

int
main(int argc, char **argv)
{
  if (argc-1 < 1)
    return 1;

  return eval_f(argv[1]);
}
