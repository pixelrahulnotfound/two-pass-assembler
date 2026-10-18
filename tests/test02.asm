; test02.asm - error handling, should produce errors for every line below
; each line has a comment explaining expected error
label:
label: ; duplicate label definition
br nonesuch ; no such label
ldc 08ge ; invalid number
ldc ; missing operand
add 5 ; unexpected operand
ldc 5, 6 ; extra text at end of line (comma counts as extra)
0def: ; invalid label name (starts with digit)
fibble ; bogus mnemonic
0def ; bogus mnemonic (also invalid use)
