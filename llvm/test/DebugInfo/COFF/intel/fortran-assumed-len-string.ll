; This test verifies that the CodeView backend emits the expected
; debug info for a Fortran assumed length string.

; The IR is based on the following Fortran program:
;
; PROGRAM test2
;     IMPLICIT NONE
;     CHARACTER*10, ALLOCATABLE :: a
;     ALLOCATE( a )
;     a = 'abcdefghij'
;     call sub2( a )
; CONTAINS
;     SUBROUTINE sub2( b )
;         IMPLICIT NONE
;         CHARACTER( * ), ALLOCATABLE :: b
;         b = 'klmnopqrst'
;         RETURN
;     END SUBROUTINE sub2
; END PROGRAM test2

; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; CHECK:      RefSym ([[REFSYMREC:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_REFSYM (0x20C)
; CHECK-NEXT:    Decoded Sym {
; CHECK-NEXT:      Length: 17
; CHECK-NEXT:      Kind: S_CONSTANT (0x1107)
; CHECK-NEXT:      Type: __int64 (0x13)
; CHECK-NEXT:      Value: 1
; CHECK-NEXT:      Name: .T1_.len
; CHECK:      DimVarU ([[DIMVARUREC:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMVARU (0x1209)
; CHECK-NEXT:    IndexType: unsigned __int64 (0x23)
; CHECK-NEXT:    Rank: 1
; CHECK-NEXT:    UpperBound: [[REFSYMREC]]
; CHECK:      DimArray ([[DIMARRAYREC:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMARRAY (0x1508)
; CHECK-NEXT:    ElementType: char (0x70)
; CHECK-NEXT:    DimInfo: [[DIMVARUREC]]
; CHECK-NEXT:    Name: CHARACTER_1
; CHECK:      Pointer ([[PTR1REC:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_POINTER (0x1002)
; CHECK-NEXT:    PointeeType: [[DIMARRAYREC]]
; CHECK:      Pointer ([[PTR2REC:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_POINTER (0x1002)
; CHECK-NEXT:    PointeeType: * ([[PTR1REC]])
; CHECK:      LocalSym {
; CHECK-NEXT:    Kind: S_LOCAL (0x113E)
; CHECK-NEXT:    Type: *& ([[PTR2REC]])
; CHECK-NEXT:    Flags [ (0x1)
; CHECK-NEXT:      IsParameter (0x1)
; CHECK-NEXT:    ]
; CHECK-NEXT:    VarName: B
; CHECK:      LocalSym {
; CHECK-NEXT:    Kind: S_LOCAL (0x113E)
; CHECK-NEXT:    Type: __int64 (0x13)
; CHECK-NEXT:    Flags [ (0x0)
; CHECK-NEXT:    ]
; CHECK-NEXT:    VarName: .T1_.len

; ModuleID = 'char.bc'
source_filename = "char.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@"TEST2$A" = internal global ptr null, align 8, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 !dbg !2 {
alloca_0:
  call void @TEST2_ip_SUB2(ptr @"TEST2$A", i64 10), !dbg !14, !llfort.type_idx !13
  ret void, !dbg !15
}

; Function Attrs: noinline nounwind optnone uwtable
define void @TEST2_ip_SUB2(ptr noalias %"B$argptr", i64 %".T1_.len$val") #0 !dbg !16 {
alloca_1:
  %"B$locptr" = alloca ptr, align 8
  %.T1_.len = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %.T1_.len, metadata !18, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.declare(metadata ptr %"B$locptr", metadata !20, metadata !DIExpression(DW_OP_deref)), !dbg !24
  ret void, !dbg !25
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!10, !11, !12}
!llvm.dbg.cu = !{!6}
!omp_offload.info = !{}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "A", linkageName: "TEST2$A", scope: !2, file: !3, line: 3, type: !8, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "TEST2", linkageName: "MAIN__", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6)
!3 = !DIFile(filename: "char.f90", directory: "d:\\tmp")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Fortran", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0}
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!9 = !DIStringType(name: "CHARACTER_0", size: 80)
!10 = !{i32 8, !"PIC Level", i32 2}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 2, !"CodeView", i32 1}
!13 = !{i64 18}
!14 = !DILocation(line: 6, scope: !2)
!15 = !DILocation(line: 7, scope: !2)
!16 = distinct !DISubprogram(name: "SUB2", linkageName: "TEST2_ip_SUB2", scope: !2, file: !3, line: 8, type: !4, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !17)
!17 = !{!18, !20}
!18 = !DILocalVariable(name: ".T1_.len", scope: !16, type: !19, flags: DIFlagArtificial)
!19 = !DIBasicType(name: "INTEGER*8", size: 64, encoding: DW_ATE_signed)
!20 = !DILocalVariable(name: "B", arg: 1, scope: !16, file: !3, line: 8, type: !21)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!22 = !DIStringType(name: "CHARACTER_1", stringLength: !18)
!24 = !DILocation(line: 8, scope: !16)
!25 = !DILocation(line: 13, scope: !16)
