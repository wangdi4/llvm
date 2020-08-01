; Check that line number of store is preserved after memcpy transform.

; RUN: opt -scoped-noalias-aa -hir-ssa-deconstruction -disable-output -hir-temp-cleanup -hir-runtime-dd -hir-idiom -print-after=hir-idiom < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa,scoped-noalias-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-idiom,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; CHECK:       BEGIN REGION { modified }
; CHECK: :3>      @llvm.memcpy.p0i8.p0i8.i32(&((i8*)(%a)[0]),  &((i8*)(%a)[100]),  400,  0);
; CHECK:       END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %a) !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, metadata !13, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32 0, metadata !15, metadata !DIExpression()), !dbg !18
  br label %for.body, !dbg !19

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.01, metadata !15, metadata !DIExpression()), !dbg !18
  %add = add nsw i32 %i.01, 100, !dbg !20
  %idxprom = sext i32 %add to i64, !dbg !22
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %idxprom, !dbg !22
  %0 = load i32, i32* %arrayidx, align 4, !dbg !22
  %idxprom1 = sext i32 %i.01 to i64, !dbg !23
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %idxprom1, !dbg !23
  store i32 %0, i32* %arrayidx2, align 4, !dbg !24
  br label %for.inc, !dbg !23

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1, !dbg !25
  call void @llvm.dbg.value(metadata i32 %inc, metadata !15, metadata !DIExpression()), !dbg !18
  %cmp = icmp slt i32 %inc, 100, !dbg !26
  br i1 %cmp, label %for.body, label %for.end, !dbg !19, !llvm.loop !27

for.end:                                          ; preds = %for.inc
  ret void, !dbg !29
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

attributes #0 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) dev.8.x.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "memcpy-dbg.c", directory: "/export/iusers/pgprokof/xmain-ws5/llvm/test/Transforms/Intel_LoopTransforms/HIRIdiomRecognition")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"icx (ICX) dev.8.x.0"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!13 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!14 = !DILocation(line: 0, scope: !8)
!15 = !DILocalVariable(name: "i", scope: !16, file: !1, line: 2, type: !17)
!16 = distinct !DILexicalBlock(scope: !8, file: !1, line: 2, column: 3)
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !DILocation(line: 0, scope: !16)
!19 = !DILocation(line: 2, column: 3, scope: !16)
!20 = !DILocation(line: 3, column: 15, scope: !21)
!21 = distinct !DILexicalBlock(scope: !16, file: !1, line: 2, column: 3)
!22 = !DILocation(line: 3, column: 12, scope: !21)
!23 = !DILocation(line: 3, column: 5, scope: !21)
!24 = !DILocation(line: 3, column: 10, scope: !21)
!25 = !DILocation(line: 2, column: 22, scope: !21)
!26 = !DILocation(line: 2, column: 17, scope: !21)
!27 = distinct !{!27, !19, !28}
!28 = !DILocation(line: 3, column: 19, scope: !16)
!29 = !DILocation(line: 4, column: 1, scope: !8)
