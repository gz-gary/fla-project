; the finite set of states
#Q = {wait_a,wait_b,reject_back,reject0,reject1,reject2,reject3,reject4,reject5,reject6,reject7,reject8,reject9,reject10,reject11,reject12,reject13,accept_back,mult_forward,halt_reject,halt_accept,mult_backward,rm_left}

; the finite set of input symbols
#S = {a,b}

; the complete set of tape symbols
#G = {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,_}

; the start state
#q0 = wait_a

; the blank symbol
#B = _

; the set of final states
#F = {halt_reject,halt_accept}

; the number of tapes
#N = 3

; the transition functions

; State 0: start state
wait_a a__ a__ r** wait_a
wait_a b__ b__ r** wait_b
wait_b b__ b__ r** wait_b
wait_b a__ a__ l** reject_back
wait_b ___ ___ l** accept_back

; only a
wait_a ___ ___ l** rm_left
rm_left a__ ___ l**  rm_left
rm_left ___ ___ *** halt_accept

; go back when reject
reject_back ___ ___ r** reject0
reject_back *__ *__ l** reject_back
reject0 *__ i__ r** reject1
reject0 ___ i__ r** reject1
reject1 *__ l__ r** reject2
reject1 ___ l__ r** reject2
reject2 *__ l__ r** reject3
reject2 ___ l__ r** reject3
reject3 *__ e__ r** reject4
reject3 ___ e__ r** reject4
reject4 *__ g__ r** reject5
reject4 ___ g__ r** reject5
reject5 *__ a__ r** reject6
reject5 ___ a__ r** reject6
reject6 *__ l__ r** reject7
reject6 ___ l__ r** reject7
reject7 *__ ___ r** reject8
reject7 ___ ___ r** reject8
reject8 *__ i__ r** reject9
reject8 ___ i__ r** reject9
reject9 *__ n__ r** reject10
reject9 ___ n__ r** reject10
reject10 *__ p__ r** reject11
reject10 ___ p__ r** reject11
reject11 *__ u__ r** reject12
reject11 ___ u__ r** reject12
reject12 *__ t__ r** reject13
reject12 ___ t__ r** reject13
reject13 *__ ___ r** reject13
reject13 ___ ___ l** halt_reject

; go back and copy as and bs
accept_back b__ _b_ ll* accept_back
accept_back a__ __a l*l accept_back
accept_back ___ ___ *rr mult_forward

; do mult
mult_forward _ba cba r*r mult_forward
mult_forward _b_ _b_ **l mult_backward
mult_backward _ba _ba **l mult_backward
mult_backward _b_ _b_ *rr mult_forward
mult_forward __a __a *** halt_accept
