; test07.asm - polynomial 2x^2 + 3x + 1 evaluated at x = 5
; expected: 2*25 + 15 + 1 = 66 in register A
; uses mul, add, adc, ldnl, stnl
br main
x: data 5
t: data 0
res: data 0
main:
; ---- t = x * x ----
ldc x
ldnl 0 ; A=x
ldc x
ldnl 0 ; A=x, B=x
mul ; A=x*x=25
ldc t
stnl 0 ; t=25
; ---- res = t + t = 2*x*x ----
ldc t
ldnl 0 ; A=25
ldc t
ldnl 0 ; A=25, B=25
add ; A=50
ldc res
stnl 0 ; res=50
; ---- A = x + x + x = 3*x ----
ldc x
ldnl 0 ; A=5
ldc x
ldnl 0 ; A=5, B=5
add ; A=10
ldc x
ldnl 0 ; A=5, B=10
add ; A=15
; ---- A = res + 3x + 1 = 66 ----
ldc res
ldnl 0 ; A=50, B=15
add ; A=65
adc 1 ; A=66
HALT
