; Loads A0 from the x component of iTex0 and emits c[A0 + 0] as the diffuse
; color.
mov oPos, iPos
arl a0, iTex0
mov oDiffuse, c[A0 + 0]
