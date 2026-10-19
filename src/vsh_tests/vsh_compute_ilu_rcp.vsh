; c[188 + i].n = RCP(c[96 + i].n) for i in [0, 3], one component at a time
; since RCP only consumes a single scalar.
rcp c[188].x, c[96].x
rcp c[188].y, c[96].y
rcp c[188].z, c[96].z
rcp c[188].w, c[96].w

rcp c[189].x, c[97].x
rcp c[189].y, c[97].y
rcp c[189].z, c[97].z
rcp c[189].w, c[97].w

rcp c[190].x, c[98].x
rcp c[190].y, c[98].y
rcp c[190].z, c[98].z
rcp c[190].w, c[98].w

rcp c[191].x, c[99].x
rcp c[191].y, c[99].y
rcp c[191].z, c[99].z
rcp c[191].w, c[99].w
mov oPos, v0
