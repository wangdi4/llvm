; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

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
; CHECK:      define {{.*}} void @__omp_offloading_{{.*}}_MAIN___l8(
; CHECK-SAME: [[TYPE:ptr addrspace\(1\)]] [[PARAM:%[0-9a-zA-Z_\.]+]]
; CHECK-SAME: !dbg [[SP:![0-9]+]]
; CHECK:      call void @llvm.dbg.value(metadata [[TYPE]] [[PARAM]]
; CHECK-SAME: metadata [[LOCAL:![0-9]+]]
; CHECK-SAME: metadata !DIExpression(DW_OP_deref)
; CHECK:      }
;
; CHECK:      [[FILE:![0-9]+]] = !DIFile(filename: "codeextractor_debug_module.f90"
; CHECK:      [[TYPE:![0-9]+]] = !DICompositeType(tag: DW_TAG_array_type
; CHECK:      [[SP]] = distinct !DISubprogram(name: "MAIN__.DIR.OMP.TARGET
; CHECK-SAME: scope: [[FILE]]
; CHECK:      [[LOCAL]] = !DILocalVariable(name: "array_1"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: file: [[FILE]]
; CHECK-SAME: line: 5
; CHECK-SAME: type: [[TYPE]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@module_1_mp_array_1_ = available_externally addrspace(1) global [10 x i32] zeroinitializer, align 8, !dbg !0, !llfort.type_idx !16
@strlit = internal unnamed_addr addrspace(1) constant [49 x i8] c";anon_var$;codeextractor_debug_module.f90;8;24;;\00", !llfort.type_idx !17

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 !dbg !2 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !23
  br label %bb_new3, !dbg !24

bb_new3:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @module_1_mp_array_1_ to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @module_1_mp_array_1_ to ptr addrspace(4)), i64 40, i64 33, ptr addrspace(1) @strlit, ptr addrspace(4) null) ], !dbg !29
  %"$loop_ctr" = alloca i64, align 8, !dbg !26, !llfort.type_idx !30
  store i64 1, ptr %"$loop_ctr", align 8, !dbg !26
  br label %loop_test5, !dbg !26

loop_test5:  ; preds = %loop_body6, %bb_new3
  %"$loop_ctr_fetch.2" = load i64, ptr %"$loop_ctr", align 8, !dbg !26
  %rel.1 = icmp sle i64 %"$loop_ctr_fetch.2", 10, !dbg !26
  br i1 %rel.1, label %loop_body6, label %loop_exit7, !dbg !26

loop_body6:  ; preds = %loop_test5
  %"$loop_ctr_fetch.1" = load i64, ptr %"$loop_ctr", align 8, !dbg !26
  %"val$[]" = call ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8 0, i64 1, i64 4, ptr addrspace(4) elementtype(i32) addrspacecast (ptr addrspace(1) @module_1_mp_array_1_ to ptr addrspace(4)), i64 %"$loop_ctr_fetch.1"), !dbg !26, !llfort.type_idx !27
  store i32 0, ptr addrspace(4) %"val$[]", align 4, !dbg !26
  %"$loop_ctr_fetch.3" = load i64, ptr %"$loop_ctr", align 8, !dbg !26
  %add.1 = add nsw i64 %"$loop_ctr_fetch.3", 1, !dbg !26
  store i64 %add.1, ptr %"$loop_ctr", align 8, !dbg !26
  br label %loop_test5, !dbg !26

loop_exit7:  ; preds = %loop_test5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !24
  ret void, !dbg !28

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare ptr addrspace(4) @llvm.intel.subscript.p4.i64.i64.p4.i64(i8 %0, i64 %1, i64 %2, ptr addrspace(4) %3, i64 %4) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!llvm.module.flags = !{!18, !19, !20, !21}
!llvm.dbg.cu = !{!6}
!omp_offload.info = !{!22}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "array_1", linkageName: "module_1_mp_array_1_", scope: !2, file: !3, line: 5, type: !12, isLocal: false, isDefinition: true)
!2 = distinct !DISubprogram(name: "program_1", linkageName: "MAIN__", scope: !3, file: !3, line: 5, type: !4, scopeLine: 5, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !6, retainedNodes: !11)
!3 = !DIFile(filename: "codeextractor_debug_module.f90", directory: "/path/to")
!4 = !DISubroutineType(types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 22.0-1750", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, imports: !8, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0}
!8 = !{!9}
!9 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !2, entity: !10, file: !3, line: 6)
!10 = !DIModule(scope: null, name: "module_1", isDecl: true)
!11 = !{}
!12 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, elements: !14)
!13 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!14 = !{!15}
!15 = !DISubrange(count: 10, lowerBound: 1)
!16 = !{i64 15}
!17 = !{i64 23}
!18 = !{i32 2, !"Debug Info Version", i32 3}
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 7, !"openmp", i32 50}
!21 = !{i32 7, !"openmp-device", i32 50}
!22 = !{i32 0, i32 53, i32 -1914798726, !"MAIN__", i32 8, i32 0, i32 0}
!23 = !{i64 19}
!24 = !DILocation(line: 10, column: 9, scope: !25)
!25 = distinct !DILexicalBlock(scope: !2, file: !3, line: 8, column: 9)
!26 = !DILocation(line: 9, column: 3, scope: !25)
!27 = !{i64 25}
!28 = !DILocation(line: 11, column: 1, scope: !2)
!29 = !DILocation(line: 8, column: 9, scope: !25)
!30 = !{i64 3}
