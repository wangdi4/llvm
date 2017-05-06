; Test whether getSequence() generates 1 shuffle followed by 1 store
; for the 2 scatters in this test.
; Input scatters:
; %1 = shufflevector <2 x 64>* <Base:0x39efb70 Offset:0>, <2 x 64> %undef, <2 x 32> <0, 2> // scatter1
; %2 = shufflevector <2 x 64>* <Base:0x39ef990 Offset:0>, <2 x 64> %undef, <2 x 32> <1, 3> // scatter2
;
; Expected sequence:
; %4 = shufflevector <2 x 64> %1, <2 x 64> %2, <4 x 32> <0, 2, 1, 3>
; call void @mask.store.64.4 (<4 x 64> %4, <4 x 64>* <Base:0x437cb70 Offset:0>, 1111)
; REQUIRES: asserts
; RUN: intelovls-test < %s 2>&1 | FileCheck %s
;
; intelovls-test is not built with the required target information by default
; in order to keep its size small. Lack of target info makes this test fail.
; To test manually define OVLSTESTCLIENT in intelovls-test.h
; XFAIL: *
;
; CHECK: [[TMP1:%.*]] = shufflevector <2 x 64>* <[[Base:.*]] Offset:0>, <2 x 64> %undef, <2 x 32> <0, 2>
; CHECK: [[TMP2:%.*]] = shufflevector <2 x 64>* <[[Base:.*]] Offset:0>, <2 x 64> %undef, <2 x 32> <1, 3>
; CHECK: [[TMP4:%.*]] = shufflevector <2 x 64> [[TMP1]], <2 x 64> [[TMP2]], <4 x 32> <0, 2, 1, 3>
; CHECK-NEXT: call void @mask.store.64.4 (<4 x 64> [[TMP4]], <4 x 64>* <[[Base:.*]] Offset:0>, 1111)
# 32
1 A 0 f64 2 SStore C 16
2 A 8 f64 2 SStore C 16