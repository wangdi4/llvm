; RUN: llc %s -o %t -filetype=obj
; RUN: llvm-readobj --codeview %t | FileCheck %s 
; RUN: llc  -disable-intel-codeview-oem-extensions %s -o %t -filetype=obj
; RUN: llvm-readobj --codeview %t | FileCheck %s --check-prefix INTEL
; Test enabling/disabling of CodeView LF_OEM record emission.
;
; This test is handwritten IR based on this Fortran code:
;    -------------------------------------------------------
; test.f90:
;        character(:), allocatable :: ch(:)
;
;        call subr(ch)
;
;        contains
;
;        subroutine subr(ch)
;        character(:), allocatable :: ch(:)
;
;        allocate(character*2::ch(1))
;        end subroutine
;
;        end
;
; CHECK: OEMType {{.*}}
; CHECK: TypeLeafKind: LF_OEM (0x100F)
; CHECK: OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK: OEMType: LF_recOEM_MSF90_DESCRIPTOR (0x5)
; CHECK: OEMType {{.*}}
; CHECK: TypeLeafKind: LF_OEM (0x100F)
; CHECK: OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK: OEMType: LF_recOEM_MSF90_DESCR_ARR (0x0)
; CHECK: DisplayName: CH
; INTEL-NOT: OEMType
; INTEL-NOT: LF_OEM
; INTEL-NOT: LF_OEM_IDENT_MSF90
; INTEL-NOT: LF_recOEM_MSF90_DESCRIPTOR
; INTEL-NOT: LF_recOEM_MSF90_DESCR_ARR
; INTEL: DisplayName: CH

target triple = "x86_64-pc-windows-msvc"

%"QNCA_a0$i8*$rank1$.1" = type { i8*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"_UNNAMED_MAIN$$$CH" = internal global %"QNCA_a0$i8*$rank1$.1" { i8* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }, !dbg !0

define void @MAIN__() #0 !dbg !2 {
alloca_0:
  ret void, !dbg !17
}

define void @"_UNNAMED_MAIN$$_ip_SUBR"() #0 !dbg !18 {
alloca_1:
  ret void, !dbg !25
}

!llvm.module.flags = !{!13, !14}
!llvm.dbg.cu = !{!6}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "CH", linkageName: "_UNNAMED_MAIN$$$CH", scope: !2, file: !3, line: 1, type: !9, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "_UNNAMED_MAIN$$", linkageName: "MAIN__", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6, retainedNodes: !8)
!3 = !DIFile(filename: "test.f90", directory: "c:\\path\\to")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Fortran", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0}
!8 = !{}
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !11, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!10 = !DIStringType(name: "CHARACTER_0")
!11 = !{!12}
!12 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 48, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 56, DW_OP_deref))
!13 = !{i32 2, !"Debug Info Version", i32 3}
!14 = !{i32 2, !"CodeView", i32 1}
!15 = !DILocation(line: 1, scope: !2)
!16 = !DILocation(line: 3, scope: !2)
!17 = !DILocation(line: 5, scope: !2)
!18 = distinct !DISubprogram(name: "SUBR", linkageName: "_UNNAMED_MAIN$$_ip_SUBR", scope: !2, file: !3, line: 7, type: !4, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !19)
!19 = !{!20}
!20 = !DILocalVariable(name: "CH", arg: 1, scope: !18, file: !3, line: 7, type: !21)
!21 = !DICompositeType(tag: DW_TAG_array_type, baseType: !22, elements: !11, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!22 = !DIStringType(name: ".str.CH", stringLengthExpression: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 8))
!23 = !DILocation(line: 7, scope: !18)
!24 = !DILocation(line: 10, scope: !18)
!25 = !DILocation(line: 11, scope: !18)
