; RUN: opt -S -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s 2>&1 | FileCheck %s
; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload %s 2>&1 | FileCheck %s

; When the parallel region is code extracted, make sure the privatized
; variables "a" and "b" retain the debug information.

; CHECK: @a.ascast.priv.__global = internal addrspace(1) global i32 0, align 1
;
; CHECK: define{{.*}} void @__omp_offloading_{{.*}} !dbg [[WREGION:![0-9]+]] {
; CHECK:   %b.ascast.priv = alloca i64
; CHECK:   call void @llvm.dbg.declare(metadata i64* %b.ascast.priv, metadata [[B:![0-9]+]], metadata !DIExpression())
; CHECK-NOT: call void @llvm.dbg.declare({{.*}}@a.ascast.priv.__global
; CHECK:   store i32 42, i32 addrspace(1)* @a.ascast.priv.__global, align 4
; CHECK:   store i64 42, i64* %b.ascast.priv, align 4
; CHECK: }
;
; CHECK: [[WREGION]] = distinct !DISubprogram(name: "foo.DIR.OMP.TARGET{{.*}}",
; CHECK: [[B]] = !DILocalVariable(name: "b",
; CHECK-SAME: scope: [[WREGION]]

; ModuleID = 'foo.cpp'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo() #0 !dbg !11 {
entry:
  %a = alloca i32, align 8
  %a.ascast = addrspacecast i32* %a to i32 addrspace(4)*
  %b = alloca i64, align 8
  %b.ascast = addrspacecast i64* %b to i64 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %a.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(i64 addrspace(4)* %b.ascast) ], !dbg !16
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %a.ascast, metadata !13, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.declare(metadata i64 addrspace(4)* %b.ascast, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 42, i32 addrspace(4)* %a.ascast, align 4
  store i64 42, i64 addrspace(4)* %b.ascast, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
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
!9 = !DIBasicType(name: "int", size: 64, encoding: DW_ATE_signed)
!10 = !{!8}
!11 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 22, type: !12, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !4)
!12 = !DISubroutineType(types: !10)
!13 = !DILocalVariable(name: "a", scope: !11, file: !7, line: 28, type: !8)
!14 = !DILocalVariable(name: "b", scope: !11, file: !7, line: 28, type: !9)
!15 = !DILocation(line: 28, column: 13, scope: !11)
!16 = !DILocation(line: 26, column: 1, scope: !11)
