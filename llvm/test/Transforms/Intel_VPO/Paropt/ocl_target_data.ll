; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
; void foo(int *a) {
;   int i;
; #pragma omp target data map(a[0:100])
; #pragma omp target
;   for (i = 0; i < 100; ++i) {
;     a[i] = 0;
;   }
; }

; Make sure that no data mapping related structures are left in IR.
; In addition, check that there is only one Function in IR.
; CHECK: define weak dso_local
; CHECK-NOT: define{{.*}}dso_local
; CHECK-NOT: offload_baseptrs
; CHECK-NOT: offload_ptrs

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define protected spir_func void @foo(ptr addrspace(4) noundef %a) #0 {
entry:
  %a.addr = alloca ptr addrspace(4), align 8
  %i = alloca i32, align 4
  %a.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %a.addr.ascast = addrspacecast ptr %a.addr to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %a.map.ptr.tmp.ascast = addrspacecast ptr %a.map.ptr.tmp to ptr addrspace(4)
  store ptr addrspace(4) %a, ptr addrspace(4) %a.addr.ascast, align 8, !tbaa !8
  %0 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8, !tbaa !8
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %0, ptr addrspace(4) %arrayidx, i64 400, i64 3, ptr null, ptr null) ]
  %3 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8, !tbaa !8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %3, ptr addrspace(4) %3, i64 0, i64 544, ptr null, ptr null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1) ]
  store ptr addrspace(4) %3, ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  store i32 0, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %5 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  %cmp = icmp slt i32 %5, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8, !tbaa !8
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  %idxprom = sext i32 %7 to i64
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(4) %6, i64 %idxprom
  store i32 0, ptr addrspace(4) %arrayidx1, align 4, !tbaa !12
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  br label %for.cond, !llvm.loop !14

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1921967628, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}
!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.mustprogress"}
