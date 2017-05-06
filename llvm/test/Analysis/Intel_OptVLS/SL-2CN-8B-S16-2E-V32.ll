; Test whether getSequence() generates 1 contiguous load followed by 2 shuffles
; for the 2 gathers in this test, which is considered to be the optimized load+shuffle
; sequence. The key-point of this test is the different vector length. The vector
; length of each gather is 16 bytes whereas the vector length of the underlying
; architecture is 32 bytes. 
; Expected sequence:
;  %1 = mask.load.64.4 (<Base:0x38eea50 Offset:0>, 1111)
;  %2 = shufflevector <4 x 64> %1, <4 x 64> %undef, <2 x 32> <0, 2>
;  %3 = shufflevector <4 x 64> %1, <4 x 64> %undef, <2 x 32> <1, 3>
;
; REQUIRES: asserts
; RUN: intelovls-test < %s 2>&1 | FileCheck %s
;
; intelovls-test is not built with the required target information by default
; in order to keep its size small. Lack of target info makes this test fail.
; To test manually define OVLSTESTCLIENT in intelovls-test.h
; XFAIL: *
;
; CHECK: mask.load.64.4
; CHECK: shufflevector
; CHECK: shufflevector
; CHECK-NOT: shufflevector
# 32
1 A 0 f64 2 SLoad C 16
2 A 8 f64 2 SLoad C 16