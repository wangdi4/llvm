; RUN: llc < %s -filetype=obj | llvm-readobj - --codeview | FileCheck %s
;
; This test verifies that the CodeView backend emits the expected
; debug info for a Fortran assumed rank array.
;
; This test is handwritten IR based on this Fortran code:
; -------------------------------------------------------
;   1    PROGRAM  TestProg
;   2   
;   3      REAL :: Array_1(1, 2)
;   4      Array_1 = 2.2
;   5      call TestRank (Array_1)
;   6   
;   7    CONTAINS
;   8   
;   9     SUBROUTINE TestRank(Assumed_Rank)
;   10       REAL :: Assumed_Rank(..)
;   11     END SUBROUTINE TestRank
;   12   
;   13   END PROGRAM TestProg
; -------------------------------------------------------
;
; CHECK:          CodeViewTypes [
; CHECK-NEXT:       Section: .debug$T
; CHECK:            FuncId ([[PROGFUNCID:0x[A-Z0-9]+]]) {
; CHECK-NEXT:         TypeLeafKind: LF_FUNC_ID
; CHECK-NEXT:         ParentScope: 0x0
; CHECK-NEXT:         FunctionType: void ()
; CHECK-NEXT:         Name: TESTPROG
; CHECK-NEXT:       }
; CHECK:            FuncId ([[TRFUNCID:0x[A-Z0-9]+]]) {
; CHECK-NEXT:         TypeLeafKind: LF_FUNC_ID
; CHECK-NEXT:         ParentScope: 0x0
; CHECK-NEXT:         FunctionType: void ()
; CHECK-NEXT:         Name: TESTRANK
; CHECK-NEXT:       }
; CHECK:            OEMType ([[DESC:0x[A-Z0-9]+]]) {
; CHECK-NEXT:         TypeLeafKind: LF_OEM (0x100F)
; CHECK-NEXT:         OEMId: LF_OEM_IDENT_MSF90 (0xF090)
; CHECK-NEXT:         OEMType: LF_recOEM_MSF90_DESCR_ARR (0x0)
; CHECK-NEXT:         Count: 2
; CHECK-NEXT:         ElementType: float (0x40)
; CHECK-NEXT:         BoundsKey: 0x0
; CHECK-NEXT:         ArrayRank: 0
; CHECK-NEXT:         DescrSize: 72
; CHECK-NEXT:       }
; CHECK:            Pointer ([[PTR:0x[A-Z0-9]+]]) {
; CHECK-NEXT:         TypeLeafKind: LF_POINTER ([[PROGFUNCID]])
; CHECK-NEXT:         PointeeType: [[DESC]]
; CHECK:            }
; CHECK:          CodeViewDebugInfo [
; CHECK-NEXT:       Section: .debug$S
; CHECK:            GlobalProcIdSym {
; CHECK-NEXT:         Kind: S_GPROC32_ID
; CHECK:              FunctionType: TESTRANK ([[TRFUNCID]])
; CHECK-NEXT:         CodeOffset: TESTPROG_ip_TESTRANK
; CHECK:              DisplayName: TESTPROG::TESTRANK
; CHECK-NEXT:         LinkageName: TESTPROG_ip_TESTRANK
; CHECK-NEXT:       }
; CHECK:            LocalSym {
; CHECK-NEXT:         Kind: S_LOCAL (0x113E)
; CHECK-NEXT:         Type: & ([[PTR]])
; CHECK-NEXT:         Flags [ (0x1)
; CHECK-NEXT:           IsParameter (0x1)
; CHECK-NEXT:         ]
; CHECK-NEXT:         VarName: ASSUMED_RANK
; CHECK-NEXT:       }
; CHECK:            ProcEnd {
; CHECK-NEXT:         Kind: S_PROC_ID_END
;
; ModuleID = 'test.bc'
source_filename = "test.f90"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"TESTPROG$ARRAY_1" = internal global [2 x [1 x float]] zeroinitializer, align 16, !dbg !0
@0 = internal unnamed_addr constant i32 2, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 !dbg !2 {
alloca_0:
  %.T4_ = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !17, !llfort.type_idx !18
  call void @TESTPROG_ip_TESTRANK(ptr %.T4_), !dbg !19, !llfort.type_idx !20
  ret void, !dbg !21
}

; Function Attrs: noinline nounwind optnone uwtable
define void @TESTPROG_ip_TESTRANK(ptr noalias dereferenceable(72) %"ASSUMED_RANK$argptr") #0 !dbg !22 {
alloca_1:
  %"ASSUMED_RANK$locptr" = alloca ptr, align 8
  store ptr %"ASSUMED_RANK$argptr", ptr %"ASSUMED_RANK$locptr", align 8
  %ASSUMED_RANK.8 = load ptr, ptr %"ASSUMED_RANK$locptr", align 8
  call void @llvm.dbg.declare(metadata ptr %"ASSUMED_RANK$locptr", metadata !24, metadata !DIExpression(DW_OP_deref)), !dbg !28
  ret void, !dbg !29
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!14, !15, !16}
!llvm.dbg.cu = !{!6}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ARRAY_1", linkageName: "TESTPROG$ARRAY_1", scope: !2, file: !3, line: 3, type: !9, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "TESTPROG", linkageName: "MAIN__", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6, retainedNodes: !8)
!3 = !DIFile(filename: "test.f90", directory: "d:\\tmp")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0}
!8 = !{}
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !11)
!10 = !DIBasicType(name: "REAL*4", size: 32, encoding: DW_ATE_float)
!11 = !{!12, !13}
!12 = !DISubrange(count: 1, lowerBound: 1)
!13 = !DISubrange(count: 2, lowerBound: 1)
!14 = !{i32 8, !"PIC Level", i32 2}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 2, !"CodeView", i32 1}
!17 = !DILocation(line: 1, scope: !2)
!18 = !{i64 92}
!19 = !DILocation(line: 5, scope: !2)
!20 = !{i64 40}
!21 = !DILocation(line: 7, scope: !2)
!22 = distinct !DISubprogram(name: "TESTRANK", linkageName: "TESTPROG_ip_TESTRANK", scope: !2, file: !3, line: 9, type: !4, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !23)
!23 = !{!24}
!24 = !DILocalVariable(name: "ASSUMED_RANK", arg: 1, scope: !22, file: !3, line: 9, type: !25)
!25 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !26, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), rank: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 32, DW_OP_deref))
!26 = !{!27}
!27 = !DIGenericSubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_over, DW_OP_constu, 24, DW_OP_mul, DW_OP_constu, 16, DW_OP_constu, 48, DW_OP_plus, DW_OP_plus, DW_OP_plus, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_over, DW_OP_constu, 24, DW_OP_mul, DW_OP_constu, 16, DW_OP_constu, 48, DW_OP_plus, DW_OP_plus, DW_OP_plus, DW_OP_deref, DW_OP_over, DW_OP_constu, 24, DW_OP_mul, DW_OP_constu, 0, DW_OP_constu, 48, DW_OP_plus, DW_OP_plus, DW_OP_push_object_address, DW_OP_plus, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_over, DW_OP_constu, 24, DW_OP_mul, DW_OP_constu, 8, DW_OP_constu, 48, DW_OP_plus, DW_OP_plus, DW_OP_plus, DW_OP_deref))
!28 = !DILocation(line: 9, scope: !22)
!29 = !DILocation(line: 11, scope: !22)

