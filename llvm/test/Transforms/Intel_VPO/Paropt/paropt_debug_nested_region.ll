; RUN: opt -S -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt 2>&1 %s
; RUN: opt -S -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload %s 2>&1 | FileCheck %s
;
; This is a reduced test case based on IR generated for target_data.cpp below.
; Validate debug information is properly emitted for offloaded variable "i".
;
; -- target_data.cpp ----------------------------------------------------------
; #include <omp.h>
;
; #define DATA_SIZE 4096
;
; int main() {
;   int array[DATA_SIZE];
;   int workSize[2];
;   int deviceId = omp_get_default_device();
;
;   for (int i = 0; i < DATA_SIZE; ++i)
;     array[i] = 0;
;
; #pragma omp target enter data map(to: array, workSize)
; #pragma omp target parallel for
;   for (int i = 0; i < DATA_SIZE; i++) {
;     array[i] += i;
;     if (i == 0) {
;       workSize[0] = omp_get_num_teams();
;       workSize[1] = omp_get_thread_limit();
;     }
;   }
; #pragma omp target exit data map(from: array, workSize)
;
;   return 0;
; }
; -----------------------------------------------------------------------------
;
; CHECK: define {{.*}} void @__omp_offloading{{.*}} !dbg [[REGION:![0-9]+]] {
; CHECK:   %i2.ascast.priv = alloca i32, align 1
; CHECK:   call void @llvm.dbg.declare(metadata i32* %i2.ascast.priv, metadata [[I:![0-9]+]], metadata !DIExpression())
; CHECK-NOT:  call void @llvm.dbg.{{.*}}({{.*}}, metadata [[I]]
; CHECK:   store i32 0, i32* %i2.ascast.priv, align 4
; CHECK-NOT:  call void @llvm.dbg.{{.*}}({{.*}}, metadata [[I]]
; CHECK: }
; CHECK: [[REGION]] = distinct !DISubprogram(name: "main.DIR.OMP.{{.*}}"
; CHECK: [[BLOCK:![0-9]+]] = distinct !DILexicalBlock(scope: [[REGION]],{{.*}}line: 22, column: 1
; CHECK: [[I]] = !DILocalVariable(name: "i",{{.*}}scope: [[BLOCK]]
;

; ModuleID = 'target_data.cpp'
source_filename = "target_data.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden void @main() !dbg !9 {
entry:
  %i2 = alloca i32, align 4
  %i2.ascast = addrspacecast i32* %i2 to i32 addrspace(4)*
  br label %for.end

for.end:                                          ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i2.ascast) ], !dbg !12
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i2.ascast) ], !dbg !14
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %for.end
  br i1 undef, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i2.ascast, metadata !8, metadata !DIExpression()), !dbg !11
  store i32 0, i32 addrspace(4)* %i2.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind readnone speculatable willreturn }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!5}
!llvm.module.flags = !{!6, !7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "target_data.cpp", directory: "/path/to")
!2 = !{}
!3 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!4 = !{!3}
!5 = !{i32 0, i32 2065, i32 21976446, !"_Z4main", i32 14, i32 0, i32 0}
!6 = !{i32 7, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !DILocalVariable(name: "i", scope: !9, file: !1, line: 15, type: !3)
!9 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 5, type: !10, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !4)
!11 = !DILocation(line: 15, column: 12, scope: !9)
!12 = !DILocation(line: 21, column: 1, scope: !13)
!13 = distinct !DILexicalBlock(scope: !9, file: !1, line: 21, column: 1)
!14 = !DILocation(line: 22, column: 1, scope: !15)
!15 = distinct !DILexicalBlock(scope: !9, file: !1, line: 22, column: 1)
