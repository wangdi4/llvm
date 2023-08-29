; RUN: opt -passes="infer-address-spaces" -S -override-flat-addr-space=4 -o - %s | FileCheck %s
;
; Validate debug intrinsics are updated during address space inference.
;
; CHECK: call void @llvm.dbg.value(metadata ptr addrspace(1) %glob.ascast, metadata !9, metadata !DIExpression(DW_OP_deref)), !dbg !21
; CHECK: call void @llvm.dbg.declare(metadata ptr addrspace(1) @a.ascast.priv.__global, metadata !20, metadata !DIExpression()), !dbg !22
; CHECK: call void @llvm.dbg.value(metadata i64 3, metadata !18, metadata !DIExpression()), !dbg !23
; CHECK: call void @llvm.dbg.value(metadata ptr addrspace(1) @a.ascast.priv.__global, metadata !11, metadata !DIExpression()), !dbg !23

; ModuleID = 'minimal.ll'
source_filename = "minimal.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@a.ascast.priv.__global = internal addrspace(1) global i32 0, align 4

; Function Attrs: convergent nounwind
define void @function(ptr addrspace(1) %glob.ascast) #0 !dbg !5 {
newFuncRoot:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %glob.ascast, metadata !9, metadata !DIExpression(DW_OP_deref)), !dbg !21
  call void @llvm.dbg.declare(metadata ptr addrspace(1) @a.ascast.priv.__global, metadata !20, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.value(metadata i64 3, metadata !18, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.value(metadata ptr addrspace(4) addrspacecast (ptr addrspace(1) @a.ascast.priv.__global to ptr addrspace(4)), metadata !11, metadata !DIExpression()), !dbg !23
  store i32 0, ptr addrspace(1) @a.ascast.priv.__global, align 4, !dbg !24
  %0 = load i32, ptr addrspace(1) @a.ascast.priv.__global, align 4, !dbg !25
  %inc = add nsw i32 %0, 1, !dbg !25
  store i32 %inc, ptr addrspace(1) @a.ascast.priv.__global, align 4, !dbg !25
  store i32 0, ptr addrspace(4) addrspacecast (ptr addrspace(1) @a.ascast.priv.__global to ptr addrspace(4)), align 4, !dbg !26
  %1 = load i32, ptr addrspace(1) @a.ascast.priv.__global, align 4, !dbg !27
  %inc3 = add nsw i32 %1, 1, !dbg !27
  store i32 %inc3, ptr addrspace(1) @a.ascast.priv.__global, align 4, !dbg !27
  ret void
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { convergent nounwind }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 7, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "spirv", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4)
!3 = !DIFile(filename: "test.cpp", directory: "/path/to")
!4 = !{}
!5 = distinct !DISubprogram(name: "main.DIR.OMP.TARGET.2.split", scope: null, file: !3, line: 26, type: !6, scopeLine: 26, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagMainSubprogram, unit: !2, templateParams: !4, retainedNodes: !8)
!6 = !DISubroutineType(types: !7)
!7 = !{null}
!8 = !{!9, !11, !16, !18, !20}
!9 = !DILocalVariable(name: "glob", scope: !5, file: !3, line: 24, type: !10)
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DILocalVariable(name: "pa", scope: !12, file: !3, line: 31, type: !14)
!12 = distinct !DILexicalBlock(scope: !13, file: !3, line: 27, column: 2)
!13 = distinct !DILexicalBlock(scope: !5, file: !3, line: 26, column: 1)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocalVariable(name: "c", scope: !12, file: !3, line: 30, type: !17)
!17 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!18 = !DILocalVariable(name: "b", scope: !12, file: !3, line: 29, type: !19)
!19 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!20 = !DILocalVariable(name: "a", scope: !12, file: !3, line: 28, type: !15)
!21 = !DILocation(line: 24, column: 18, scope: !5)
!22 = !DILocation(line: 28, column: 13, scope: !12)
!23 = !DILocation(line: 0, scope: !12)
!24 = !DILocation(line: 32, column: 11, scope: !12)
!25 = !DILocation(line: 36, column: 14, scope: !12)
!26 = !DILocation(line: 38, column: 17, scope: !12)
!27 = !DILocation(line: 39, column: 14, scope: !12)
