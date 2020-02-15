; REQUIRES: intel_feature_isa_amx_transpose2
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-transpose2 | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-transpose2
; CHECK:    t2transposew    %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2transposewt1  %{{.*}}, (%{{.*}},%{{.*}}), %tmm4


define void @test_amx(i64 %addr, i64 %addrx, i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1) {
; amx-transpose2
call void @llvm.x86.t2transposew  (i8 3, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2transposewt1(i8 4, i8* %addr1, i64 %stride, i64 %rvalue)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-transpose2
declare void @llvm.x86.t2transposew  (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2transposewt1(i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
