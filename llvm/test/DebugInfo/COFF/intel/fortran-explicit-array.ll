; By default, llc uses DIMARRAY and DIMCONLU to encode the debug info
; of a Fortran explicit array.
; When -disable-intel-codeview-oem-extensions is specified, llc uses
; DIMARRAY instead, reason being DIMARRAY and DIMCONLU are not recognized
; by recent versions of VS debugger.
;
; This test verifies the following:
;   1. The expected debug info for an explicit Fortran array is emitted
;      according to the compiler OEM option.
;   2. Shared dimension info is cached and emitted properly.
;   3. The tools llvm-readobj and llvm-pdbutil dump the debug info properly.

; RUN: llc < %s -filetype=obj > %t.obj
; RUN: llvm-readobj --codeview %t.obj | FileCheck %s
; RUN: llvm-pdbutil dump -types -symbols %t.obj | FileCheck %s --check-prefix PDBUTIL
; RUN: llc < %s -disable-intel-codeview-oem-extensions -filetype=obj > %t.obj
; RUN: llvm-pdbutil dump -types -symbols %t.obj | FileCheck %s --check-prefix NOOEM
;
; CHECK:      DimConLU ([[DIMINFO:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMCONLU (0x1208)
; CHECK-NEXT:    IndexType: long (0x12)
; CHECK-NEXT:    Rank: 3
; CHECK-NEXT:    LowerBound: -1
; CHECK-NEXT:    UpperBound: 5
; CHECK-NEXT:    LowerBound: 2
; CHECK-NEXT:    UpperBound: 10
; CHECK-NEXT:    LowerBound: 1
; CHECK-NEXT:    UpperBound: 9
; CHECK:      DimArray ([[DIMARRAY2:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMARRAY (0x1508)
; CHECK-NEXT:    ElementType: float (0x40)
; CHECK-NEXT:    DimInfo: [[DIMINFO]]
; CHECK:      DimArray ([[DIMARRAY:0x[A-F0-9]+]]) {
; CHECK-NEXT:    TypeLeafKind: LF_DIMARRAY (0x1508)
; CHECK-NEXT:    ElementType: int (0x74)
; CHECK-NEXT:    DimInfo: [[DIMINFO]]
; CHECK:        Type: [[DIMARRAY2]]
; CHECK-NEXT:   DisplayName: ARR2
; CHECK:        Type: [[DIMARRAY]]
; CHECK-NEXT:   DisplayName: ARR

; PDBUTIL:       [[DIMINFO:0x[A-F0-9]+]] | LF_DIMCONLU [size = 36]
; PDBUTIL-NEXT:      index type: 0x0012 (long), rank: 3
; PDBUTIL-NEXT:      Rank 1 bounds: (-1, 5)
; PDBUTIL-NEXT:      Rank 2 bounds: (2, 10)
; PDBUTIL-NEXT:      Rank 3 bounds: (1, 9)
; PDBUTIL:       [[DIMARRAY2:0x[A-F0-9]+]] | LF_DIMARRAY [size = 16]
; PDBUTIL-NEXT:      dim info: [[DIMINFO]], element type: 0x0040 (float)
; PDBUTIL:       [[DIMARRAY:0x[A-F0-9]+]] | LF_DIMARRAY [size = 16]
; PDBUTIL-NEXT:      dim info: [[DIMINFO]], element type: 0x0074 (int)
; PDBUTIL:       S_LDATA32 [size = 20] `ARR2`
; PDBUTIL-NEXT:      type = [[DIMARRAY2]]
; PDBUTIL:       S_LDATA32 [size = 20] `ARR`
; PDBUTIL-NEXT:      type = [[DIMARRAY]]

; NOOEM:      [[DIM23:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 36, index type: 0x0023 (unsigned __int64), element type: 0x0040 (float)
; NOOEM:      [[DIM22:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 324, index type: 0x0023 (unsigned __int64), element type: [[DIM23]]
; NOOEM:      [[DIM21:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 2268, index type: 0x0023 (unsigned __int64), element type: [[DIM22]]
; NOOEM:      [[DIM3:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 36, index type: 0x0023 (unsigned __int64), element type: 0x0074 (int)
; NOOEM:      [[DIM2:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 324, index type: 0x0023 (unsigned __int64), element type: [[DIM3]]
; NOOEM:      [[DIM1:0x[A-F0-9]+]] | LF_ARRAY [size = 16]
; NOOEM-NEXT:     size: 2268, index type: 0x0023 (unsigned __int64), element type: [[DIM2]]
; NOOEM:      S_LDATA32 [size = 20] `ARR2`
; NOOEM-NEXT:     type = [[DIM21]]
; NOOEM:      S_LDATA32 [size = 20] `ARR`
; NOOEM-NEXT:     type = [[DIM1]]

; ModuleID = 'test.f90'
source_filename = "test.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@"TEST$ARR2" = internal global [9 x [9 x [7 x float]]] zeroinitializer, align 16, !dbg !0
@"TEST$ARR" = internal global [9 x [9 x [7 x i32]]] zeroinitializer, align 16, !dbg !8
@0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 !dbg !2 {
  ret void, !dbg !34
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }

!llvm.module.flags = !{!19, !20, !21, !22}
!llvm.dbg.cu = !{!6}
!omp_offload.info = !{}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ARR2", linkageName: "TEST$ARR2", scope: !2, file: !3, line: 3, type: !17, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "TEST", linkageName: "MAIN__", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6, retainedNodes: !16)
!3 = !DIFile(filename: "test.f90", directory: "d:\\")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 22.0-1279", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0, !8}
!8 = !DIGlobalVariableExpression(var: !9, expr: !DIExpression())
!9 = distinct !DIGlobalVariable(name: "ARR", linkageName: "TEST$ARR", scope: !2, file: !3, line: 2, type: !10, isLocal: true, isDefinition: true)
!10 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, elements: !12)
!11 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!12 = !{!13, !14, !15}
!13 = !DISubrange(lowerBound: -1, upperBound: 5)
!14 = !DISubrange(lowerBound: 2, upperBound: 10)
!15 = !DISubrange(count: 9, lowerBound: 1)
!16 = !{}
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, elements: !12)
!18 = !DIBasicType(name: "REAL*4", size: 32, encoding: DW_ATE_float)
!19 = !{i32 7, !"PIC Level", i32 2}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 2, !"CodeView", i32 1}
!22 = !{i32 2, !"CodeViewGHash", i32 1}
!34 = !DILocation(line: 7, scope: !2)
