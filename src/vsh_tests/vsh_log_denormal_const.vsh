; Encodes LOGP(c[190].x) for denormal inspection:
;   diffuse.x = exponent + 160 (c[1].x = 1/255, c[160].x = 160/255)
;   diffuse.y = (mantissa - 1) * 255
;   diffuse.z = log2 + 160
mov oPos, iPos
mov r1, c[1]
mov r3, c[160]
logp r0, c[190].x
mad oDiffuse.x, r0.x, r1.x, r3.x
add oDiffuse.y, r0.y, -c[0].y
mad oDiffuse.z, r0.z, r1.x, r3.x
