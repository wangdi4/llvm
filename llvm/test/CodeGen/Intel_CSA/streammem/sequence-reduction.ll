; RUN: llc -mtriple=csa -csa-hw-reducer-experiment < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define double @copy(double* noalias %arr, i64 %N) #0 !dbg !7 {
entry:
  tail call void @llvm.dbg.value(metadata double* %arr, i64 0, metadata !15, metadata !25), !dbg !26
  tail call void @llvm.dbg.value(metadata i64 %N, i64 0, metadata !16, metadata !25), !dbg !27
  tail call void @llvm.dbg.value(metadata double 0.000000e+00, i64 0, metadata !17, metadata !25), !dbg !28
  tail call void @llvm.dbg.value(metadata i64 0, i64 0, metadata !18, metadata !25), !dbg !29
  tail call void @llvm.dbg.value(metadata i64 0, i64 0, metadata !18, metadata !25), !dbg !29
  tail call void @llvm.dbg.value(metadata double 0.000000e+00, i64 0, metadata !17, metadata !25), !dbg !28
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %sum.loop = phi double [ 0.0, %entry ], [ %sum, %loop ]
  tail call void @llvm.dbg.value(metadata double %sum.loop, i64 0, metadata !17, metadata !25), !dbg !28
  tail call void @llvm.dbg.value(metadata i64 %indvar, i64 0, metadata !18, metadata !25), !dbg !29
  %addr.arr = getelementptr inbounds double, double* %arr, i64 %indvar
  tail call void @llvm.dbg.value(metadata double* %addr.arr, i64 0, metadata !45, metadata !25), !dbg !37
  %val = load double, double* %addr.arr, align 8
  tail call void @llvm.dbg.value(metadata double %val, i64 0, metadata !19, metadata !25), !dbg !37
  %sum = fadd double %sum.loop, %val
  tail call void @llvm.dbg.value(metadata double %sum, i64 0, metadata !22, metadata !25), !dbg !39
  tail call void @llvm.dbg.value(metadata double %sum, i64 0, metadata !17, metadata !25), !dbg !28
  %indvar.next = add nuw i64 %indvar, 1
  tail call void @llvm.dbg.value(metadata i64 %indvar.next, i64 0, metadata !23, metadata !25), !dbg !41
  tail call void @llvm.dbg.value(metadata i64 %indvar.next, i64 0, metadata !18, metadata !25), !dbg !29
  tail call void @llvm.dbg.value(metadata i64 %indvar.next, i64 0, metadata !18, metadata !25), !dbg !29
  tail call void @llvm.dbg.value(metadata double %sum, i64 0, metadata !17, metadata !25), !dbg !28
  %exitcond = icmp slt i64 %indvar.next, %N
  br i1 %exitcond, label %loop, label %exit
; CHECK: .result .lic .i0 %[[OUTORD:[a-z0-9_.]+]]
; CHECK: .param .lic .i0 %[[INORD:[a-z0-9_.]+]]
; CHECK: .param .lic .i64 %[[SRC:[a-z0-9_.]+]]
; CHECK: .param .lic .i64 %[[LEN:[a-z0-9_.]+]]
; CHECK: maxs64 %[[SAFELEN:[a-z0-9_.]+]], %ign, 1, %[[LEN]]
; CHECK: sld64 %[[VAL:[a-z0-9_\.]+]], %[[SRC]], %[[SAFELEN]], 1, %[[OUTORD]], %[[INORD]], MEMLEVEL_T0
; CHECK: redaddf64 %[[REDUCE:[a-z0-9_\.]+]], 0, %[[VAL]],

exit:
  ret double %sum
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.0 (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-clang ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 9e41b55d7bf4215cea5bf57c262805a7985a3d8e)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "sequence-reduction.c", directory: "/nfs/site/home/dwoodwor/knp_work/testing")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 6.0.0 (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-clang ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 9e41b55d7bf4215cea5bf57c262805a7985a3d8e)"}
!7 = distinct !DISubprogram(name: "copy", scope: !1, file: !1, line: 2, type: !8, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !14)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !13}
!10 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!11 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !12)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!13 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!14 = !{!15, !16, !17, !18, !19, !22, !23}
!15 = !DILocalVariable(name: "arr", arg: 1, scope: !7, file: !1, line: 2, type: !11)
!16 = !DILocalVariable(name: "N", arg: 2, scope: !7, file: !1, line: 2, type: !13)
!17 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 3, type: !10)
!18 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 4, type: !13)
!19 = !DILocalVariable(name: "val", scope: !20, file: !1, line: 6, type: !21)
!20 = distinct !DILexicalBlock(scope: !7, file: !1, line: 5, column: 17)
!21 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !10)
!22 = !DILocalVariable(name: "new_sum", scope: !20, file: !1, line: 7, type: !21)
!23 = !DILocalVariable(name: "new_i", scope: !20, file: !1, line: 9, type: !24)
!24 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !13)
!25 = !DIExpression()
!26 = !DILocation(line: 2, column: 30, scope: !7)
!27 = !DILocation(line: 2, column: 45, scope: !7)
!28 = !DILocation(line: 3, column: 10, scope: !7)
!29 = !DILocation(line: 4, column: 13, scope: !7)
!30 = !DILocation(line: 5, column: 12, scope: !7)
!31 = !DILocation(line: 5, column: 3, scope: !7)
!32 = !DILocation(line: 6, column: 24, scope: !20)
!33 = !{!34, !34, i64 0}
!34 = !{!"double", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !DILocation(line: 6, column: 18, scope: !20)
!38 = !DILocation(line: 7, column: 32, scope: !20)
!39 = !DILocation(line: 7, column: 18, scope: !20)
!40 = !DILocation(line: 9, column: 31, scope: !20)
!41 = !DILocation(line: 9, column: 21, scope: !20)
!42 = distinct !{!42, !31, !43}
!43 = !DILocation(line: 11, column: 3, scope: !7)
!44 = !DILocation(line: 12, column: 3, scope: !7)
!45 = !DILocalVariable(name: "arraddr", scope: !20, file: !1, line: 6, type: !11)
