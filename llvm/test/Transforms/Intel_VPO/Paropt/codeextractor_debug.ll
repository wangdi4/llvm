; RUN: opt -S -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s 2>&1 | FileCheck %s
; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload %s 2>&1 | FileCheck %s

; When a region is code extracted, make sure the existing debug information is
; updated accordingly. Specifically:
;   1. A DISubprogram is created for the extracted routine.
;   2. No llvm.dbg. intrinsics should exist for variable "a".
;   3. One llvm.dbg.value intrinsic should exist for variable "b".

; CHECK:       define{{.*}} void @__omp_offloading_{{[^(]+}}(
; CHECK-SAME:  i32 addrspace(1)*{{.*}} %b.ascast
; CHECK-SAME:  )
; CHECK-SAME:  !dbg [[SP:![0-9]+]]
; CHECK-LABEL: newFuncRoot:
; CHECK-NOT:   call void @llvm.dbg.value(metadata i32 addrspace(1)* %a
; CHECK:       call void @llvm.dbg.value(metadata i32 addrspace(1)* %b.ascast,
; CHECK-SAME:  metadata [[B_VAR:![0-9]+]]
; CHECK-SAME:  metadata !DIExpression(DW_OP_deref))
; CHECK-SAME:  !dbg [[L28C13:![0-9]+]]
; CHECK-NOT:   call void @llvm.dbg.value(metadata i32 addrspace(1)* %a
; CHECK-LABEL: master.thread.code:
; CHECK:       store i32 42, i32 addrspace(1)* %b.ascast, align 4
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

; ModuleID = 'foo.cpp'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo() #0 !dbg !10 {
entry:
  %a = alloca i32, align 4
  %a.ascast = addrspacecast i32* %a to i32 addrspace(4)*
  %b = alloca i32, align 4
  %b.ascast = addrspacecast i32* %b to i32 addrspace(4)*
  call void @llvm.dbg.declare(metadata i32* %b, metadata !13, metadata !DIExpression()), !dbg !14
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %b.ascast, i32 addrspace(4)* %b.ascast, i64 4, i64 35) ], !dbg !15
  call void @llvm.dbg.declare(metadata i32* %a, metadata !12, metadata !DIExpression()), !dbg !14
  store i32 42, i32 addrspace(4)* %b.ascast, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  store i32 42, i32* %a, align 4
  ret void
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!6}
!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 2052, i32 95427652, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 7, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{}
!5 = !{!"clang version 8.0.0"}
!6 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !7, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, imports: !4, splitDebugInlining: false, nameTableKind: None)
!7 = !DIFile(filename: "foo.cpp", directory: "/path/to")
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !{!8}
!10 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 22, type: !11, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !4)
!11 = !DISubroutineType(types: !9)
!12 = !DILocalVariable(name: "a", scope: !10, file: !7, line: 28, type: !8)
!13 = !DILocalVariable(name: "b", scope: !10, file: !7, line: 28, type: !8)
!14 = !DILocation(line: 28, column: 13, scope: !10)
!15 = !DILocation(line: 26, column: 1, scope: !10)
