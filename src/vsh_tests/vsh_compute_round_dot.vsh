; For k in [0, 3], with A = c[96 + 2k] and B = c[97 + 2k]:
;   c[188].k = DP3(A, B), c[189].k = DPH(A, B), c[190].k = DP4(A, B)
mov r0, c[96]
dp3 c[188].x, r0, c[97]
dph c[189].x, r0, c[97]
dp4 c[190].x, r0, c[97]
mov r0, c[98]
dp3 c[188].y, r0, c[99]
dph c[189].y, r0, c[99]
dp4 c[190].y, r0, c[99]
mov r0, c[100]
dp3 c[188].z, r0, c[101]
dph c[189].z, r0, c[101]
dp4 c[190].z, r0, c[101]
mov r0, c[102]
dp3 c[188].w, r0, c[103]
dph c[189].w, r0, c[103]
dp4 c[190].w, r0, c[103]
mov oPos, v0
