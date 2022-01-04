; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; Validate correct emission of the Intel OEM record LF_recOEM_MSF90_DESCRIPTOR.
;
; This test is handwritten IR based on this Fortran code:
; -------------------------------------------------------
;   1     character(:), allocatable :: ch
;   2
;   3     allocate(character*5::ch)
;   4     ch = '12345'
;   5
;   6     print *, ch
;   7
;   8     end
; -------------------------------------------------------
;
; CHECK:      CodeViewTypes [
; CHECK:        Section: .debug$T (5)
;
; CHECK:        Array ([[STRING:0x[A-Z0-9]+]]) {
; CHECK-NEXT:     TypeLeafKind: LF_ARRAY (0x1503)
; CHECK-NEXT:     ElementType: char (0x70)
; CHECK-NEXT:     IndexType: unsigned __int64 (0x23)
; CHECK-NEXT:     SizeOf: 0
; CHECK-NEXT:     Name: .str.CH
; CHECK-NEXT:   }
;
; CHECK:        OEMType ([[DESC:0x[A-Z0-9]+]]) {
; CHECK-NEXT:     TypeLeafKind: LF_OEM (0x100F)
; CHECK-NEXT:     OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK-NEXT:     OEMType: LF_recOEM_MSF90_DESCRIPTOR (0x5)
; CHECK-NEXT:     Count: 1
; CHECK-NEXT:     RefType: .str.CH (0x1003)
; CHECK-NEXT:     DescrSize: 48
; CHECK-NEXT:   }
;
; CHECK:      ]
;
; CHECK:      CodeViewDebugInfo [
; CHECK-NEXT:   Section: .debug$S (4)
; CHECK:        Subsection [
; CHECK-NEXT:     SubSectionType: Symbols (0xF1)
;
; CHECK:          GlobalProcIdSym {
; CHECK:            Kind: S_GPROC32_ID (0x1147)
; CHECK:            DisplayName: _UNNAMED_MAIN$$
; CHECK:            LinkageName: MAIN__
; CHECK:          }
;
; CHECK:          DataSym {
; CHECK-NEXT:       Kind: S_LDATA32 (0x110C)
; CHECK-NEXT:       DataOffset: _UNNAMED_MAIN$$$CH+0x0
; CHECK-NEXT:       Type: [[DESC]]
; CHECK-NEXT:       DisplayName: CH
; CHECK-NEXT:       LinkageName: _UNNAMED_MAIN$$$CH
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

%"QNCA_a0$i8*$rank0$" = type { i8*, i64, i64, i64, i64, i64 }

@"_UNNAMED_MAIN$$$CH" = internal global %"QNCA_a0$i8*$rank0$" zeroinitializer, !dbg !0

define void @MAIN__() #0 !dbg !2 {
alloca_0:
  %fetch.1 = load i64, i64* getelementptr inbounds (%"QNCA_a0$i8*$rank0$", %"QNCA_a0$i8*$rank0$"* @"_UNNAMED_MAIN$$$CH", i32 0, i32 1), align 1, !dbg !14
  ret void, !dbg !17
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!10, !11}
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
!9 = !DIStringType(name: ".str.CH", stringLengthExpression: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 8))
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 2, !"CodeView", i32 1}
!13 = !DILocation(line: 1, scope: !2)
!14 = !DILocation(line: 3, scope: !2)
!15 = !DILocation(line: 4, scope: !2)
!16 = !DILocation(line: 6, scope: !2)
!17 = !DILocation(line: 8, scope: !2)
