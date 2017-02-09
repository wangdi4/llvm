; Test whether getSequence() generates four contiguous loads plus 8 shuffles
; for the four gathers in this test, which seems to be the optimized load+shuffle
; sequence.
;  %1 = mask.load.64.4 (<Base:0x38eea50 Offset:0>, 1111)
;  %2 = mask.load.64.4 (<Base:0x38eea50 Offset:32>, 1111)
;  %3 = mask.load.64.4 (<Base:0x38eea50 Offset:64>, 1111)
;  %4 = mask.load.64.4 (<Base:0x38eea50 Offset:96>, 1111)
;  %5 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><0, 4, 2, 6>
;  %6 = shufflevector <4 x 64> %1, <4 x 64> %2, <4 x 32><1, 5, 3, 7>
;  %7 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><0, 4, 2, 6>
;  %8 = shufflevector <4 x 64> %3, <4 x 64> %4, <4 x 32><1, 5, 3, 7>
;  %9 = shufflevector <4 x 64> %5, <4 x 64> %7, <4 x 32><0, 1, 4, 5>
;  %10 = shufflevector <4 x 64> %5, <4 x 64> %7, <4 x 32><2, 3, 6, 7>
;  %11 = shufflevector <4 x 64> %6, <4 x 64> %8, <4 x 32><0, 1, 4, 5>
;  %12 = shufflevector <4 x 64> %6, <4 x 64> %8, <4 x 32><2, 3, 6, 7>
;
; REQUIRES: asserts
; RUN: intelovls-test < %s 2>&1 | FileCheck %s
;
; intelovls-test is not built with the required target information by default
; in order to keep its size small. Lack of target info makes this test fail.
; To test manually define OVLSTESTCLIENT in intelovls-test.h
; XFAIL: *
;
; CHECK: [[TMP1:%.*]] = mask.load.64.4 (<[[Base:.*]] Offset:0>, 1111)
; CHECK-NEXT: [[TMP2:%.*]] = mask.load.64.4 (<[[Base:.*]] Offset:32>, 1111)
; CHECK-NEXT: [[TMP3:%.*]] = mask.load.64.4 (<[[Base:.*]] Offset:64>, 1111)
; CHECK-NEXT: [[TMP4:%.*]] = mask.load.64.4 (<[[Base:.*]] Offset:96>, 1111)
; CHECK-NEXT: [[TMP5:%.*]] = shufflevector <4 x 64> [[TMP1]], <4 x 64> [[TMP2]], <4 x 32><0, 4, 2, 6>
; CHECK-NEXT: [[TMP6:%.*]] = shufflevector <4 x 64> [[TMP1]], <4 x 64> [[TMP2]], <4 x 32><1, 5, 3, 7>
; CHECK-NEXT: [[TMP7:%.*]] = shufflevector <4 x 64> [[TMP3]], <4 x 64> [[TMP4]], <4 x 32><0, 4, 2, 6>
; CHECK-NEXT: [[TMP8:%.*]] = shufflevector <4 x 64> [[TMP3]], <4 x 64> [[TMP4]], <4 x 32><1, 5, 3, 7>
; CHECK-NEXT: [[TMP9:%.*]] = shufflevector <4 x 64> [[TMP5]], <4 x 64> [[TMP7]], <4 x 32><0, 1, 4, 5>
; CHECK-NEXT: [[TMP10:%.*]] = shufflevector <4 x 64> [[TMP5]], <4 x 64> [[TMP7]], <4 x 32><2, 3, 6, 7>
; CHECK-NEXT: [[TMP11:%.*]] = shufflevector <4 x 64> [[TMP6]], <4 x 64> [[TMP8]], <4 x 32><0, 1, 4, 5>
; CHECK-NEXT: [[TMP12:%.*]] = shufflevector <4 x 64> [[TMP6]], <4 x 64> [[TMP8]], <4 x 32><2, 3, 6, 7>
; CHECK-NOT: shufflevector
# 32
1 A 0 f64 4 SLoad C 32
2 A 8 f64 4 SLoad C 32
3 A 16 f64 4 SLoad C 32
4 A 24 f64 4 SLoad C 32