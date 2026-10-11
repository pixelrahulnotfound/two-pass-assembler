; test05.asm - sum 1..5 = 15, then mean = sum/5 = 3 using div
; tests loops, brz/brlz/br, add, sub, div, variables via ldnl/stnl
br main
i: data 1
n: data 5
s: data 0
main:
loop: ldc i
ldnl 0 ; A=i
ldc n
ldnl 0 ; A=n, B=i
sub ; A=i-n
brlz cont ; if i-n < 0 then i<n -> continue (also handles ==0? need <=0)
brz cont ; if i-n == 0 then i==n -> continue (include n)
br done ; else i>n -> done
cont: ldc s
ldnl 0 ; A=sum
ldc i
ldnl 0 ; A=i, B=sum
add ; A=sum+i
ldc s ; B=newsum, A=addr_s
stnl 0 ; s=newsum
ldc i
ldnl 0 ; A=i
adc 1 ; A=i+1
ldc i ; B=newi, A=addr_i
stnl 0 ; i=newi
br loop
done: ldc s
ldnl 0 ; A=sum=15
ldc tmp
stnl 0 ; tmp=sum (save, since next load would push sum to B - actually we want B=sum)
; want A=sum/n: need B=sum, A=n. Load n now: B=sum? trace: after store, A=addr_tmp,B=sum. ldc n: B=addr_tmp,A=addr_n. Oops lost sum. Reload:
ldc s
ldnl 0 ; A=sum
ldc n
ldnl 0 ; A=n, B=sum
div ; A=sum/n=3
HALT
tmp: data 0
