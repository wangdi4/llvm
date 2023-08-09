; RUN: opt -S -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s 2>&1 | FileCheck %s
; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload %s 2>&1 | FileCheck %s

; When a region is code extracted, make sure the existing debug information is
; updated accordingly. Specifically:
;   1. A DISubprogram is created for the extracted routine.
;   2. No llvm.dbg. intrinsics should exist for variable "a".
;   3. One llvm.dbg.value intrinsic should exist for variable "b".

; CHECK:       define{{.*}} void @__omp_offloading_{{[^(]+}}(
; CHECK-SAME:  ptr addrspace(1) {{.*}} %b.ascast
; CHECK-SAME:  )
; CHECK-SAME:  !dbg [[SP:![0-9]+]]
; CHECK-LABEL: newFuncRoot:
; CHECK-NOT:   call void @llvm.dbg.value(metadata ptr addrspace(1) %a
; CHECK:       call void @llvm.dbg.value(metadata ptr addrspace(1) %b.ascast,
; CHECK-SAME:  metadata [[B_VAR:![0-9]+]]
; CHECK-SAME:  metadata !DIExpression(DW_OP_deref))
; CHECK-SAME:  !dbg [[L28C13:![0-9]+]]
; CHECK-NOT:   call void @llvm.dbg.value(metadata ptr addrspace(1) %a
; CHECK-LABEL: master.thread.code:
; CHECK:       store i32 42, ptr addrspace(1) %b.ascast, align 4
; CHECK:       }
;
; CHECK:       [[FILE:![0-9]+]] = !DIFile(filename: "foo.cpp"
; CHECK:       [[SP]] = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET{{.*}}"
; CHECK-SAME:  scope: [[FILE]]
; CHECK-SAME:  DIFlagArtificial
; CHECK-SAME:  DISPFlagLocalToUnit
; CHECK-SAME:  DISPFlagDefinition
; CHECK-NOT:   !DILocalVariable(name: "a"
; CHECK:       [[B_VAR]] = !DILocalVariable(name: "b",
; CHECK-SAME:  scope: [[SP]]
; CHECK-NOT:   !DILocalVariable(name: "a"
; CHECK:       [[L28C13]] = !DILocation(line: 28, column: 13
; CHECK-SAME:  scope: [[SP]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define dso_local spir_func void @foo() #0 !dbg !8 {
entry:
  %a = alloca i32, align 4
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b = alloca i32, align 4
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  call void @llvm.dbg.declare(metadata ptr %b, metadata !12, metadata !DIExpression()), !dbg !13
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %b.ascast, i64 4, i64 35) ], !dbg !14
  call void @llvm.dbg.declare(metadata ptr %a, metadata !15, metadata !DIExpression()), !dbg !13
  store i32 42, ptr addrspace(4) %b.ascast, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  store i32 42, ptr %a, align 4
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "may-have-openmp-directive"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5, !6}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "foo.cpp", directory: "/path/to")
!2 = !{}
!3 = !{i32 0, i32 2052, i32 95427652, !"foo", i32 3, i32 0, i32 0}
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 22, type: !9, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "b", scope: !8, file: !1, line: 28, type: !11)
!13 = !DILocation(line: 28, column: 13, scope: !8)
!14 = !DILocation(line: 26, column: 1, scope: !8)
!15 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 28, type: !11)
