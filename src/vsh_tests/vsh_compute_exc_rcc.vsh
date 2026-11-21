; c[188 + i] = rcc(c[96].i) for component i in [x, y, z, w]. The ILU result
; is written directly to the constant, with no intermediate MOV.
rcc c[188], c[96].x
rcc c[189], c[96].y
rcc c[190], c[96].z
rcc c[191], c[96].w
mov oPos, v0
