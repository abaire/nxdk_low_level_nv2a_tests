; With A = c[96] and the same bits supplied as v1:
;   c[184] = A, c[185] = -A, c[186] = A.wzyx, c[187] = -A.yxwz
;   c[188] = A via r0, c[189] = -(A via r0)
;   c[190] = v1, c[191] = -v1
mov c[184], c[96]
mov c[185], -c[96]
mov c[186], c[96].wzyx
mov c[187], -c[96].yxwz
mov r0, c[96]
mov c[188], r0
mov c[189], -r0
mov c[190], v1
mov c[191], -v1
mov oPos, v0
