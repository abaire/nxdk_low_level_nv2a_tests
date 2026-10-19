; Checks which temporary register receives the ILU result of a paired
; MAC + ILU instruction.
;   c[188] = R1 after a paired RSQ targeting R1.x (control)
;   c[189] = R1 after being reset to c[96]
;   c[190] = R1 after a paired RSQ targeting R10.x
;   c[191] = R10 after a paired RSQ targeting R10.x
mov r6, c[96]

dp4 oD0.x, r6, c[96] + rsq r1.x, c[96].x

mov c[188], r1

mov r1, c[96]
mov r10, c[96]
mov c[189], r1

dp4 oD0.x, r6, c[96] + rsq r10.x, c[96].x

mov c[190], r1
mov c[191], r10
mov oPos, v0
