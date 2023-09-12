
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-recognize-par-loop,print<hir>" -S < %s 2>&1 | FileCheck %s

; The test checks if
;   '%1 = @llvm.directive.region.entry()' / '@llvm.directive.region.exit(%1)'
; pair of intrinsics originating from OpenMP's 'parallel for' is recognized and
; consumed by the hir-recognize-par-loop pass.

; Generated on Linux from the following source src.cpp below.
; --- Host compilation:
; clang -cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
;   -fintel-compatibility -fopenmp -fopenmp-targets=x86_64 -target-cpu x86-64 \
;   -O2 -fno-intel-openmp-offload -mllvm -paropt=31 -fopenmp-late-outline \
;   -o src.bc src.cpp
; INTEL_FEATURE_CSA
; --- Device compilation:
; clang -cc1 -triple csa -S -fintel-compatibility -fopenmp \
;   -fopenmp-targets=x86_64 -fopenmp-is-device \
;   -fopenmp-host-ir-file-path src.bc -O2 -fno-intel-openmp-offload \
;   -mllvm -paropt=31 -mllvm -disable-hir-vec-dir-insert \
;   -mllvm -disable-hir-general-unroll -fopenmp-late-outline \
;   -mllvm -hir-recognize-par-loop src.cpp
; end INTEL_FEATURE_CSA
;
; void loop1(int *ip, int n) {
;     #pragma omp target map(from : ip[n])
;     {
;
;       #pragma omp parallel for num_threads(3)
;       for (int i=0; i<n; i++) {
;           ip[i] = i;
;       }
;     }
; }

; ------- code before hir-recognize-par-loop
; CHECK: Function

; CHECK: BEGIN REGION { }
; CHECK:       %1 = @llvm.directive.region.entry(); [ DIR.OMP.PARALLEL.LOOP(), QUAL.OMP.NUM_THREADS(3), QUAL.OMP.FIRSTPRIVATE(&((%.omp.lb)[0])), QUAL.OMP.NORMALIZED.IV(null), QUAL.OMP.NORMALIZED.UB(null), QUAL.OMP.PRIVATE(&((%i)[0])), QUAL.OMP.SHARED(&((%ip.addr)[0])) ]
; CHECK:       %2 = (%.omp.lb)[0];
; CHECK:       if (%2 < %n)
; CHECK:       {
; CHECK:         %4 = (%ip.addr)[0];

; CHECK:         + DO i1 = 0, -1 * sext.i32.i64(%2) + smax(sext.i32.i64((-1 + trunc.i64.i32(%n))), sext.i32.i64(%2)), 1   <DO_LOOP>
; CHECK:         |   @llvm.lifetime.start.p0{{.*}}(4,  &({{.*}}(%i)[0]));
; CHECK:         |   (%4)[i1 + sext.i32.i64(%2)] = i1 + sext.i32.i64(%2);
; CHECK:         |   @llvm.lifetime.end.p0{{.*}}(4,  &({{.*}}(%i)[0]));
; CHECK:         + END LOOP
; CHECK:       }
; CHECK:      @llvm.directive.region.exit(%1); [ DIR.OMP.END.PARALLEL.LOOP() ]
; CHECK: END REGION

; ------- code after hir-recognize-par-loop
; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK:       %2 = (%.omp.lb)[0];
; CHECK:       if (%2 < %n)
; CHECK:       {
; CHECK:         %4 = (%ip.addr)[0];

; CHECK:         + DO i1 = 0, -1 * sext.i32.i64(%2) + smax(sext.i32.i64((-1 + trunc.i64.i32(%n))), sext.i32.i64(%2)), 1   <DO_LOOP> <parallel>
; CHECK:         |   @llvm.lifetime.start.p0{{.*}}(4,  &({{.*}}(%i)[0]));
; CHECK:         |   (%4)[i1 + sext.i32.i64(%2)] = i1 + sext.i32.i64(%2);
; CHECK:         |   @llvm.lifetime.end.p0{{.*}}(4,  &({{.*}}(%i)[0]));
; CHECK:         + END LOOP
; CHECK:       }
; CHECK: END REGION


; Function Attrs: norecurse nounwind
define weak void @__omp_offloading_546836e2_9c9fe__Z5loop1Pii_l20(i64 %n, ptr %ip) #0 {
entry:
  %ip.addr = alloca ptr, align 8
  %.omp.lb = alloca i32, align 4
  %i = alloca i32, align 4
  %n.addr.sroa.0.0.extract.trunc = trunc i64 %n to i32
  store ptr %ip, ptr %ip.addr, align 8, !tbaa !3
  %cmp = icmp sgt i32 %n.addr.sroa.0.0.extract.trunc, 0
  %0 = bitcast ptr %.omp.lb to ptr
  br i1 %cmp, label %DIR.OMP.PARALLEL.LOOP.112, label %omp.precond.end

DIR.OMP.PARALLEL.LOOP.112:                        ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %0) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !7
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(ptr %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.PRIVATE"(ptr %i), "QUAL.OMP.SHARED"(ptr %ip.addr) ]
  %2 = load i32, ptr %.omp.lb, align 4, !tbaa !7, !alias.scope !909, !noalias !911
  %cmp413 = icmp slt i32 %2, %n.addr.sroa.0.0.extract.trunc
  br i1 %cmp413, label %omp.inner.for.body.preheader, label %omp.loop.exit

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.PARALLEL.LOOP.112
  %3 = bitcast ptr %i to ptr
  %4 = load ptr, ptr %ip.addr, align 8, !tbaa !3, !alias.scope !912, !noalias !911
  %5 = sext i32 %2 to i64
  %sub2 = shl i64 %n, 32
  %sext = add i64 %sub2, -4294967296
  %6 = ashr exact i64 %sext, 32
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.preheader, %omp.inner.for.body
  %indvars.iv = phi i64 [ %5, %omp.inner.for.body.preheader ], [ %indvars.iv.next, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %3) #2
  %7 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds i32, ptr %4, i64 %indvars.iv
  store i32 %7, ptr %arrayidx, align 4, !tbaa !7, !alias.scope !13
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %3) #2
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp4 = icmp slt i64 %indvars.iv, %6
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit.loopexit

omp.loop.exit.loopexit:                           ; preds = %omp.inner.for.body
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.loop.exit.loopexit, %DIR.OMP.PARALLEL.LOOP.112
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry, %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!0 = !{i32 0, i32 1416115938, i32 641534, !"_Z5loop1Pii", i32 20, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPi", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
!909 = !{!9}
!9 = distinct !{!9, !10, !"OMPAliasScope"}
!10 = distinct !{!10, !"OMPDomain"}
!911 = !{!11}
!11 = distinct !{!11, !10, !"OMPAliasScope"}
!912 = !{!12}
!12 = distinct !{!12, !10, !"OMPAliasScope"}
!13 = !{!12, !11}
