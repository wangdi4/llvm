; INTEL_FEATURE_CSA
; RUN: opt -passes="require<vpo-wrninfo>,require<vpo-wrncollection>,require<domtree>,vpo-paropt" -opt-remark-emitter -pass-remarks=vpo-paropt-transform-csa -csa-omp-paropt-loop-splitting -disable-output %s 2>&1 | FileCheck %s
; REQUIRES: csa-registered-target
;
; This test check that CSA paropt lowering uses assumption cache for
; eliminating zero trip tests in workers.
;
; Using LLVM IR from the follwing code. For this code paropt is expected to
; create two workers and the first worker should not have ZTT.
;
; void foo(int begin, int end) {
;   __builtin_assume((end - begin) > 0);
; #pragma omp parallel for num_threads(2)
;   for (int i = begin; i < end; ++i) {}
; }

; CHECK:     remark:{{.*}}: CSA Paropt zero trip count check was not inserted for Worker #0, as directed by the __builtin_assume.
; CHECK-NOT: remark:{{.*}}: CSA Paropt zero trip count check was not inserted for Worker #1, as directed by the __builtin_assume.

target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %begin, i32 %end) local_unnamed_addr #0 !dbg !7 {
entry:
  %.omp.iv = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %sub = sub nsw i32 %end, %begin, !dbg !9
  %cmp = icmp sgt i32 %sub, 0, !dbg !9
  call void @llvm.assume(i1 %cmp), !dbg !9
  %0 = bitcast i32* %.omp.iv to i8*, !dbg !10
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1, !dbg !10
  %1 = bitcast i32* %.capture_expr. to i8*, !dbg !10
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1, !dbg !10
  store i32 %begin, i32* %.capture_expr., align 4, !dbg !11, !tbaa !12
  %cmp6 = icmp sgt i32 %end, %begin, !dbg !11
  br i1 %cmp6, label %omp.precond.then, label %omp.precond.end, !dbg !10

omp.precond.then:                                 ; preds = %entry
  %sub4 = add nsw i32 %sub, -1, !dbg !11
  %2 = bitcast i32* %.omp.lb to i8*, !dbg !10
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1, !dbg !10
  store i32 0, i32* %.omp.lb, align 4, !dbg !11, !tbaa !12
  %3 = bitcast i32* %.omp.ub to i8*, !dbg !10
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1, !dbg !10
  store volatile i32 %sub4, i32* %.omp.ub, align 4, !dbg !11, !tbaa !12
  br label %DIR.OMP.PARALLEL.LOOP.1, !dbg !10

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %omp.precond.then
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %.capture_expr.), "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg !10
  br label %DIR.OMP.PARALLEL.LOOP.117, !dbg !11

DIR.OMP.PARALLEL.LOOP.117:                        ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %5 = load i32, i32* %.omp.lb, align 4, !dbg !11, !tbaa !12
  store volatile i32 %5, i32* %.omp.iv, align 4, !dbg !11, !tbaa !12
  %6 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !12
  %7 = bitcast i32* %i to i8*
  %8 = load i32, i32* %.capture_expr., align 4
  br label %omp.inner.for.cond, !dbg !10

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.117
  %9 = load volatile i32, i32* %.omp.iv, align 4, !dbg !11, !tbaa !12
  %cmp7 = icmp sgt i32 %9, %6, !dbg !11
  br i1 %cmp7, label %omp.loop.exit, label %omp.inner.for.body, !dbg !10

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1, !dbg !10
  %10 = load volatile i32, i32* %.omp.iv, align 4, !dbg !11, !tbaa !12
  %add8 = add nsw i32 %8, %10, !dbg !11
  store i32 %add8, i32* %i, align 4, !dbg !11, !tbaa !12
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #1, !dbg !11
  %11 = load volatile i32, i32* %.omp.iv, align 4, !dbg !11, !tbaa !12
  %add9 = add nsw i32 %11, 1, !dbg !11
  store volatile i32 %add9, i32* %.omp.iv, align 4, !dbg !11, !tbaa !12
  br label %omp.inner.for.cond, !dbg !10, !llvm.loop !16

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !10
  br label %omp.precond.end, !dbg !10

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %12 = bitcast i32* %.omp.ub to i8*, !dbg !10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #1, !dbg !10
  %13 = bitcast i32* %.omp.lb to i8*, !dbg !10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #1, !dbg !10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #1, !dbg !10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #1, !dbg !10
  ret void, !dbg !17
}

; Function Attrs: nounwind
declare void @llvm.assume(i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.dbg.intel.emit_class_debug_always = !{!5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5e6a6384b6d3c8cb1e0741f7998b2f023e6e48f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a891db277c1c4c8ba3619b734ed98be990e0f27a)", isOptimized: true, runtimeVersion: 0, emissionKind: NoDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "t.c", directory: "/nfs/sc/proj/icl/devshare/sdmitrie/csa/assume")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{!"true"}
!6 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5e6a6384b6d3c8cb1e0741f7998b2f023e6e48f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a891db277c1c4c8ba3619b734ed98be990e0f27a)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 2, scope: !7)
!10 = !DILocation(line: 3, scope: !7)
!11 = !DILocation(line: 4, scope: !7)
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = distinct !{!16, !10, !10}
!17 = !DILocation(line: 5, scope: !7)
; end INTEL_FEATURE_CSA
