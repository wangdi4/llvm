; RUN: llc -O2 < %s -pass-remarks-missed=csa-streammem 2>&1 | FileCheck %s
; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @foo(i32* nocapture %a, i32* nocapture readonly %b, i32 %N) local_unnamed_addr #0 !dbg !7 {
entry:
  tail call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !13, metadata !18), !dbg !19
  tail call void @llvm.dbg.value(metadata i32* %b, i64 0, metadata !14, metadata !18), !dbg !20
  tail call void @llvm.dbg.value(metadata i32 %N, i64 0, metadata !15, metadata !18), !dbg !21
  tail call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !16, metadata !18), !dbg !22
  tail call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !16, metadata !18), !dbg !22
  %cmp5 = icmp sgt i32 %N, 0, !dbg !23
  br i1 %cmp5, label %for.body.lr.ph, label %for.end, !dbg !25

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body, !dbg !25

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  tail call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !16, metadata !18), !dbg !22
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv, !dbg !26
  %0 = load i32, i32* %arrayidx, align 4, !dbg !26, !tbaa !27
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv, !dbg !31
  store i32 %0, i32* %arrayidx2, align 4, !dbg !32, !tbaa !27
; CHECK: remark: foo.c:3:12: streaming memory conversion failed:
; CHECK-SAME: memory ordering tokens are not loop-invariant
; CHECK: remark: foo.c:3:10: streaming memory conversion failed:
; CHECK-SAME: memory ordering tokens are not loop-invariant
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !33
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !23
  br i1 %exitcond, label %for.end, label %for.body, !dbg !25, !llvm.loop !34

for.end:                                          ; preds = %for.body, %entry
  %sub = add nsw i32 %N, -1, !dbg !36
  %idxprom3 = sext i32 %sub to i64, !dbg !37
  %arrayidx4 = getelementptr inbounds i32, i32* %a, i64 %idxprom3, !dbg !37
  %1 = load i32, i32* %arrayidx4, align 4, !dbg !37, !tbaa !27
  ret i32 %1, !dbg !38
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 12d25386d7a7e9be21398be64ab018da3fdb39ea)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "foo.c", directory: "/nfs/site/home/jcranmer/tmp")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 12d25386d7a7e9be21398be64ab018da3fdb39ea)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !12)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!12 = !{!13, !14, !15, !16}
!13 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 1, type: !11)
!14 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "N", arg: 3, scope: !7, file: !1, line: 1, type: !10)
!16 = !DILocalVariable(name: "i", scope: !17, file: !1, line: 2, type: !10)
!17 = distinct !DILexicalBlock(scope: !7, file: !1, line: 2, column: 3)
!18 = !DIExpression()
!19 = !DILocation(line: 1, column: 15, scope: !7)
!20 = !DILocation(line: 1, column: 24, scope: !7)
!21 = !DILocation(line: 1, column: 31, scope: !7)
!22 = !DILocation(line: 2, column: 12, scope: !17)
!23 = !DILocation(line: 2, column: 21, scope: !24)
!24 = distinct !DILexicalBlock(scope: !17, file: !1, line: 2, column: 3)
!25 = !DILocation(line: 2, column: 3, scope: !17)
!26 = !DILocation(line: 3, column: 12, scope: !24)
!27 = !{!28, !28, i64 0}
!28 = !{!"int", !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 3, column: 5, scope: !24)
!32 = !DILocation(line: 3, column: 10, scope: !24)
!33 = !DILocation(line: 2, column: 27, scope: !24)
!34 = distinct !{!34, !25, !35}
!35 = !DILocation(line: 3, column: 15, scope: !17)
!36 = !DILocation(line: 4, column: 14, scope: !7)
!37 = !DILocation(line: 4, column: 10, scope: !7)
!38 = !DILocation(line: 4, column: 3, scope: !7)
