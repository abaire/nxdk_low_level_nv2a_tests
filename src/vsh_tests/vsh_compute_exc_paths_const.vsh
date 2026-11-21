; As vsh_compute_exc_paths_attr.vsh, with A = c[98] moved into r0 instead of
; read from v1.
mov r0, c[98]
mul c[184], r0, c[96]
mul c[185], c[96], r0
mov r1, c[97]
mad c[186], r0, c[96], r1
mad c[187], c[96], r0, r1
dp4 c[188].x, r0, c[96]
dp3 c[188].y, r0, c[96]
dph c[188].z, r0, c[96]
dph c[188].w, c[96], r0
add c[189], r0, c[96]
min c[190], r0, c[96]
max c[191], r0, c[96]
mov oPos, v0
