; RUN: opt -passes=sycl-kernel-barrier -S %s -o - | FileCheck %s

; Check that debug location of hoisted value is dropped and insert point is
; after debug intrinsic and alloca.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare void @llvm.dbg.value(metadata, metadata, metadata) #0

declare void @_Z18work_group_barrierj12memory_scope()

define void @_ZGVeN16uu___omp_offloading_801_b809e_test_target_device_IP_test_target_device_clause__l40(ptr addrspace(1) %"ascast$test_target_device_clause$ARRAY$_277", i64 %"ascast$test_target_device_clause$DEV$_277.val.zext") {
newFuncRoot:
; CHECK-LABEL: newFuncRoot:
; CHECK-NEXT:  call void @llvm.dbg.value(metadata ptr addrspace(1) %"ascast$test_target_device_clause$ARRAY$_277", metadata {{.*}}, metadata !DIExpression(DW_OP_deref)), !dbg
; CHECK-NEXT:  %pCurrBarrier = alloca i32, align 4
; CHECK-NEXT:  %pCurrSBIndex = alloca i64, align 8
; CHECK-NEXT:  %pLocalIds = alloca [3 x i64], align 8
; CHECK-NEXT:  %"ascast$test_target_device_clause$DEV$_277.val.zext.trunc" = trunc i64 %"ascast$test_target_device_clause$DEV$_277.val.zext" to i32
; CHECK-NOT:   !dbg
; CHECK-NEXT:  %add.2 = add i32 %"ascast$test_target_device_clause$DEV$_277.val.zext.trunc", 1
; CHECK-NOT:   !dbg
; CHECK:       br label %FirstBB

  call void @dummy_barrier.()
  call void @llvm.dbg.value(metadata ptr addrspace(1) %"ascast$test_target_device_clause$ARRAY$_277", metadata !3, metadata !DIExpression(DW_OP_deref)), !dbg !10
  %"ascast$test_target_device_clause$DEV$_277.val.zext.trunc" = trunc i64 %"ascast$test_target_device_clause$DEV$_277.val.zext" to i32, !dbg !11
  %add.2 = add i32 %"ascast$test_target_device_clause$DEV$_277.val.zext.trunc", 1, !dbg !13
  br label %Split.Barrier.BB21

Split.Barrier.BB21:                               ; preds = %newFuncRoot
  call void @_Z18work_group_barrierj12memory_scope()
  %0 = or i32 %add.2, 0
  ret void
}

declare void @dummy_barrier.()

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !2, producer: "Intel(R) Fortran 24.0-1114", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!2 = !DIFile(filename: "test", directory: "")
!3 = !DILocalVariable(name: "array", scope: !4, file: !2, line: 26, type: !8)
!4 = distinct !DISubprogram(name: "test_target_device_IP_test_target_device_clause_", scope: null, file: !2, line: 40, type: !5, scopeLine: 40, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagMainSubprogram, unit: !1, templateParams: !7, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{}
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 32000, elements: !7)
!9 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!10 = !DILocation(line: 26, column: 23, scope: !4)
!11 = !DILocation(line: 40, column: 7, scope: !12)
!12 = distinct !DILexicalBlock(scope: !4, file: !2, line: 40, column: 7)
!13 = !DILocation(line: 41, column: 41, scope: !12)
