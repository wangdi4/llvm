; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc _Z9bitselectDv64_Dh -S | FileCheck %s

; CHECK: define {{.*}} <64 x half> @_Z9bitselectDv64_DhS_S_(<64 x half> {{.*}} %a, <64 x half> {{.*}} %b, <64 x half> {{.*}} %c)
; CHECK-NEXT: entry:
; CHECK-NEXT: [[CAST_A:%.*]] = bitcast <64 x half> %a to <64 x i16>
; CHECK-NEXT: [[CAST_B:%.*]] = bitcast <64 x half> %b to <64 x i16>
; CHECK-NEXT: [[CAST_C:%.*]] = bitcast <64 x half> %c to <64 x i16>
; CHECK-NEXT: [[NEG:%.*]] = xor <64 x i16> [[CAST_C]], <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
; CHECK-NEXT: [[AND_A:%.*]] = and <64 x i16> [[NEG]], [[CAST_A]]
; CHECK-NEXT: [[AND_B:%.*]] = and <64 x i16> [[CAST_C]], [[CAST_B]]
; CHECK-NEXT: [[OR:%.*]] = or <64 x i16> [[AND_A]], [[AND_B]]
; CHECK-NEXT: [[RET:%.*]] = bitcast <64 x i16> [[OR]] to <64 x half>
; CHECK-NEXT: ret <64 x half> [[RET]]
