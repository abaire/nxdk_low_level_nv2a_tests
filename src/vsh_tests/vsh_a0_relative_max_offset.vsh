; Loads A0 from the x component of iTex0 and emits c[A0 + 191] as the diffuse
; color. 191 is the largest static offset the assembler accepts.
mov oPos, iPos
arl a0, iTex0
mov oDiffuse, c[A0 + 191]
