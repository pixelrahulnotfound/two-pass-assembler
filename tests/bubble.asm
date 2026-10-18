; bubble.asm - bubble sort 5 numbers ascending
; arr = [5,3,4,1,2] -> sorted [1,2,3,4,5]
; uses pointer p, lens, tmp variables; outer i, inner j
; load pattern: ldc var / ldnl 0 => A=var
; store pattern: (A=value) ldc var / stnl 0
; array load mem[p]: ldc p / ldnl 0 / ldnl 0
; array load mem[p+1]: ldc p / ldnl 0 / adc 1 / ldnl 0
br main
arr: data 5
data 3
data 4
data 1
data 2
n: data 5
i: data 0
j: data 0
p: data 0
tmpA: data 0
tmpB: data 0
tmpP: data 0
main:
; i = 0
ldc 0
ldc i
stnl 0
outer:
; if i >= n-1 -> done : compute (n-1)-i, brlz done
ldc n
ldnl 0
adc -1 ; A=n-1
ldc tmpA
stnl 0 ; tmpA=n-1
ldc i
ldnl 0 ; A=i
ldc tmpA
ldnl 0 ; A=n-1, B=i -> sub = i-(n-1)? No: B=i? trace A=i, ldc tmpA:B=i,A=addr, ldnl:A=n-1,B=i, sub:i-(n-1). want (n-1)-i so reverse:
; reload correctly: load tmpA then load i
ldc tmpA
ldnl 0 ; A=n-1
ldc i
ldnl 0 ; A=i, B=n-1
sub ; A=(n-1)-i
brlz done ; if n-1-i < 0 -> done
brz done ; if ==0 -> still need? i==n-1 means outer done (no inner). yes done
; j = 0
ldc 0
ldc j
stnl 0
; p = arr (base address)
ldc arr
ldc p
stnl 0
inner:
; limit = n-1-i : compute n-1-i, compare j-limit <0?
ldc n
ldnl 0
adc -1
ldc tmpA
stnl 0 ; tmpA=n-1
ldc tmpA
ldnl 0 ; A=n-1
ldc i
ldnl 0 ; A=i,B=n-1
sub ; A=n-1-i -> limit
ldc tmpB
stnl 0 ; tmpB=limit
ldc j
ldnl 0 ; A=j
ldc tmpB
ldnl 0 ; A=limit,B=j
sub ; A=j-limit
brlz innerBody ; j<limit -> body
brz innerBody ; j==limit? No: j must be < limit (since j+1 <= n-1-i-? ). For bubble, j < n-1-i. If == -> exit
br innerNext ; j>=limit -> next outer
innerBody:
; a = mem[p]
ldc p
ldnl 0
ldnl 0 ; A=a
ldc tmpA
stnl 0 ; tmpA=a
; b = mem[p+1]
ldc p
ldnl 0 ; A=p
adc 1 ; A=p+1
ldnl 0 ; A=b
ldc tmpB
stnl 0 ; tmpB=b
; compare a-b: load tmpA then tmpB -> B=a,A=b -> sub=a-b? trace: load tmpA:A=a, load tmpB: ldc tmpB:B=a,A=addr, ldnl:A=b,B=a, sub=a-b
ldc tmpA
ldnl 0 ; A=a
ldc tmpB
ldnl 0 ; A=b,B=a
sub ; A=a-b
brlz noswap ; if a-b<0 -> a<b -> no swap
brz noswap ; if == -> no swap
; swap: mem[p]=b, mem[p+1]=a
; store b at p: load tmpB (A=b), load p addr -> B=b,A=p -> stnl
ldc tmpB
ldnl 0 ; A=b
ldc p
ldnl 0 ; A=p,B=b
stnl 0 ; mem[p]=b
; store a at p+1: load tmpA (A=a), load p -> A=p, adc1 -> A=p+1, B=a -> stnl
ldc tmpA
ldnl 0 ; A=a
ldc p
ldnl 0 ; A=p,B=a
adc 1 ; A=p+1,B=a
stnl 0 ; mem[p+1]=a
noswap:
; p = p+1
ldc p
ldnl 0
adc 1
ldc p
stnl 0
; j = j+1
ldc j
ldnl 0
adc 1
ldc j
stnl 0
br inner
innerNext:
; i = i+1
ldc i
ldnl 0
adc 1
ldc i
stnl 0
br outer
done: HALT
