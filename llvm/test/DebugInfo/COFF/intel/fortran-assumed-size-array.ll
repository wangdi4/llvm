; This test verifies that the CodeView backend emits the expected
; debug info for a Fortran assumed size array.

; The IR is based on the following Fortran subroutine:
;
; subroutine foo(A)
;   integer :: A(5,*)
;   print *, A(1,1)
; end subroutine foo

; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; CHECK:      DimConLU ([[DIMINFO:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMCONLU (0x1208)
; CHECK-NEXT:    IndexType: long (0x12)
; CHECK-NEXT:    Rank: 2
; CHECK-NEXT:    LowerBound: 1
; CHECK-NEXT:    UpperBound: 5
; CHECK-NEXT:    LowerBound: 1
; CHECK-NEXT:    UpperBound: 1
; CHECK:      DimArray ([[DIMARRAY:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMARRAY (0x1508)
; CHECK-NEXT:    ElementType: int (0x74)
; CHECK-NEXT:    DimInfo: [[DIMINFO]]
; CHECK:      Pointer ([[POINTER:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_POINTER (0x1002)
; CHECK-NEXT:    PointeeType: [[DIMARRAY]]
; CHECK:      LocalSym {
; CHECK-NEXT:    Kind: S_LOCAL (0x113E)
; CHECK-NEXT:    Type: & ([[POINTER]])
; CHECK-NEXT:    Flags [ (0x1)
; CHECK-NEXT:      IsParameter (0x1)
; CHECK-NEXT:    ]
; CHECK-NEXT:    VarName: A

; ModuleID = 'fortran-assumed-size-array.f90'
source_filename = "fortran-assumed-size-array.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; Function Attrs: noinline nounwind optnone uwtable
define void @FOO(i32* noalias dereferenceable(4) %"FOO$A$argptr") #0 !dbg !14 {
alloca_0:
  %"FOO$A$locptr" = alloca i32*, align 8
  call void @llvm.dbg.declare(metadata i32** %"FOO$A$locptr", metadata !18, metadata !DIExpression(DW_OP_deref)), !dbg !24
  ret void, !dbg !26
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.dbg.cu = !{!4}
!omp_offload.info = !{}

!0 = !{i32 7, !"PIC Level", i32 2}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !{i32 2, !"CodeView", i32 1}
!3 = !{i32 2, !"CodeViewGHash", i32 1}
!4 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !5, producer: "Intel(R) Fortran 22.0-1327", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!5 = !DIFile(filename: "fortran-assumed-size-array.f90", directory: "d:\\")
!14 = distinct !DISubprogram(name: "FOO", linkageName: "FOO", scope: !5, file: !5, line: 1, type: !15, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !4, retainedNodes: !17)
!15 = !DISubroutineType(types: !16)
!16 = !{null}
!17 = !{!18}
!18 = !DILocalVariable(name: "A", arg: 1, scope: !14, file: !5, line: 1, type: !19)
!19 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, elements: !21)
!20 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!21 = !{!22, !23}
!22 = !DISubrange(count: 5, lowerBound: 1)
!23 = !DISubrange(lowerBound: 1)
!24 = !DILocation(line: 1, scope: !14)
!26 = !DILocation(line: 5, scope: !14)
