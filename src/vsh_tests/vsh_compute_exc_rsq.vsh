; c[188 + i] = rsq(c[96].i) for component i in [x, y, z, w]. The ILU result
; is written directly to the constant, with no intermediate MOV.
rsq c[188], c[96].x
rsq c[189], c[96].y
rsq c[190], c[96].z
rsq c[191], c[96].w
mov oPos, v0
