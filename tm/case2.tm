; the finite set of states
#Q = {init,copy,cmp,cmp_less,cmp_incr1,cmp_incr2,cmp_back,cmp_greater,cmp_equal,false0,false1,false2,false3,true0,true1,true2,halt_accept}

; the finite set of input symbols
#S = {1}

; the complete set of tape symbols
#G = {1,_,t,r,u,e,f,a,l,s}

; the start state
#q0 = init

; the blank symbol
#B = _

; the set of final states
#F = {halt_accept}

; the number of tapes
#N = 4

; the transition functions

; set a=1 (3rd tape)

init ____ __1_ **** copy
init *___ *_1_ **** copy

; empty input is false

init ____ f___ r*** false0
false0 ____ a___ r*** false1
false1 ____ l___ r*** false2
false2 ____ s___ r*** false3
false3 ____ e___ r*** halt_accept

; copy input to last tape
copy 1_1_ __11 r**l copy
copy __1_ __1_ ***r cmp

; compare input and s (2nd tape)
cmp _111 _111 *r*r cmp
cmp __11 __11 **** cmp_less
cmp _11_ _11_ **** cmp_greater
cmp __1_ __1_ **** cmp_equal
cmp_less __11 _111 *rr* cmp_less

; a+=2 then go back
cmp_less ___1 __11 **r* cmp_incr1
cmp_incr1 ___1 __11 **r* cmp_incr2
cmp_incr2 ___1 ___1 *ll* cmp_back

cmp_back _111 _111 *ll* cmp_back
cmp_back __11 __11 **l* cmp_back
cmp_back _1_1 _1_1 *l** cmp_back
cmp_back ___1 ___1 ***l cmp_back
cmp_back ____ ____ *rrr cmp

; greater -> not a perfect square
cmp_greater _11_ f11_ r*** false0
false0 _11_ a11_ r*** false1
false1 _11_ l11_ r*** false2
false2 _11_ s11_ r*** false3
false3 _11_ e11_ r*** halt_accept

; equal -> a perfect square
cmp_equal __1_ t_1_ r*** true0
true0 __1_ r_1_ r*** true1
true1 __1_ u_1_ r*** true2
true2 __1_ e_1_ r*** halt_accept