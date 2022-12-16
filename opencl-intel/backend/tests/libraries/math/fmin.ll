; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc '_Z4fminDv16_fS_' -S | FileCheck %s

; CHECK: define {{.*}}<16 x float> @_Z4fminDv16_fS_(<16 x float> {{.*}}[[A:%.*]], <16 x float> {{.*}}[[B:%.*]])
; CHECK-NEXT: entry:
; CHECK-NEXT: [[ISNaN:%.*]] = {{.*}} call <16 x i32> @_Z5isnanDv16_f(<16 x float> {{.*}}[[A]])
; CHECK-NEXT: [[CMP:%.*]] = fcmp contract olt <16 x float> [[B]], [[A]]
; CHECK-NEXT: [[ISNEG:%.*]] = icmp slt <16 x i32> [[ISNaN]], zeroinitializer
; CHECK-NEXT: [[CMP2:%.*]] = or <16 x i1> [[ISNEG]], [[CMP]]
; CHECK-NEXT: select <16 x i1> [[CMP2]], <16 x float> [[B]], <16 x float> [[A]]
