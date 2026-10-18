; test08.asm - factorial 5! = 120 using a loop with mul
; tests mul in a loop, brz, br, sub via adc -1, variables
br main
n: data 5
acc: data 1
main:
loop: ldc n
ldnl 0 ; A=n
brz done ; if n==0 finish (mul needs A!=0 anyway)
; ---- acc = acc * n ----
ldc acc
ldnl 0 ; A=acc
ldc n
ldnl 0 ; A=n, B=acc
mul ; A=acc*n
ldc acc
stnl 0 ; acc=acc*n
; ---- n = n - 1 ----
ldc n
ldnl 0
adc -1 ; A=n-1
ldc n
stnl 0
br loop
done: ldc acc
ldnl 0 ; A=120
HALT
