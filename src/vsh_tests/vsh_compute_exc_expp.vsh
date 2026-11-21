; c[188 + i] = expp(c[96].i) for component i in [x, y, z, w]. The ILU result
; is written directly to the constant, with no intermediate MOV.
expp c[188], c[96].x
expp c[189], c[96].y
expp c[190], c[96].z
expp c[191], c[96].w
mov oPos, v0
