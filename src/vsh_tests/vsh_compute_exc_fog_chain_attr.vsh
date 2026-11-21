; Fixed function fog distance model applied to the eye-space position P = v1 (vertex attribute):
;   c[187] = rcp(P.w)
;   c[188] = p = P * rcp(P.w)
;   c[189] = d2 = DP3(p, p)
;   c[190] = rsq(d2)
;   c[191] = d = d2 * rsq(d2)
; Intermediates stay in temporaries; each is copied to its own output.
rcp r0.x, v1.w
mov c[187], r0.xxxx
mul r1, v1, r0.x
mov c[188], r1
dp3 r2.x, r1, r1
mov c[189], r2.xxxx
rsq r3.x, r2.x
mov c[190], r3.xxxx
mul r4.x, r2.x, r3.x
mov c[191], r4.xxxx
mov oPos, v0
