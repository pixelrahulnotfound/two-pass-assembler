; test01.asm - valid nonsense file, should assemble with no errors
; tests labels, forward refs, negative/positive numbers, branch offsets
label: ; an unused label (warning ok)
ldc 0
ldc -5
ldc +5
loop: br loop ; infinite loop, offset should be -1
br next ; offset should be zero
next:
ldc loop ; load code address
ldc var1 ; forward ref
var1: data 0 ; a variable
HALT
