; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc '_Z3madDv64_Dh' -S | FileCheck %s

; CHECK: define {{.*}}<64 x half> @_Z3madDv64_DhS_S_(<64 x half> {{.*}}[[A:%.*]], <64 x half> {{.*}}[[B:%.*]], <64 x half> {{.*}}[[C:%.*]])
; CHECK-NEXT: entry:
; CHECK-NEXT: [[MUL:%.*]] = fmul contract <64 x half> [[A]], [[B]]
; CHECK-NEXT: [[ADD:%.*]] = fadd contract <64 x half> [[MUL]], [[C]]
; CHECK-NEXT: ret <64 x half> [[ADD]]
