; Pass-through check for iTex0.x. c[189] = (2^126, 2^75, 2^74, 1).
;   diffuse.x = |v * 2^126|  (0x00400000 -> 0.5 when preserved)
;   diffuse.y = |v * 2^75 * 2^74| (saturates for any preserved denormal)
mov oPos, iPos
mov r4, c[189]
mul r0, iTex0.x, r4.x
max oDiffuse.x, r0.x, -r0.x
mul r1, iTex0.x, r4.y
mul r1, r1.x, r4.z
max oDiffuse.y, r1.x, -r1.x
