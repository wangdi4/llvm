; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -S %s | FileCheck %s

; This test ensures the compiler doesn't crash and does proper matching of
; global reduction buffers between Paropt prepare and transform passes when
; there're 2 reduction items: supported and unsupported.

; CHECK-LABEL: red.update.body:

; ModuleID = 'test.f90'
source_filename = "test.f90"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%"QNCA_a0$double addrspace(4)*$rank2$" = type { double addrspace(4)*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$double addrspace(4)*$rank2$.0" = type { double addrspace(4)*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"_unnamed_main$$_$DA2D" = internal addrspace(1) global %"QNCA_a0$double addrspace(4)*$rank2$" { double addrspace(4)* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@"_unnamed_main$$_$FA2D" = internal addrspace(1) global %"QNCA_a0$double addrspace(4)*$rank2$" { double addrspace(4)* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

; Function Attrs: noinline nounwind uwtable
define void @MAIN__() #0 {
alloca_0:
  %"_unnamed_main$$_$NINTN" = alloca i32, align 8
  %"ascastB$val" = addrspacecast i32* %"_unnamed_main$$_$NINTN" to i32 addrspace(4)*
  %fetch.1 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* getelementptr inbounds (%"QNCA_a0$double addrspace(4)*$rank2$", %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i32 0, i32 0), align 1
  %fetch.2 = load i64, i64 addrspace(4)* getelementptr inbounds (%"QNCA_a0$double addrspace(4)*$rank2$", %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i32 0, i32 1), align 1
  %"val$[]_fetch.3" = load i64, i64 addrspace(4)* getelementptr inbounds (%"QNCA_a0$double addrspace(4)*$rank2$", %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i32 0, i32 6, i32 0, i32 0), align 1
  %"val$[]1_fetch.4" = load i64, i64 addrspace(4)* getelementptr inbounds (i64, i64 addrspace(4)* getelementptr inbounds (%"QNCA_a0$double addrspace(4)*$rank2$", %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i32 0, i32 6, i32 0, i32 0), i64 3), align 1
  %mul.1 = mul nsw i64 %"val$[]_fetch.3", %"val$[]1_fetch.4"
  %mul.2 = mul nsw i64 %fetch.2, %mul.1
  br label %DIR.OMP.TARGET

call.post.list7:                                  ; preds = %DIR.OMP.TARGET
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void

DIR.OMP.TARGET:                                 ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %"ascastB$val", i32 addrspace(4)* %"ascastB$val", i64 4, i64 547, i8 addrspace(4)* null, i8 addrspace(4)* null), "QUAL.OMP.MAP.TOFROM"(%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i64 96, i64 32, i8 addrspace(4)* null, i8 addrspace(4)* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), double addrspace(4)* %fetch.1, i64 %mul.2, i64 562949953421843, i8 addrspace(4)* null, i8 addrspace(4)* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i64 addrspace(4)* getelementptr inbounds (%"QNCA_a0$double addrspace(4)*$rank2$", %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*), i32 0, i32 1), i64 88, i64 562949953421313, i8 addrspace(4)* null, i8 addrspace(4)* null) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD:F90_DV"(%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)* addrspacecast (%"QNCA_a0$double addrspace(4)*$rank2$" addrspace(1)* @"_unnamed_main$$_$FA2D" to %"QNCA_a0$double addrspace(4)*$rank2$" addrspace(4)*)), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %"ascastB$val") ]
  br label %call.post.list7
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 64773, i32 86377327, !"MAIN__", i32 3, i32 0, i32 0}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
