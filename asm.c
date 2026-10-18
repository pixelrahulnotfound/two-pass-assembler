/*
 * IITPSx Assembler - CS2102 MiniProject 2026
 * Declaration: This is my own work, written from scratch.
 * Target: ISO C89, compile with:
 * gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes asm.c -o asm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct OpInfo {
  const char *name;
  int opcode;
  int has_operand;
  int is_branch;
} OpInfo;

/* Instruction metadata drives operand checks and branch-offset encoding. */
static const OpInfo op_table[] = {
  {"ldc", 0, 1, 0}, {"adc", 1, 1, 0}, {"ldl", 2, 1, 0},
  {"stl", 3, 1, 0}, {"ldnl", 4, 1, 0}, {"stnl", 5, 1, 0},
  {"add", 6, 0, 0}, {"sub", 7, 0, 0}, {"shl", 8, 0, 0},
  {"shr", 9, 0, 0}, {"adj", 10, 1, 0}, {"a2sp", 11, 0, 0},
  {"sp2a", 12, 0, 0}, {"call", 13, 1, 1}, {"return", 14, 0, 0},
  {"brz", 15, 1, 1}, {"brlz", 16, 1, 1}, {"br", 17, 1, 1},
  {"HALT", 18, 0, 0}, {"halt", 18, 0, 0}, {"mul", 19, 0, 0}, {"div", 20, 0, 0},
  /* Pseudo-ops are handled separately and do not use a machine opcode. */
  {"data", -1, 1, 0}, {"SET", -1, 1, 0}
};

/* The symbol table and input buffers use fixed bounds for this C89 implementation. */
#define MAX_SYM 1024
#define MAX_LINE 1024
#define MAX_LAB 64

typedef struct Symbol {
  char name[MAX_LAB + 1];
  int value;
  int is_set;
} Symbol;

static Symbol symtab[MAX_SYM];
static int nsym = 0;
static int nerr = 0;
static const char *curfile = "";

const OpInfo *findOp(const char *name);
int isValidLabel(const char *name);
int findSym(const char *name);
int addSym(const char *name, int val, int is_set);
void trim(char *s);
int parseNum(const char *s, long *out);
int isSetOp(const char *mn);
int isDataOp(const char *mn);
int assemble(const char *src, int pass, FILE *bin, FILE *lst);

const OpInfo *findOp(const char *name)
{
  int i;
  int n;
  if (name == NULL) {
    return NULL;
  }
  if (name[0] == '\0') {
    return NULL;
  }
  n = (int)(sizeof(op_table) / sizeof(op_table[0]));
  for (i = 0; i < n; i++) {
    if (strcmp(op_table[i].name, name) == 0) {
      return &op_table[i];
    }
  }
  return NULL;
}

int isValidLabel(const char *name)
{
  int i;
  if (name == NULL) {
    return 0;
  }
  if (name[0] == '\0') {
    return 0;
  }
  if (!isalpha((unsigned char)name[0])) {
    return 0;
  }
  for (i = 1; name[i] != '\0'; i++) {
    if (!isalnum((unsigned char)name[i])) {
      return 0;
    }
  }
  return 1;
}

