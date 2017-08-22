; Test whether getSequence() generates 2 contiguous loads followed by 4 shuffles
; for the 2 gathers in this test, which seems to be the optimized load+shuffle
; sequence.
; Expected sequence:
;  %1 = mask.load.64.4 (<Base:0x38eea50 Offset:0>, 1111)
;  %2 = mask.load.64.4 (<Base:0x38eea50 Offset:32>, 1111)
;  %3 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><0, 1, 4, 5>
;  %4 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><2, 3, 6, 7>
;  %5 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><0, 4, 2, 6>
;  %6 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><1, 5, 3, 7>
;
; REQUIRES: asserts
; RUN: intelovls-test < %s 2>&1 | FileCheck %s
;
; intelovls-test is not built with the required target information by default
; in order to keep its size small. Lack of target info makes this test fail.
; To test manually define OVLSTESTCLIENT in intelovls-test.h
; XFAIL: *
;
; CHECK: [[TMP1:%.*]] = mask.load.64.4 (<4 x 64>* <[[Base:.*]] Offset:0>, 1111)
; CHECK-NEXT: [[TMP2:%.*]] = mask.load.64.4 (<4 x 64>* <[[Base:.*]] Offset:32>, 1111)
; CHECK-NEXT: [[TMP3:%.*]] = shufflevector <4 x 64> [[TMP1]], <4 x 64> [[TMP2]], <4 x 32> <0, 1, 4, 5>
; CHECK-NEXT: [[TMP4:%.*]] = shufflevector <4 x 64> [[TMP1]], <4 x 64> [[TMP2]], <4 x 32> <2, 3, 6, 7>
; CHECK-NEXT: [[TMP5:%.*]] = shufflevector <4 x 64> [[TMP3]], <4 x 64> [[TMP4]], <4 x 32> <0, 4, 2, 6>
; CHECK-NEXT: [[TMP6:%.*]] = shufflevector <4 x 64> [[TMP3]], <4 x 64> [[TMP4]], <4 x 32> <1, 5, 3, 7>
; CHECK-NOT: shufflevector

# 32
1 A 0 f64 4 SLoad C 16
2 A 8 f64 4 SLoad C 16