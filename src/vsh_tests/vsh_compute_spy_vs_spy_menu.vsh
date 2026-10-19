; Excerpt of the Spy vs Spy menu vertex shader, which reads oPos back through
; R12. The original v0 input is supplied via v1.
;   c[188] = R12 after the final oPos MAD
;   c[189] = R1.x = RCC(R12.w)
;   c[190] = R11
;   c[191] = R0
mul r0.yzw, v1.wxyz, c[120].zx
mov r0.x, c[125].z

mov c[191], r0

dp4 r11.z, r0.yzwx, c[98]
dp4 r11.x, r0.yzwx, c[96]
dp4 r11.y, r0.yzwx, c[97]

mov c[190], r11

dp4 oPos.w, r0.yzwx, c[99]
mov oPos.xyz, r11
rcc r1.x, r12.w

mov c[189], r1.xxxx

mul oPos.xyz, r12.xyz, c[58].xyz
mad oPos.xyz, r12.xyz, r1.x, c[59].xyz

mov c[188], r12
mov oPos, v0
