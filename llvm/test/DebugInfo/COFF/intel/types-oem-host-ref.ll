; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; Validate correct emission of the Intel OEM record LF_recOEM_MSF90_HOST_REF.
;
; This test is handwritten IR based on this Fortran code:
; -------------------------------------------------------
;   1     SUBROUTINE outer_1(outer_1_param_1)
;   2     IMPLICIT NONE
;   3     integer, intent(in) :: outer_1_param_1
;   4     integer :: outer_1_local_1
;   5     
;   6     outer_1_local_1 = outer_1_param_1
;   7   
;   8     CALL inner_1
;   9     RETURN
;  10     
;  11     CONTAINS
;  12       SUBROUTINE inner_1
;  13         integer :: inner_1_local_1
;  14         inner_1_local_1 = outer_1_local_1 + outer_1_param_1
;  15         RETURN
;  16       END SUBROUTINE inner_1
;  17     END SUBROUTINE outer_1
; -------------------------------------------------------
;
; CHECK:      CodeViewTypes [
; CHECK:        Section: .debug$T (7)
;
; CHECK:        Pointer (0x1003) {
; CHECK-NEXT:     TypeLeafKind: LF_POINTER (0x1002)
; CHECK-NEXT:     PointeeType: int (0x74)
;
; CHECK:        OEMType (0x1005) {
; CHECK-NEXT:     TypeLeafKind: LF_OEM (0x100F)
; CHECK-NEXT:     OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK-NEXT:     OEMType: LF_recOEM_MSF90_HOST_REF (0x3)
; CHECK-NEXT:     Count: 1
; CHECK-NEXT:     RefType: int (0x74)
; CHECK-NEXT:     Offset: 8
; CHECK-NEXT:   }
; CHECK-NEXT:   OEMType (0x1006) {
; CHECK-NEXT:     TypeLeafKind: LF_OEM (0x100F)
; CHECK-NEXT:     OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK-NEXT:     OEMType: LF_recOEM_MSF90_HOST_REF (0x3)
; CHECK-NEXT:     Count: 1
; CHECK-NEXT:     RefType: int& (0x1003)
; CHECK-NEXT:     Offset: 0
; CHECK-NEXT:   }
;
; CHECK:      CodeViewDebugInfo [
; CHECK:        Section: .debug$S (6)
;
; CHECK:        Subsection [
; CHECK-NEXT:     SubSectionType: Symbols (0xF1)
;
; CHECK:          GlobalProcIdSym {
; CHECK-NEXT:       Kind: S_GPROC32_ID (0x1147)
;
; CHECK:            FunctionType: OUTER_1 (0x1002)
; CHECK-NEXT:       CodeOffset: OUTER_1+0x0
;
; CHECK:            DisplayName: OUTER_1
; CHECK-NEXT:       LinkageName: OUTER_1
; CHECK-NEXT:     }
;
; CHECK:          GlobalProcIdSym {
; CHECK-NEXT:       Kind: S_GPROC32_ID (0x1147)
;
; CHECK:            FunctionType: INNER_1 (0x1004)
; CHECK-NEXT:       CodeOffset: OUTER_1_ip_INNER_1+0x0
;
; CHECK:            DisplayName: OUTER_1::INNER_1
; CHECK-NEXT:       LinkageName: OUTER_1_ip_INNER_1
; CHECK-NEXT:     }
;
; CHECK:          LocalSym {
; CHECK-NEXT:       Kind: S_LOCAL (0x113E)
; CHECK-NEXT:       Type: int (0x74)
; CHECK-NEXT:       Flags [ (0x0)
; CHECK-NEXT:       ]
; CHECK-NEXT:       VarName: INNER_1_LOCAL_1
; CHECK-NEXT:     }
;
; CHECK:          LocalSym {
; CHECK-NEXT:       Kind: S_LOCAL (0x113E)
; CHECK-NEXT:       Type: 0x1005
; CHECK-NEXT:       Flags [ (0x0)
; CHECK-NEXT:       ]
; CHECK-NEXT:       VarName: OUTER_1_LOCAL_1
; CHECK-NEXT:     }
;
; CHECK:          LocalSym {
; CHECK-NEXT:       Kind: S_LOCAL (0x113E)
; CHECK-NEXT:       Type: 0x1006
; CHECK-NEXT:       Flags [ (0x0)
; CHECK-NEXT:       ]
; CHECK-NEXT:       VarName: OUTER_1_PARAM_1
; CHECK-NEXT:     }
;
;
; -----------------------------------------------------------------------------
; ModuleID = 'uplevel-simple.bc'
source_filename = "uplevel-simple.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%OUTER_1.uplevel_type = type { i32*, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define void @OUTER_1(i32* noalias readonly dereferenceable(4) %"OUTER_1$OUTER_1_PARAM_1$argptr") #0 !dbg !13 {
alloca_0:
  %"$io_ctx" = alloca [6 x i64], align 8
  %OUTER_1.uplevel_rec.0 = alloca %OUTER_1.uplevel_type, align 8
  %"OUTER_1$OUTER_1_PARAM_1$locptr" = alloca i32*, align 8
  %.T1_ = alloca i64, align 8
  store i32* %"OUTER_1$OUTER_1_PARAM_1$argptr", i32** %"OUTER_1$OUTER_1_PARAM_1$locptr", align 1
  %"OUTER_1$OUTER_1_PARAM_1.1" = load i32*, i32** %"OUTER_1$OUTER_1_PARAM_1$locptr", align 1
  call void @llvm.dbg.declare(metadata i32** %"OUTER_1$OUTER_1_PARAM_1$locptr", metadata !17, metadata !DIExpression(DW_OP_deref)), !dbg !20
  %OUTER_1_PARAM_1.ul.GEP.0p = getelementptr inbounds %OUTER_1.uplevel_type, %OUTER_1.uplevel_type* %OUTER_1.uplevel_rec.0, i32 0, i32 0
  store i32* %"OUTER_1$OUTER_1_PARAM_1.1", i32** %OUTER_1_PARAM_1.ul.GEP.0p, align 1
  %OUTER_1_PARAM_1.ul.GEP.0.2 = load i32*, i32** %OUTER_1_PARAM_1.ul.GEP.0p, align 1
  %OUTER_1_LOCAL_1.ul.GEP.1 = getelementptr inbounds %OUTER_1.uplevel_type, %OUTER_1.uplevel_type* %OUTER_1.uplevel_rec.0, i32 0, i32 1
  call void @llvm.dbg.declare(metadata %OUTER_1.uplevel_type* %OUTER_1.uplevel_rec.0, metadata !19, metadata !DIExpression(DW_OP_plus_uconst, 8)), !dbg !21
  %OUTER_1_PARAM_1.ul.GEP.0.2_fetch.3 = load i32, i32* %OUTER_1_PARAM_1.ul.GEP.0.2, align 1, !dbg !22
  store i32 %OUTER_1_PARAM_1.ul.GEP.0.2_fetch.3, i32* %OUTER_1_LOCAL_1.ul.GEP.1, align 8, !dbg !22
  call void @OUTER_1_ip_INNER_1(%OUTER_1.uplevel_type* %OUTER_1.uplevel_rec.0), !dbg !23
  store i64 0, i64* %.T1_, align 8, !dbg !24
  ret void, !dbg !25
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define void @OUTER_1_ip_INNER_1(%OUTER_1.uplevel_type* %"INNER_1.uplevel_ptr1$argptr") #0 !dbg !26 {
alloca_1:
  %"$io_ctx" = alloca [6 x i64], align 8
  %"INNER_1.uplevel_ptr1$locptr" = alloca %OUTER_1.uplevel_type*, align 8
  %"INNER_1$INNER_1_LOCAL_1$_1" = alloca i32, align 8
  %.T2_ = alloca i64, align 8
  store %OUTER_1.uplevel_type* %"INNER_1.uplevel_ptr1$argptr", %OUTER_1.uplevel_type** %"INNER_1.uplevel_ptr1$locptr", align 1
  %INNER_1.uplevel_ptr1.4 = load %OUTER_1.uplevel_type*, %OUTER_1.uplevel_type** %"INNER_1.uplevel_ptr1$locptr", align 1
  %OUTER_1_PARAM_1.ul.GEP.0p = getelementptr inbounds %OUTER_1.uplevel_type, %OUTER_1.uplevel_type* %INNER_1.uplevel_ptr1.4, i32 0, i32 0
  %OUTER_1_PARAM_1.ul.GEP.0.5 = load i32*, i32** %OUTER_1_PARAM_1.ul.GEP.0p, align 1
  call void @llvm.dbg.declare(metadata i32* %"INNER_1$INNER_1_LOCAL_1$_1", metadata !28, metadata !DIExpression()), !dbg !31
  %OUTER_1_LOCAL_1.ul.GEP.1 = getelementptr inbounds %OUTER_1.uplevel_type, %OUTER_1.uplevel_type* %INNER_1.uplevel_ptr1.4, i32 0, i32 1
  call void @llvm.dbg.declare(metadata %OUTER_1.uplevel_type** %"INNER_1.uplevel_ptr1$locptr", metadata !29, metadata !DIExpression(DW_OP_deref, DW_OP_plus_uconst, 8)), !dbg !32
  call void @llvm.dbg.declare(metadata %OUTER_1.uplevel_type** %"INNER_1.uplevel_ptr1$locptr", metadata !30, metadata !DIExpression(DW_OP_deref, DW_OP_deref)), !dbg !33
  %OUTER_1_LOCAL_1.ul.GEP.1_fetch.6 = load i32, i32* %OUTER_1_LOCAL_1.ul.GEP.1, align 8, !dbg !34
  %OUTER_1_PARAM_1.ul.GEP.0.5_fetch.7 = load i32, i32* %OUTER_1_PARAM_1.ul.GEP.0.5, align 1, !dbg !34
  %add.1 = add nsw i32 %OUTER_1_LOCAL_1.ul.GEP.1_fetch.6, %OUTER_1_PARAM_1.ul.GEP.0.5_fetch.7, !dbg !34
  store i32 %add.1, i32* %"INNER_1$INNER_1_LOCAL_1$_1", align 8, !dbg !34
  store i64 0, i64* %.T2_, align 8, !dbg !35
  ret void, !dbg !36
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.dbg.cu = !{!3}
!llvm.linker.options = !{!5, !6, !7, !8, !9, !10, !11, !12}
!omp_offload.info = !{}

!0 = !{i32 7, !"PIC Level", i32 2}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !{i32 2, !"CodeView", i32 1}
!3 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !4, producer: "Fortran", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!4 = !DIFile(filename: "uplevel-simple.f90", directory: "d:\\tmp3")
!5 = !{!"/DEFAULTLIB:libcmt"}
!6 = !{!"/DEFAULTLIB:ifconsol.lib"}
!7 = !{!"/DEFAULTLIB:libifcoremt.lib"}
!8 = !{!"/DEFAULTLIB:libifport.lib"}
!9 = !{!"/DEFAULTLIB:libircmt"}
!10 = !{!"/DEFAULTLIB:libmmt"}
!11 = !{!"/DEFAULTLIB:oldnames"}
!12 = !{!"/DEFAULTLIB:svml_dispmt"}
!13 = distinct !DISubprogram(name: "OUTER_1", linkageName: "OUTER_1", scope: !4, file: !4, line: 1, type: !14, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{null}
!16 = !{!17, !19}
!17 = !DILocalVariable(name: "OUTER_1_PARAM_1", arg: 1, scope: !13, file: !4, line: 1, type: !18)
!18 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!19 = !DILocalVariable(name: "OUTER_1_LOCAL_1", scope: !13, file: !4, line: 4, type: !18)
!20 = !DILocation(line: 1, scope: !13)
!21 = !DILocation(line: 4, scope: !13)
!22 = !DILocation(line: 6, scope: !13)
!23 = !DILocation(line: 8, scope: !13)
!24 = !DILocation(line: 9, scope: !13)
!25 = !DILocation(line: 11, scope: !13)
!26 = distinct !DISubprogram(name: "INNER_1", linkageName: "OUTER_1_ip_INNER_1", scope: !13, file: !4, line: 12, type: !14, scopeLine: 12, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !27)
!27 = !{!28, !29, !30}
!28 = !DILocalVariable(name: "INNER_1_LOCAL_1", scope: !26, file: !4, line: 13, type: !18)
!29 = !DILocalVariable(name: "OUTER_1_LOCAL_1", scope: !26, file: !4, line: 4, type: !18, flags: DIFlagUplevelReference)
!30 = !DILocalVariable(name: "OUTER_1_PARAM_1", scope: !26, file: !4, line: 1, type: !18, flags: DIFlagUplevelReference)
!31 = !DILocation(line: 13, scope: !26)
!32 = !DILocation(line: 4, scope: !26)
!33 = !DILocation(line: 1, scope: !26)
!34 = !DILocation(line: 14, scope: !26)
!35 = !DILocation(line: 15, scope: !26)
!36 = !DILocation(line: 16, scope: !26)
