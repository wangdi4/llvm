; Test to check that DbgLoc metadata captured in incoming Loop's LoopID
; are preserved appropriately in generated vector code.

; RUN: opt < %s -S -passes=vplan-vec -vplan-force-vf=2 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: define dso_local void @foo(ptr nocapture %A)
; CHECK:         vector.body:
; CHECK:           br i1 [[EXIT_COND:%.*]], label [[EXIT_BB:%.*]], label %vector.body, !llvm.loop [[VEC_LOOP_ID:!.*]]
; CHECK:       [[VEC_LOOP_ID]] = distinct !{[[VEC_LOOP_ID]], [[LOCRANGE_START:!.*]], [[LOCRANGE_END:!.*]], [[IS_VEC_MD:!.*]]}
; CHECK:       [[LOCRANGE_START]] = !DILocation(line: 2, column: 1, scope: [[SCOPEMD:!.*]])
; CHECK:       [[LOCRANGE_END]] = !DILocation(line: 2, column: 17, scope: [[SCOPEMD:!.*]])
; CHECK:       [[IS_VEC_MD]] = !{!"llvm.loop.isvectorized", i32 1}

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @foo(ptr nocapture %A) local_unnamed_addr {
DIR.OMP.SIMD.113:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.113
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %iv.trunc = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %iv.trunc, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body, !llvm.loop !3

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "debugify", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test.c", directory: "")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !{!3, !4, !9, !10}
!4 = !DILocation(line: 2, column: 1, scope: !5)
!5 = distinct !DILexicalBlock(scope: !6, file: !1, line: 2, column: 1)
!6 = distinct !DISubprogram(name: "foo", linkageName: "foo", scope: null, file: !1, line: 1, type: !7, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!7 = !DISubroutineType(types: !8)
!8 = !{}
!9 = !DILocation(line: 2, column: 17, scope: !5)
!10 = !{!"llvm.loop.vectorize.enable", i1 true}
