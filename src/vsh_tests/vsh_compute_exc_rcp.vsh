; c[188 + i] = rcp(c[96].i) for component i in [x, y, z, w]. The ILU result
; is written directly to the constant, with no intermediate MOV.
rcp c[188], c[96].x
rcp c[189], c[96].y
rcp c[190], c[96].z
rcp c[191], c[96].w
mov oPos, v0
