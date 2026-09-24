#include <stdio.h>
#include <stdlib.h>
typedef struct OpInfo {
  const char *name;
  int opcode;
  int has_operand;
  int is_branch;
} OpInfo;

static const OpInfo op_table[] = {
    {"ldc", 0, 1, 0},   {"adc", 1, 1, 0},   {"ldl", 2, 1, 0},
    {"stl", 3, 1, 0},   {"ldnl", 4, 1, 0},  {"stnl", 5, 1, 0},
    {"add", 6, 0, 0},   {"sub", 7, 0, 0},   {"shl", 8, 0, 0},
    {"shr", 9, 0, 0},   {"adj", 10, 1, 0},  {"a2sp", 11, 0, 0},
    {"sp2a", 12, 0, 0}, {"call", 13, 1, 1}, {"return", 14, 0, 0},
    {"brz", 15, 1, 1},  {"brlz", 16, 1, 1}, {"br", 17, 1, 1},
    {"HALT", 18, 0, 0}, {"mul", 19, 0, 0},  {"div", 20, 0, 0},
    {"data", -1, 1, 0}, {"SET", -1, 1, 0}};
