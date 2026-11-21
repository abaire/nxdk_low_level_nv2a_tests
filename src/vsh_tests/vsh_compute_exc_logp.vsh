; c[188 + i] = logp(c[96].i) for component i in [x, y, z, w]. The ILU result
; is written directly to the constant, with no intermediate MOV.
logp c[188], c[96].x
logp c[189], c[96].y
logp c[190], c[96].z
logp c[191], c[96].w
mov oPos, v0
