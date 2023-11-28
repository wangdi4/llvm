; Test case to verify that we maintain the loop location information
; for the optimization report when a remainder loop with TC=1 is
; unrolled.
;
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -disable-output -intel-opt-report=high -vplan-vec-scenario='n0;v4;m4' < %s 2>&1 | FileCheck %s
;
; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=high -vplan-vec-scenario='n0;v4;m4' < %s 2>&1 | FileCheck %s
;
; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=high -vplan-vec-scenario='n0;v4;m4' < %s 2>&1 | FileCheck %s
;
; C Source:
;
;     1 void foo(long *arr)
;     2 {
;     3   long l1;
;     4
;     5   for (l1 = 0; l1 < 1027; l1++)
;     6     arr[l1] = l1;
;     7 }
;
; CHECK: LOOP BEGIN at t.c (5, 3)
; CHECK: LOOP BEGIN at t.c (5, 3)
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: remark #15440: remainder loop was vectorized (masked)
;
define void @foo(ptr %arr) {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %l1.04 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %l1.04
  store i64 %l1.04, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.04, 1
  %exitcond.not = icmp eq i64 %inc, 1027
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.3, label %for.body, !llvm.loop !10

DIR.OMP.END.SIMD.3:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.end

for.end:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -O1 -S -emit-llvm -qopt-report=2 -o tt.ll t.c -fv\
eclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "t.c", directory: "/localdisk2/sguggill/svn_workspaces/vls_avx512/llvm/llvm/lib/Transforms/Vectorize/Intel_VPlan/VPlanHIR")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"uwtable", i32 2}
!5 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !6, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!6 = !DISubroutineType(types: !7)
!7 = !{}
!8 = !DILocation(line: 5, column: 3, scope: !5)
!9 = !DILocation(line: 6, column: 3, scope: !5)
!10 = distinct !{!10, !8, !9}
