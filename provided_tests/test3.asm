; test3.asm
; Test SET
; This is an assembler-only sample: it defines constants and has no HALT instruction.
val: SET 75
ldc     val
adc     val2
val2: SET 66
