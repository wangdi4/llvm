; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

; This test checks that memory intrinsics (memcpy, memmove, memset) are guarded
; correctly in SPIR-V target regions.

; Original code:
; double A [32] = {0};
;
; int main() {
; #pragma omp target
;   {
;     double X [32];
;     memcpy(X, A, 256);
;     memmove(X, A, 256);
;     memset(X, 0, 256);
;     memcpy(A, X, 256);
;     memmove(A, X, 256);
;     memset(A, 0, 256);
;   }
; }

; memcpy, memmove, and memset should not be under a master thread guard if
; their destination is thread-local because the local value would not be
; assigned on any of the non-master threads.
; CHECK-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NOT: br i1 %is.master.thread
; CHECK: call void @llvm.memcpy.p0.p1.i64(ptr align 8 %X.ascast.priv.gep, ptr addrspace(1) align 8 %A, i64 256, i1 false)
; CHECK: call void @llvm.memmove.p0.p1.i64(ptr align 8 %X.ascast.priv.gep, ptr addrspace(1) align 8 %A, i64 256, i1 false)
; CHECK: call void @llvm.memset.p0.i64(ptr align 8 %X.ascast.priv.gep, i8 0, i64 256, i1 false)

; memcpy, memmove, and memset should be under a master thread guard if
; assigning to non-thread-local memory.
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: br i1 %is.master.thread{{\d*}}, label %master.thread.code{{\d*}}, label %master.thread.fallthru{{\d*}}
; CHECK: master.thread.code{{\d*}}:
; CHECK: call void @llvm.memcpy.p1.p0.i64(ptr addrspace(1) align 8 %A, ptr align 8 %X.ascast.priv.gep, i64 256, i1 false)
; CHECK: call void @llvm.memmove.p1.p0.i64(ptr addrspace(1) align 8 %A, ptr align 8 %X.ascast.priv.gep, i64 256, i1 false)
; CHECK: call void @llvm.memset.p1.i64(ptr addrspace(1) align 8 %A, i8 0, i64 256, i1 false)
; CHECK: br label %master.thread.fallthru{{\d*}}
; CHECK: master.thread.fallthru{{\d*}}:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@A = external addrspace(1) global [32 x double], align 8

; Function Attrs: convergent noinline nounwind
define protected i32 @main() local_unnamed_addr {
DIR.OMP.TARGET.310:
  %X = alloca [32 x double], align 8
  %X.ascast = addrspacecast ptr %X to ptr addrspace(4)
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.310
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([32 x double] addrspace(1)* @A, [32 x double] addrspace(1)* @A, i64 256, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"([32 x double] addrspace(4)* %X.ascast, double 0.000000e+00, i64 32),
    "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  br label %DIR.OMP.TARGET.312

DIR.OMP.TARGET.312:                               ; preds = %DIR.OMP.TARGET.2
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.5, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.312
  %1 = addrspacecast ptr addrspace(1) @A to ptr addrspace(4)
  call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %X.ascast, ptr addrspace(4) noundef align 8 dereferenceable(256) %1, i64 256, i1 false)
  call void @llvm.memmove.p4.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %X.ascast, ptr addrspace(4) noundef align 8 dereferenceable(256) %1, i64 256, i1 false)
  call void @llvm.memset.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %X.ascast, i8 0, i64 256, i1 false)

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %1, ptr addrspace(4) noundef align 8 dereferenceable(256) %X.ascast, i64 256, i1 false)
  call void @llvm.memmove.p4.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %1, ptr addrspace(4) noundef align 8 dereferenceable(256) %X.ascast, i64 256, i1 false)
  call void @llvm.memset.p4.i64(ptr addrspace(4) noundef align 8 dereferenceable(256) %1, i8 0, i64 256, i1 false)
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.TARGET.312, %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.513

DIR.OMP.END.TARGET.513:                           ; preds = %DIR.OMP.END.TARGET.4
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i64, i1 immarg)
declare void @llvm.memmove.p4.p4.i64(ptr addrspace(4) nocapture writeonly, ptr addrspace(4) nocapture readonly, i64, i1 immarg)
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"_Z4main", i32 6, i32 0, i32 0, i32 0}
