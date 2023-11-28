; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

; This test checks that master thread guarding in SPIR-V target regions is able
; to make use of memory function attributes to determine if a function needs
; guarding.

; This test is based on target_spirv_private_memintrin_opaqueptrs.ll.

; Calls that only write to thread-local memory should not be under a master
; thread guard.
; CHECK-NOT: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NOT: br i1 %is.master.thread
; CHECK: call spir_func double @dummy_modf(double 0.000000e+00, ptr addrspace(4) %X.ascast.priv.gep.ascast)
; CHECK: call spir_func void @foo(ptr addrspace(4) %X.ascast.priv.gep.ascast, ptr addrspace(4) %X.ascast.priv.gep.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast)
; CHECK: call spir_func void @foo(ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast)
; CHECK: call spir_func void @bar()

; Calls that may write to non-thread-local memory or throw should be under a
; master thread guard.
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: br i1 %is.master.thread{{\d*}}, label %master.thread.code{{\d*}}, label %master.thread.fallthru{{\d*}}
; CHECK: master.thread.code{{\d*}}:
; CHECK: call spir_func double @dummy_modf(double 0.000000e+00, ptr addrspace(4) %A.ascast)
; CHECK: call spir_func void @foo(ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %Y.ascast.priv.gep.ascast, ptr addrspace(4) %Y.ascast.priv.gep.ascast)
; CHECK: call spir_func void @bar()
; CHECK: call spir_func void @bar()
; CHECK: call spir_func void @foo_maythrow(ptr addrspace(4) %Y.ascast.priv.gep.ascast, ptr addrspace(4) %Y.ascast.priv.gep.ascast, ptr addrspace(4) %Y.ascast.priv.gep.ascast, ptr addrspace(4) %Y.ascast.priv.gep.ascast)
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
  %Y = alloca [32 x double], align 8
  %X.ascast = addrspacecast ptr %X to ptr addrspace(4)
  %Y.ascast = addrspacecast ptr %Y to ptr addrspace(4)
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.310
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([32 x double] addrspace(1)* @A, [32 x double] addrspace(1)* @A, i64 256, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"([32 x double] addrspace(4)* %X.ascast, double 0.000000e+00, i64 32),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"([32 x double] addrspace(4)* %Y.ascast, double 0.000000e+00, i64 32),
    "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  br label %DIR.OMP.TARGET.312

DIR.OMP.TARGET.312:                               ; preds = %DIR.OMP.TARGET.2
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.5, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.312
  %A.ascast = addrspacecast ptr addrspace(1) @A to ptr addrspace(4)

  %1 = call spir_func double @dummy_modf(double 0.000000e+00, ptr addrspace(4) %X.ascast)
  call spir_func void @foo(ptr addrspace(4) %X.ascast, ptr addrspace(4) %X.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast)
  call spir_func void @foo(ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast) memory(argmem:read)
  call spir_func void @bar() memory(read)

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  %3 = call spir_func double @dummy_modf(double 0.000000e+00, ptr addrspace(4) %A.ascast)
  call spir_func void @foo(ptr addrspace(4) %A.ascast, ptr addrspace(4) %A.ascast, ptr addrspace(4) %Y.ascast, ptr addrspace(4) %Y.ascast)
  call spir_func void @bar()
  call spir_func void @bar() memory(inaccessiblemem:write)
  call spir_func void @foo_maythrow(ptr addrspace(4) %Y.ascast, ptr addrspace(4) %Y.ascast, ptr addrspace(4) %Y.ascast, ptr addrspace(4) %Y.ascast)

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

declare spir_func double @dummy_modf(double, ptr addrspace(4) noalias nocapture writeonly) memory(argmem: write) nounwind
declare spir_func void @foo(ptr addrspace(4), ptr addrspace(4) writeonly, ptr addrspace(4) readonly, ptr addrspace(4) readnone) memory(read, argmem:readwrite) nounwind
declare spir_func void @foo_maythrow(ptr addrspace(4), ptr addrspace(4) writeonly, ptr addrspace(4) readonly, ptr addrspace(4) readnone) memory(read, argmem:readwrite)
declare spir_func void @bar() nounwind

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"_Z4main", i32 6, i32 0, i32 0, i32 0}