int findSym(const char *name)
{
  /* Return the symbol-table index, or -1 when the name has not been defined. */
  int i;
  for (i = 0; i < nsym; i++) {
    if (strcmp(symtab[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

int addSym(const char *name, int val, int is_set)
{
  /* Keep each label unique and within the fixed-size symbol table. */
  if (nsym >= MAX_SYM) {
    return -1;
  }
  if (findSym(name) >= 0) {
    return -1;
  }
  strncpy(symtab[nsym].name, name, MAX_LAB);
  symtab[nsym].name[MAX_LAB] = '\0';
  symtab[nsym].value = val;
  symtab[nsym].is_set = is_set;
  nsym++;
  return 0;
}

void trim(char *s)
{
  /* Remove surrounding whitespace in place before tokenizing a source line. */
  char *p;
  char *q;
  size_t len;
  p = s;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
    p++;
  }
  if (p != s) {
    memmove(s, p, strlen(p) + 1);
  }
  len = strlen(s);
  while (len > 0) {
    q = s + len - 1;
    if (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') {
      *q = '\0';
      len--;
    } else {
      break;
    }
  }
}

/* return 1=number ok, 0=not a number (may be label), -1=invalid number */
int parseNum(const char *s, long *out)
{
  char *end;
  long v;
  if (s == NULL || s[0] == '\0') {
    return 0;
  }
  /* must start like a number: digit or sign followed by digit */
  if (s[0] == '+' || s[0] == '-') {
    if (s[1] == '\0') {
      return 0;
    }
    if (!isdigit((unsigned char)s[1]) && !(s[1] == '0')) {
      /* could still be sign+label? labels cannot start with sign, so invalid */
      /* treat as not-a-number only if it looks like label; sign never starts label */
      /* check: if rest is valid label chars -> invalid number */
      return -1;
    }
  } else if (!isdigit((unsigned char)s[0])) {
    return 0;
  }
  /* Base zero accepts decimal, 0x-prefixed hexadecimal, and leading-zero octal. */
  v = strtol(s, &end, 0);
  if (end == s) {
    return 0;
  }
  if (*end != '\0') {
    return -1;
  }
  *out = v;
  return 1;
}

int isSetOp(const char *mn)
{
  return strcmp(mn, "SET") == 0;
}

int isDataOp(const char *mn)
{
  return strcmp(mn, "data") == 0;
}

/*
 * Single routine for both passes.
 * pass 1: collect labels, no output, do not fail on undefined labels.
 * pass 2: output code, fail on undefined labels.
 */
int assemble(const char *src, int pass, FILE *bin, FILE *lst)
{
  FILE *fp;
  char line[MAX_LINE];
  char work[MAX_LINE];
  int pc;
  int lineno;
  int err0;

  err0 = nerr;
  fp = fopen(src, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s: cannot open source file\n", src);
    nerr++;
    return nerr - err0;
  }
  pc = 0;
  lineno = 0;
  while (fgets(line, (int)sizeof(line), fp) != NULL) {
    char *semi;
    char *colon;
    char label[MAX_LAB + 1];
    char rest[MAX_LINE];
    char mn[MAX_LAB + 1];
    char opstr[256];
    char extra[MAX_LINE];
    const OpInfo *op;
    int haslabel;
    int n;
    long numv;
    int pr;

    lineno++;
    /* Comments are removed before labels and operands are parsed. */
    semi = strchr(line, ';');
    if (semi != NULL) {
      *semi = '\0';
    }
    strcpy(work, line);
    /* check blank */
    trim(work);
    if (work[0] == '\0') {
      continue;
    }
    /* Separate an optional label from the statement on the same source line. */
    strcpy(work, line);
    /* Keep the untrimmed line so the label delimiter can be found reliably. */
    colon = strchr(work, ':');
    label[0] = '\0';
    rest[0] = '\0';
    haslabel = 0;
    if (colon != NULL) {
      char left[MAX_LINE];
      size_t ll;
      ll = (size_t)(colon - work);
      if (ll >= sizeof(left)) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: label too long\n", src, lineno);
          nerr++;
        }
        continue;
      }
      strncpy(left, work, ll);
      left[ll] = '\0';
      strcpy(rest, colon + 1);
      trim(left);
      trim(rest);
      if (left[0] == '\0') {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: empty label before ':'\n", src, lineno);
          nerr++;
        }
        /* still try rest as instruction */
      } else if (!isValidLabel(left)) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: invalid label name '%s'\n", src, lineno, left);
          nerr++;
        }
        /* still try rest */
      } else {
        strncpy(label, left, MAX_LAB);
        label[MAX_LAB] = '\0';
        haslabel = 1;
      }
    } else {
      strcpy(rest, work);
      trim(rest);
    }
    /* A label-only line names the current address without emitting a word. */
    if (haslabel && rest[0] == '\0') {
      if (pass == 1) {
        if (findSym(label) >= 0) {
          fprintf(stderr, "%s:%d: error: duplicate label '%s'\n", src, lineno, label);
          nerr++;
        } else {
          addSym(label, pc, 0);
        }
      } else {
        if (lst != NULL) {
          fprintf(lst, "%08X %s:\n", (unsigned)pc, label);
        }
      }
      continue;
    }
    if (!haslabel && rest[0] == '\0') {
      continue;
    }
    /* parse mnemonic / operand / extra from rest */
    mn[0] = '\0';
    opstr[0] = '\0';
    extra[0] = '\0';
    n = sscanf(rest, "%63s %255s %1023c", mn, opstr, extra);
    if (n < 1) {
      continue;
    }
    op = findOp(mn);
    if (op == NULL) {
      if (pass == 2) {
        fprintf(stderr, "%s:%d: error: unknown mnemonic '%s'\n", src, lineno, mn);
        nerr++;
      }
      /* keep PC in sync: assume 1 word */
      pc++;
      continue;
    }
    /* SET assigns a constant to a label and does not advance the program counter. */
    if (isSetOp(mn)) {
      long sv;
      int got;
      if (!haslabel) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: SET without label\n", src, lineno);
          nerr++;
        }
        continue;
      }
      if (n < 2) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: missing operand for SET\n", src, lineno);
          nerr++;
        }
        if (pass == 1) {
          addSym(label, 0, 1);
        }
        continue;
      }
      if (n >= 3) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: extra text after operand\n", src, lineno);
          nerr++;
        }
      }
      pr = parseNum(opstr, &sv);
      got = 0;
      if (pr == 1) {
        got = 1;
      } else if (pr == -1) {
        if (pass == 2) {
          fprintf(stderr, "%s:%d: error: invalid number '%s'\n", src, lineno, opstr);
          nerr++;
        }
        sv = 0;
        got = 1;
      } else {
        /* label operand for SET */
        if (!isValidLabel(opstr)) {
          if (pass == 2) {
            fprintf(stderr, "%s:%d: error: invalid operand '%s'\n", src, lineno, opstr);
            nerr++;
          }
          sv = 0;
          got = 1;
        } else {
          int si;
          si = findSym(opstr);
          if (si >= 0) {
            sv = symtab[si].value;
            got = 1;
          } else {
            if (pass == 2) {
              fprintf(stderr, "%s:%d: error: no such label '%s'\n", src, lineno, opstr);
              nerr++;
            }
            sv = 0;
            got = 1;
          }
        }
      }
      if (pass == 1) {
        if (findSym(label) >= 0) {
          fprintf(stderr, "%s:%d: error: duplicate label '%s'\n", src, lineno, label);
          nerr++;
        } else {
          addSym(label, (int)sv, 1);
        }
      } else {
        if (lst != NULL) {
          fprintf(lst, "         %s SET %ld\n", label, sv);
        }
      }
      (void)got;
      continue;
    }
    /* Ordinary labels name the address of their instruction or data word. */
    if (haslabel && pass == 1) {
      if (findSym(label) >= 0) {
        fprintf(stderr, "%s:%d: error: duplicate label '%s'\n", src, lineno, label);
        nerr++;
      } else {
        addSym(label, pc, 0);
      }
    }
    /* Validate operands using the instruction table before resolving their values. */
    if (op->has_operand && n < 2) {
      if (pass == 2) {
        fprintf(stderr, "%s:%d: error: missing operand for '%s'\n", src, lineno, mn);
        nerr++;
      }
      if (pass == 2 && lst != NULL && haslabel) {
        fprintf(lst, "%08X %s:\n", (unsigned)pc, label);
      }
      pc++;
      continue;
    }
    if (!op->has_operand && n >= 2) {
      if (pass == 2) {
        fprintf(stderr, "%s:%d: error: unexpected operand for '%s'\n", src, lineno, mn);
        nerr++;
      }
      if (pass == 2 && lst != NULL && haslabel) {
        fprintf(lst, "%08X %s:\n", (unsigned)pc, label);
      }
      pc++;
      continue;
    }
    if (n >= 3) {
      if (pass == 2) {
        fprintf(stderr, "%s:%d: error: extra text after operand\n", src, lineno);
        nerr++;
      }
      if (pass == 2 && lst != NULL && haslabel) {
        fprintf(lst, "%08X %s:\n", (unsigned)pc, label);
      }
      pc++;
      continue;
    }
    /* Resolve numeric operands and symbols; branches use a PC-relative offset. */
    {
      long v;
      int addr;
      unsigned word;

      v = 0;
      addr = pc;
      if (op->has_operand) {
        pr = parseNum(opstr, &numv);
        if (pr == 1) {
          v = numv;
        } else if (pr == -1) {
          if (pass == 2) {
            fprintf(stderr, "%s:%d: error: invalid number '%s'\n", src, lineno, opstr);
            nerr++;
          }
        } else {
          if (!isValidLabel(opstr)) {
            if (pass == 2) {
              fprintf(stderr, "%s:%d: error: invalid operand '%s'\n", src, lineno, opstr);
              nerr++;
            }
          } else {
            int si;
            si = findSym(opstr);
            if (si < 0) {
              if (pass == 2) {
                fprintf(stderr, "%s:%d: error: no such label '%s'\n", src, lineno, opstr);
                nerr++;
              }
              v = 0;
            } else {
              v = symtab[si].value;
              if (op->is_branch) {
                v = v - (addr + 1);
              }
            }
          }
        }
      }
      /* Pass 2 writes data words or packs a signed 24-bit operand and 8-bit opcode. */
      if (pass == 2 && bin != NULL && lst != NULL) {
        if (haslabel) {
          fprintf(lst, "%08X %s:\n", (unsigned)addr, label);
        }
        if (isDataOp(mn)) {
          word = (unsigned)(int)v;
          fwrite(&word, sizeof(word), 1, bin);
          fprintf(lst, "%08X %08X %s %s\n", (unsigned)addr, word, mn, opstr);
        } else if (op->has_operand) {
          unsigned ov;
          ov = ((unsigned)(int)v) & 0x00FFFFFFU;
          word = (ov << 8) | ((unsigned)op->opcode & 0xFFU);
          fwrite(&word, sizeof(word), 1, bin);
          fprintf(lst, "%08X %08X %s %s\n", (unsigned)addr, word, mn, opstr);
        } else {
          word = (unsigned)op->opcode & 0xFFU;
          fwrite(&word, sizeof(word), 1, bin);
          if (n >= 2) {
            fprintf(lst, "%08X %08X %s\n", (unsigned)addr, word, mn);
          } else {
            fprintf(lst, "%08X %08X %s\n", (unsigned)addr, word, mn);
          }
        }
      }
      pc++;
    }
  }
  fclose(fp);
  return nerr - err0;
}

