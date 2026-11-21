; A = v1 (vertex attribute), B = c[96], C = c[97]:
;   c[184] = A * B, c[185] = B * A
;   c[186] = A * B + C, c[187] = B * A + C
;   c[188] = (DP4(A, B), DP3(A, B), DPH(A, B), DPH(B, A))
;   c[189] = A + B, c[190] = MIN(A, B), c[191] = MAX(A, B)
mul c[184], v1, c[96]
mul c[185], c[96], v1
mov r1, c[97]
mad c[186], v1, c[96], r1
mad c[187], c[96], v1, r1
dp4 c[188].x, v1, c[96]
dp3 c[188].y, v1, c[96]
dph c[188].z, v1, c[96]
dph c[188].w, c[96], v1
add c[189], v1, c[96]
min c[190], v1, c[96]
max c[191], v1, c[96]
mov oPos, v0
