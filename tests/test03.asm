; test03.asm - Test SET pseudo-op
; val and val2 are constants, no memory allocated
val: SET 75
ldc val
adc val2
val2: SET 66
HALT
