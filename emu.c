/* IITPSx Emulator - CS2102 MiniProject 2026
 * Name: YOUR NAME HERE
 * ID: YOUR ID HERE
 * Declaration: This is my own work, written from scratch.
 * Target: ISO C89, compile with:
 * gcc -std=c89 -pedantic -W -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes emu.c -o emu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMSIZE 65536
#define STEP_LIMIT 10000000

static int mem[MEMSIZE];
static int proglen = 0;

static const char *mnemonic(int op);
static int signExt24(unsigned raw);
static int checkAddr(int a, const char *what);

/* prototypes */
static const char *mnemonic(int op);
static int signExt24(unsigned raw);
static int checkAddr(int a, const char *what);
int main(int argc, char *argv[]);

static const char *mnemonic(int op)
{
  switch (op) {
    case 0: return "ldc";
    case 1: return "adc";
    case 2: return "ldl";
    case 3: return "stl";
    case 4: return "ldnl";
    case 5: return "stnl";
    case 6: return "add";
    case 7: return "sub";
    case 8: return "shl";
    case 9: return "shr";
    case 10: return "adj";
    case 11: return "a2sp";
    case 12: return "sp2a";
    case 13: return "call";
    case 14: return "return";
    case 15: return "brz";
    case 16: return "brlz";
    case 17: return "br";
    case 18: return "HALT";
    case 19: return "mul";
    case 20: return "div";
    default: return "???";
  }
}

static int signExt24(unsigned raw)
{
  raw = raw & 0x00FFFFFFU;
  if (raw & 0x00800000U) {
    raw |= 0xFF000000U;
  }
  return (int)raw;
}

static int checkAddr(int a, const char *what)
{
  if (a < 0 || a >= MEMSIZE) {
    fprintf(stderr, "runtime error: %s address %d out of bounds\n", what, a);
    return 0;
  }
  return 1;
}

int main(int argc, char *argv[])
{
  const char *obj;
  const char *dumpfile;
  FILE *fp;
  int A;
  int B;
  int PC;
  int SP;
  int steps;
  int dotrace;
  int dodump;
  int maxsteps;
  int i;
  int halted;
  unsigned uword;

  obj = NULL;
  dumpfile = NULL;
  dotrace = 0;
  dodump = 0;
  maxsteps = STEP_LIMIT;
  /* parse args */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-trace") == 0) {
      dotrace = 1;
    } else if (strcmp(argv[i], "-dump") == 0) {
      dodump = 1;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        dumpfile = argv[i + 1];
        i++;
      }
    } else if (strcmp(argv[i], "-steps") == 0) {
      if (i + 1 < argc) {
        maxsteps = atoi(argv[i + 1]);
        i++;
      }
    } else if (strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [-trace] [-dump [file]] [-steps N] <obj>\n", argv[0]);
      return 0;
    } else {
      obj = argv[i];
    }
  }
  if (obj == NULL) {
    fprintf(stderr, "Usage: %s [-trace] [-dump [file]] [-steps N] <obj>\n", argv[0]);
    return 1;
  }
  fp = fopen(obj, "rb");
  if (fp == NULL) {
    fprintf(stderr, "cannot open object %s\n", obj);
    return 1;
  }
  for (i = 0; i < MEMSIZE; i++) {
    mem[i] = 0;
  }
  proglen = 0;
  while (fread(&uword, sizeof(uword), 1, fp) == 1) {
    if (proglen >= MEMSIZE) {
      fprintf(stderr, "object too big\n");
      fclose(fp);
      return 1;
    }
    mem[proglen] = (int)uword;
    proglen++;
  }
  fclose(fp);
  A = 0;
  B = 0;
  PC = 0;
  SP = MEMSIZE - 1;
  steps = 0;
  halted = 0;
  while (!halted) {
    int instr;
    int op;
    int operand;
    int curpc;
    if (steps >= maxsteps) {
      fprintf(stderr, "runtime error: step limit %d exceeded (infinite loop?)\n", maxsteps);
      return 1;
    }
    if (!checkAddr(PC, "PC")) {
      return 1;
    }
    curpc = PC;
    instr = mem[PC];
    PC++;
    op = instr & 0xFF;
    operand = signExt24((unsigned)instr >> 8);
    /* data words executed as code will decode to bogus opcode -> trapped below */
    if (dotrace) {
      printf("%08X A=%d B=%d SP=%d : %s %d\n", (unsigned)curpc, A, B, SP, mnemonic(op), operand);
    }
    steps++;
    switch (op) {
      case 0: B = A; A = operand; break;
      case 1: A = A + operand; break;
      case 2:
        if (!checkAddr(SP + operand, "ldl")) return 1;
        B = A; A = mem[SP + operand]; break;
      case 3:
        if (!checkAddr(SP + operand, "stl")) return 1;
        mem[SP + operand] = A; A = B; break;
      case 4:
        if (!checkAddr(A + operand, "ldnl")) return 1;
        A = mem[A + operand]; break;
      case 5:
        if (!checkAddr(A + operand, "stnl")) return 1;
        mem[A + operand] = B; break;
      case 6: A = B + A; break;
      case 7: A = B - A; break;
      case 8:
        if (A < 0 || A >= 32) {
          fprintf(stderr, "runtime error: shl shift %d out of range\n", A);
          return 1;
        }
        A = (int)(((unsigned)B) << (unsigned)A); break;
      case 9:
        if (A < 0 || A >= 32) {
          fprintf(stderr, "runtime error: shr shift %d out of range\n", A);
          return 1;
        }
        A = B >> A; break;
      case 10: SP = SP + operand;
        if (!checkAddr(SP, "SP after adj")) return 1;
        break;
      case 11: SP = A; A = B;
        if (!checkAddr(SP, "SP after a2sp")) return 1;
        break;
      case 12: B = A; A = SP; break;
      case 13: B = A; A = PC; PC = PC + operand;
        if (!checkAddr(PC, "PC after call") && PC != proglen) return 1;
        break;
      case 14: PC = A; A = B; break;
      case 15: if (A == 0) PC = PC + operand; break;
      case 16: if (A < 0) PC = PC + operand; break;
      case 17: PC = PC + operand; break;
      case 18: halted = 1; break;
      case 19: A = B * A; break;
      case 20:
        if (A == 0) {
          fprintf(stderr, "runtime error: division by zero\n");
          return 1;
        }
        A = B / A; break;
      default:
        fprintf(stderr, "runtime error: invalid opcode %d at %d\n", op, curpc);
        return 1;
    }
    if (PC < 0 || PC >= MEMSIZE) {
      fprintf(stderr, "runtime error: PC %d out of bounds\n", PC);
      return 1;
    }
  }
  printf("HALT: A=%d B=%d PC=%d SP=%d steps=%d\n", A, B, PC, SP, steps);
  if (dodump) {
    FILE *df;
    df = stdout;
    if (dumpfile != NULL) {
      df = fopen(dumpfile, "w");
      if (df == NULL) {
        fprintf(stderr, "cannot open dump %s\n", dumpfile);
        return 1;
      }
    }
    for (i = 0; i < proglen; i++) {
      fprintf(df, "%08X %08X\n", (unsigned)i, (unsigned)mem[i]);
    }
    if (df != stdout) {
      fclose(df);
    }
  }
  return 0;
}