int main(int argc, char *argv[])
{
  const char *src;
  const char *obj;
  const char *lstn;
  FILE *bin;
  FILE *lst;
  int e1;
  int e2;

  curfile = "asm";
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <src.asm> [obj.bin] [lst.lst]\n", argv[0]);
    return 1;
  }
  src = argv[1];
  obj = (argc >= 3) ? argv[2] : "out.bin";
  lstn = (argc >= 4) ? argv[3] : "out.lst";
  nsym = 0;
  nerr = 0;
  /* Pass 1 collects symbol addresses so forward references can be resolved. */
  e1 = assemble(src, 1, NULL, NULL);
  /* Pass 2 emits the object and listing after the symbol table is complete. */
  bin = fopen(obj, "wb");
  if (bin == NULL) {
    fprintf(stderr, "cannot open output %s\n", obj);
    return 1;
  }
  lst = fopen(lstn, "w");
  if (lst == NULL) {
    fprintf(stderr, "cannot open listing %s\n", lstn);
    fclose(bin);
    return 1;
  }
  e2 = assemble(src, 2, bin, lst);
  fclose(bin);
  fclose(lst);
  if (e1 + e2 > 0) {
    fprintf(stderr, "assembly failed with %d error(s)\n", e1 + e2);
    remove(obj);
    return 1;
  }
  printf("assembled %s -> %s, %s\n", src, obj, lstn);
  return 0;
}
