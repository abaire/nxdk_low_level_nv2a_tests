; Final-draw projection and screen-space epilogue from Rugby 2005
; (xemu-project/xemu#2388). The original v0 input is supplied via v1.
;   c[188] = R12 after the DP4 projection
;   c[189] = R12 after the MUL by the viewport scale
;   c[190] = R1.x = RCC(R12.w)
;   c[191] = R12 after the final MAD
dp4 oPos.x, v1, c[0]
dp4 oPos.y, v1, c[1]
dp4 oPos.z, v1, c[2]
dp4 oPos.w, v1, c[3]

mov c[188], r12

mul oPos.xyz, r12, c[58]

mov c[189], r12

rcc r1.x, r12.w

mov c[190], r1.xxxx

mad oPos.xyz, r12, r1.x, c[59]

mov c[191], r12
mov oPos, v0
