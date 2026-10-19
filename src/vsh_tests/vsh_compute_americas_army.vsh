; Excerpt of a vertex shader from America's Army whose RCP(0) = INF result is
; later multiplied by 0. c[188] receives (R1.y * c[140].xyz, R1.y).
mov r3, c[97]
mul r6.xyzw, r3, c[121]

add r11.xyz, r6.xyz, -c[136].xyz
add r10.yzw, r6.yzx, -c[137].yzx
mul r2.xyz, r11.xyz, r11.xyz
dp3 r2.x, c[135].z, r2.xyz
mul r2.yzw, r10.yzw, r10.yzw
add r11.xyz, r6.xyz, -c[138].xyz
dp4 oPos.x, r6, c[96] + rsq r1.x, r2.x
dp3 r2.y, c[135].z, r2.wyz
mul r11.yzw, r11.yzx, r11.yzx
add r10.xyz, r6.xyz, -c[139].xyz
dst r9.xyz, r2.x, r1.x
dp3 r2.x, c[135].z, r11.wyz + rsq r1.x, r2.y
mul r11.yzw, r10.yzx, r10.yzx
dst r10.xyz, r2.y, r1.x
dp3 r2.y, c[135].z, r11.wyz + rsq r1.x, r2.x
dp3 r11.x, r9.xyz, c[144].xyz
dst r8.xyz, r2.x, r1.x
dp3 r11.y, r10.xyz, c[145].xyz + rsq r1.x, r2.y
slt r0.w, r9.y, c[144].w + rcp r1.y, r11.x
dst r11.xyz, r2.y, r1.x + rcp r1.z, r11.y
mul r7.xyz, r1.y, c[140].xyz

mul c[188].xyzw, r1.y, c[140].xyz
mov c[188].w, r1.y
mov oPos, v0
