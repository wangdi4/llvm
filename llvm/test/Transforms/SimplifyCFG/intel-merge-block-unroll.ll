; RUN: opt -passes="simplifycfg,intel-ir-optreport-emitter" -S %s 2>&1 | FileCheck %s
; CHECK: LOOP{{.*}}(4, 1)
; CHECK: remark{{.*}}loop was not vectorized
; CHECK: br i1 %cond2{{.*}}dbg{{.*}}llvm.loop

; SimpifyCFG rotates the loop (by merging the uncond backedge with the top
; test block), and loses all its metadata.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z14mkl_unrollcopyPvmPKvm(i1 %cond, i1 %cond2) !dbg !5 {
entry:
  br i1 %cond, label %while.cond, label %if.end

while.cond:                                       ; preds = %while.body, %entry
  br i1 %cond2, label %while.body, label %if.end

while.body:                                       ; preds = %while.cond
  %add.ptr188 = getelementptr inbounds i8, i8* undef, i64 256
  call void @llvm.dbg.value(metadata i8* %add.ptr188, metadata !9, metadata !DIExpression()), !dbg !13
  br label %while.cond, !dbg !14, !llvm.loop !15

if.end:                                           ; preds = %while.cond, %entry
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.debugify = !{!2, !3}
!llvm.module.flags = !{!4}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "debugify", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "intel-merge-block-unroll.ll", directory: "/")
!2 = !{i32 5}
!3 = !{i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = distinct !DISubprogram(name: "_Z14mkl_unrollcopyPvmPKvm", linkageName: "_Z14mkl_unrollcopyPvmPKvm", scope: null, file: !1, line: 1, type: !6, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !8)
!6 = !DISubroutineType(types: !7)
!7 = !{}
!8 = !{!9}
!9 = !DILocalVariable(name: "1", scope: !5, file: !1, line: 3, type: !10)
!10 = !DIBasicType(name: "ty64", size: 64, encoding: DW_ATE_unsigned)
!11 = !DILocation(line: 1, column: 1, scope: !5)
!12 = !DILocation(line: 2, column: 1, scope: !5)
!13 = !DILocation(line: 3, column: 1, scope: !5)
!14 = !DILocation(line: 4, column: 1, scope: !5)
!15 = distinct !{!15, !16, !17, !67}
!16 = !{!"llvm.loop.mustprogress"}
!17 = !{!"llvm.loop.unroll.count", i32 4}
!18 = !DILocation(line: 5, column: 1, scope: !5)

!67 = distinct !{!"intel.optreport.rootnode", !68}
!68 = distinct !{!"intel.optreport", !69, !70}
!69 = !{!"intel.optreport.debug_location", !14}
!70 = !{!"intel.optreport.remarks", !71}
!71 = !{!"intel.optreport.remark", i32 15436, !"loop was not vectorized: %s ", !""}

