; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; Validate correct emission of the Intel OEM record LF_recOEM_MSF90_DESCR_ARR.
;
; This test is handwritten IR based on this Fortran code:
;    -------------------------------------------------------
;     1      program f_bounds
;     2      character(len=5) item(1:1)
;     3      item(1) = "hello"
;     4      call sub1(item)
;     5
;     6      contains
;     7
;     8      subroutine sub1(param1)
;     9          character(len=*) :: param1(:)
;    10          do i=lbound(param1,1),ubound(param1,1)
;    11              print *, "-----------", param1(i)
;    12          end do
;    13      end subroutine
;    14
;    15      end program
;    -------------------------------------------------------
;
; CHECK:      CodeViewTypes [
; CHECK:        Section: .debug$T (6)
;
; CHECK:        Array ([[STRING:0x[A-Z0-9]+]]) {
; CHECK-NEXT:     TypeLeafKind: LF_ARRAY (0x1503)
; CHECK-NEXT:     ElementType: char (0x70)
; CHECK-NEXT:     IndexType: unsigned __int64 (0x23)
; CHECK-NEXT:     SizeOf: 0
; CHECK-NEXT:     Name: .str.PARAM1
; CHECK-NEXT:   }
;
; CHECK:        OEMType ([[DESC:0x[A-Z0-9]+]]) {
; CHECK-NEXT:     TypeLeafKind: LF_OEM (0x100F)
; CHECK-NEXT:     OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK-NEXT:     OEMType: LF_recOEM_MSF90_DESCR_ARR (0x0)
; CHECK-NEXT:     Count: 2
; CHECK-NEXT:     ElementType: .str.PARAM1 ([[STRING]])
; CHECK-NEXT:     BoundsKey: 0x0
; CHECK-NEXT:     ArrayRank: 1
; CHECK-NEXT:     DescrSize: 72
; CHECK-NEXT:   }
;
; CHECK:        Pointer ([[PTR2DESC:0x[A-Z0-9]+]]) {
; CHECK-NEXT:     TypeLeafKind: LF_POINTER (0x1002)
; CHECK-NEXT:     PointeeType: [[DESC]]
; CHECK-NEXT:     PtrType: Near64 (0xC)
; CHECK-NEXT:     PtrMode: LValueReference (0x1)
; CHECK-NEXT:     IsFlat: 0
; CHECK-NEXT:     IsConst: 0
; CHECK-NEXT:     IsVolatile: 0
; CHECK-NEXT:     IsUnaligned: 0
; CHECK-NEXT:     IsRestrict: 0
; CHECK-NEXT:     IsThisPtr&: 0
; CHECK-NEXT:     IsThisPtr&&: 0
; CHECK-NEXT:     SizeOf: 0
; CHECK-NEXT:   }
;
; CHECK:      ]
;
; CHECK:      CodeViewDebugInfo [
; CHECK-NEXT:   Section: .debug$S (5)
; CHECK:        Subsection [
; CHECK-NEXT:     SubSectionType: Symbols (0xF1)
;
; CHECK:          GlobalProcIdSym {
; CHECK:            Kind: S_GPROC32_ID (0x1147)
; CHECK:            DisplayName: SUB1
; CHECK:            LinkageName: F_BOUNDS_ip_SUB1
; CHECK:          }
;
; CHECK:          LocalSym {
; CHECK-NEXT:       Kind: S_LOCAL (0x113E)
; CHECK-NEXT:       Type: & ([[PTR2DESC]])
; CHECK-NEXT:       Flags [ (0x1)
; CHECK-NEXT:         IsParameter (0x1)
; CHECK-NEXT:       ]
; CHECK-NEXT:       VarName: PARAM1
; CHECK-NEXT:     }
;
; CHECK:          ProcEnd {
; CHECK-NEXT:       Kind: S_PROC_ID_END (0x114F)
; CHECK-NEXT:     }
;
; CHECK:        ]
; CHECK:      ]
;
; -----------------------------------------------------------------------------
; ModuleID = 'test.f90'
source_filename = "test.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%"QNCA_a0$i8*$rank1$" = type { i8*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind optnone uwtable
define void @F_BOUNDS_ip_SUB1(%"QNCA_a0$i8*$rank1$"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"PARAM1$argptr") #0 !dbg !6 {
alloca_1:
  %"PARAM1$locptr" = alloca %"QNCA_a0$i8*$rank1$"*, align 8
  store %"QNCA_a0$i8*$rank1$"* %"PARAM1$argptr", %"QNCA_a0$i8*$rank1$"** %"PARAM1$locptr", align 1
  call void @llvm.dbg.declare(metadata %"QNCA_a0$i8*$rank1$"** %"PARAM1$locptr", metadata !9, metadata !DIExpression(DW_OP_deref)), !dbg !14
  ret void, !dbg !15
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!1, !2, !3}
!llvm.dbg.cu = !{!5}

!0 = !{}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"CodeView", i32 1}
!4 = !DIFile(filename: "test.f90", directory: "C:\\path\\to")
!5 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !4, producer: "Fortran", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !0, splitDebugInlining: false, nameTableKind: None)
!6 = distinct !DISubprogram(name: "SUB1", linkageName: "F_BOUNDS_ip_SUB1", scope: !4, file: !4, line: 8, type: !7, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !5, retainedNodes: !0)
!7 = !DISubroutineType(types: !8)
!8 = !{null}
!9 = !DILocalVariable(name: "PARAM1", arg: 1, scope: !6, file: !4, line: 8, type: !10)
!10 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, elements: !12, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref))
!11 = !DIStringType(name: ".str.PARAM1", stringLengthExpression: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 8))
!12 = !{!13}
!13 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 48, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 56, DW_OP_deref))
!14 = !DILocation(line: 10, scope: !6)
!15 = !DILocation(line: 13, scope: !6)
