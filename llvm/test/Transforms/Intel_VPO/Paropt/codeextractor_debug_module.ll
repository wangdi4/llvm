; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s
;
; When generating the offload routine for the outlined target region, verify:
;   1. The module variable 'array_1' mapped to the device has debug info.
;   2. The module parameter module_1_mp_array_1 is homed to a stack location.
;   3. The original global variable debug information was pruned.
;
; The IR below was based on the following source file:
; -- codeextractor_debug_module.f90 -------------------------------------------
; MODULE module_1
;   INTEGER :: array_1(10)
; END MODULE module_1
;
; PROGRAM program_1
;   USE module_1
;   implicit none
;   !$omp target map(to: array_1)
;   array_1(:) = 0
;   !$omp end target
; END PROGRAM program_1
; -----------------------------------------------------------------------------
;
; CHECK:      define{{.*}} void @__omp_offloading_{{.*}}_MAIN___l12(
; CHECK-SAME: [[TYPE:\[10 x i32\] addrspace\(1\)\*]] [[PARAM:%[0-9a-zA-Z_\.]+]]
; CHECK-SAME: !dbg [[SP:![0-9]+]]
; CHECK:      call void @llvm.dbg.value(metadata [[TYPE]] [[PARAM]]
; CHECK-SAME: metadata [[LOCAL:![0-9]+]]
; CHECK-SAME: metadata !DIExpression(DW_OP_deref)
; CHECK:      }
;
; CHECK:      [[FILE:![0-9]+]] = !DIFile(filename: "test.f90"
; CHECK:      [[TYPE:![0-9]+]] = !DICompositeType(tag: DW_TAG_array_type
; CHECK:      [[SP]] = distinct !DISubprogram(name: "MAIN__.DIR.OMP.TARGET
; CHECK-SAME: scope: [[FILE]]
; CHECK:      [[LOCAL]] = !DILocalVariable(name: "array_1"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: file: [[FILE]]
; CHECK-SAME: line: 9
; CHECK-SAME: type: [[TYPE]]
;
; ModuleID = 'codeextractor_debug_module.f90'
source_filename = "codeextractor_debug_module.f90"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@module_1_mp_array_1_ = available_externally addrspace(1) global [10 x i32] zeroinitializer, align 8, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 !dbg !2 {
bb.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"([10 x i32] addrspace(4)* addrspacecast ([10 x i32] addrspace(1)* @module_1_mp_array_1_ to [10 x i32] addrspace(4)*), [10 x i32] addrspace(4)* addrspacecast ([10 x i32] addrspace(1)* @module_1_mp_array_1_ to [10 x i32] addrspace(4)*), i64 40, i64 33, i8 addrspace(4)* null, i8 addrspace(4)* null) ], !dbg !23
  %"$loop_ctr" = alloca i64, align 8, !dbg !21
  store i64 1, i64* %"$loop_ctr", align 1, !dbg !21
  br label %bb.2, !dbg !21

bb.2:                                       ; preds = %bb.3, %bb.1
  %"$loop_ctr_fetch.2" = load i64, i64* %"$loop_ctr", align 1, !dbg !21
  %rel.1 = icmp sle i64 %"$loop_ctr_fetch.2", 10, !dbg !21
  br i1 %rel.1, label %bb.3, label %bb.4, !dbg !21

bb.3:                                       ; preds = %bb.2
  %"$loop_ctr_fetch.1" = load i64, i64* %"$loop_ctr", align 1, !dbg !21
  %1 = sub nsw i64 %"$loop_ctr_fetch.1", 1, !dbg !21
  %2 = getelementptr inbounds i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* getelementptr inbounds ([10 x i32], [10 x i32] addrspace(1)* @module_1_mp_array_1_, i32 0, i32 0) to i32 addrspace(4)*), i64 %1, !dbg !21
  store i32 0, i32 addrspace(4)* %2, align 1, !dbg !21
  %"$loop_ctr_fetch.3" = load i64, i64* %"$loop_ctr", align 1, !dbg !21
  %add.1 = add nsw i64 %"$loop_ctr_fetch.3", 1, !dbg !21
  store i64 %add.1, i64* %"$loop_ctr", align 1, !dbg !21
  br label %bb.2, !dbg !21

bb.4:                                       ; preds = %bb.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !19
  ret void, !dbg !22
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!llvm.module.flags = !{!16, !17}
!llvm.dbg.cu = !{!6}
!omp_offload.info = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "array_1", linkageName: "module_1_mp_array_1_", scope: !11, file: !3, line: 9, type: !12, isLocal: false, isDefinition: true)
!2 = distinct !DISubprogram(name: "program_1", linkageName: "MAIN__", scope: !3, file: !3, line: 9, type: !4, scopeLine: 9, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6, retainedNodes: !7)
!3 = !DIFile(filename: "test.f90", directory: "/path/to")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 21.0-2589", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !7, globals: !8, imports: !9, splitDebugInlining: false, nameTableKind: None)
!7 = !{}
!8 = !{!0}
!9 = !{!10}
!10 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !2, entity: !11, file: !3, line: 10)
!11 = !DIModule(scope: null, name: "module_1", isDecl: true)
!12 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, elements: !14)
!13 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!14 = !{!15}
!15 = !DISubrange(count: 10, lowerBound: 1)
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 2, !"Dwarf Version", i32 4}
!18 = !{i32 0, i32 2071, i32 1862323, !"MAIN__", i32 12, i32 0, i32 0}
!19 = !DILocation(line: 14, column: 9, scope: !20)
!20 = distinct !DILexicalBlock(scope: !2, file: !3, line: 12, column: 9)
!21 = !DILocation(line: 13, column: 3, scope: !20)
!22 = !DILocation(line: 15, column: 1, scope: !2)
!23 = !DILocation(line: 12, column: 9, scope: !20)
