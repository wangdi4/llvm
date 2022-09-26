; RUN: opt -intel-opt-report=high -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-optreport-emitter" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that opt-report remarks are constructed correctly for DDEdges
; involving functions referenced by llvm.dbg.value intrinsics. It is based on a
; reduced version of SPEC CPU 2017 502.gcc which hit an assertion fail.

; CHECK: remark #15346: vector dependence: assumed ANTI dependence between x and f

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

define internal fastcc void @decompose_multiword_subregs(ptr %0) {
  br label %2

2:                                                ; preds = %8, %1
  call void @llvm.dbg.value(metadata ptr @find_decomposable_subregs, metadata !1, metadata !DIExpression()), !dbg !6
  call void @llvm.dbg.value(metadata ptr %0, metadata !7, metadata !DIExpression()), !dbg !6
  br label %3

3:                                                ; preds = %2
  %4 = call fastcc i32 undef(ptr null, i32 0, ptr @find_decomposable_subregs, ptr null)
  br label %5

5:                                                ; preds = %3
  %6 = load ptr, ptr %0, align 8
  %7 = load i32, ptr %6, align 8
  br label %8

8:                                                ; preds = %5
  %9 = icmp eq i64 0, 0
  br i1 %9, label %10, label %2

10:                                               ; preds = %8
  ret void
}

declare i32 @find_decomposable_subregs(ptr, ptr)

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !DILocalVariable(name: "f", scope: !2)
!2 = distinct !DILexicalBlock(scope: !3, line: 1099)
!3 = distinct !DISubprogram(file: !4, unit: !5)
!4 = !DIFile(filename: "lower-subreg.c", directory: "build_base_core_avx512.0000")
!5 = distinct !DICompileUnit(language: DW_LANG_C99, file: !4)
!6 = !DILocation(scope: !2)
!7 = !DILocalVariable(name: "x", scope: !2)
