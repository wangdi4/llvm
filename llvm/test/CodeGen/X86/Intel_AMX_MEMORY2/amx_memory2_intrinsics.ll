; REQUIRES: intel_feature_isa_amx_memory2
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-memory2 | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    tstorehd        %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstorehdt1      %tmm3, (%{{.*}},%{{.*}})
; CHECK:    tstorentd       %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstoreqd        %tmm5, (%{{.*}},%{{.*}})
; CHECK:    tstoreqdt1      %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstorerowd      %tmm1, (%{{.*}})
; CHECK:    tbroadcastrowd  (%{{.*}}), %tmm4

define void @test_amx(i64 %addr, i64 %addrx, i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1,i8* %addr2, <16 x float> %zmm, <4 x float> %xmm) {
call void @llvm.x86.tstorehd  (i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstorehdt1(i8* %addr1, i64 %stride, i8 3)
call void @llvm.x86.tstorentd (i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstoreqd  (i8* %addr1, i64 %stride, i8 5)
call void @llvm.x86.tstoreqdt1(i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstorerowd(i8* %addr1, i8 7)
call void @llvm.x86.tbroadcastrowd(i8 4, i8* %addr1)
ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
declare void @llvm.x86.tstorehd  (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorehdt1(i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorentd (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstoreqd  (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstoreqdt1(i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorerowd(i8* %addr1, i8 %tile1)
declare void @llvm.x86.tbroadcastrowd(i8 %tile1, i8* %addr)
