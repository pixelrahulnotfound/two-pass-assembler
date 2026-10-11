; test04.asm - Test mul and div, should leave 21 in A
ldc 6
ldc 7
mul ; A = 42
ldc 2
div ; A = 21
HALT
