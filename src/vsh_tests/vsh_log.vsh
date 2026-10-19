; Emits LOGP(iTex0.x).x + c[0].z as the diffuse color. c[0].z is 128/255, so a
; correct negative exponent clamps to 0 while a wrapped unsigned exponent
; saturates to 255.
mov oPos, iPos
logp r0, iTex0.x
add oDiffuse, r0.xxxx, c[0].zzzz
