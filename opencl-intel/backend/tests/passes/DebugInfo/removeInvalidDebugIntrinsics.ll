; RUN: %oclopt --cl-loop-creator -verify -S %s | FileCheck %s

; CHECK-NOT: call void @llvm.dbg.value
; CHECK-NOT: call void @llvm.dbg.declare

; ModuleID = 'reduced.ll'
source_filename = "test.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%0 = type { %1 }
%1 = type { [1 x i64] }

; Function Attrs: nounwind
define void @"test_kernel"(i32 addrspace(1)*, %0*, %0*, %0*) local_unnamed_addr #0 !dbg !3 !kernel_arg_addr_space !10 !kernel_arg_access_qual !11 !kernel_arg_type !12 !kernel_arg_type_qual !13 !kernel_arg_base_type !14 !vectorized_kernel !15 !no_barrier_path !16 !scalarized_kernel !17 !vectorized_width !18 !kernel_execution_length !19 !kernel_has_barrier !20 !kernel_has_global_sync !20 !max_wg_dimensions !18 {
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

declare [7 x i64] @"WG.boundaries.test_kernel"(i32 addrspace(1)*, %0*, %0*, %0*)

; Function Attrs: nounwind
define void @"__Vectorized_.test_kernel"(i32 addrspace(1)*, %0*, %0*, %0*) local_unnamed_addr #0 !dbg !21 !kernel_arg_addr_space !10 !kernel_arg_access_qual !11 !kernel_arg_type !12 !kernel_arg_type_qual !13 !kernel_arg_base_type !14 !vectorized_kernel !17 !no_barrier_path !16 !scalarized_kernel !1 !vectorized_width !22 !kernel_execution_length !23 !kernel_has_barrier !20 !kernel_has_global_sync !20 !max_wg_dimensions !18 !vectorization_dimension !2 !can_unite_workgroups !16 {
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %0, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.declare(metadata %0* %1, metadata !26, metadata !DIExpression()), !dbg !25
  ret void, !dbg !27
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { readnone }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{}
!opencl.kernels = !{!1}
!opencl.global_variable_total_size = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{void (i32 addrspace(1)*, %0*, %0*, %0*)* @"test_kernel"}
!2 = !{i32 0}
!3 = distinct !DISubprogram(name: "test_kernel", scope: null, file: !4, line: 14, type: !5, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized | DISPFlagMainSubprogram, unit: !9)
!4 = !DIFile(filename: "simple.cpp", directory: "dir")
!5 = !DISubroutineType(types: !6)
!6 = !{null, !7}
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !4, producer: "spirv", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!10 = !{i32 1, i32 0, i32 0, i32 0}
!11 = !{!"none", !"none", !"none", !"none"}
!12 = !{!"int*", !"type1", !"type2", !"type3"}
!13 = !{!"", !"", !"", !""}
!14 = !{!"int*", !"test_type", !"test_type", !"test_type"}
!15 = !{void (i32 addrspace(1)*, %0*, %0*, %0*)* @"__Vectorized_.test_kernel"}
!16 = !{i1 true}
!17 = !{null}
!18 = !{i32 1}
!19 = !{i32 18}
!20 = !{i1 false}
!21 = distinct !DISubprogram(name: "test_kernel", scope: null, file: !4, line: 14, type: !5, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized | DISPFlagMainSubprogram, unit: !9)
!22 = !{i32 4}
!23 = !{i32 19}
!24 = !DILocalVariable(name: "_arg_", arg: 1, scope: !21, file: !4, type: !7)
!25 = !DILocation(line: 0, scope: !21)
!26 = !DILocalVariable(name: "_arg_", arg: 2, scope: !21, file: !4, type: !7)
!27 = !DILocation(line: 567, column: 16, scope: !28)
!28 = !DILexicalBlockFile(scope: !21, discriminator: 0)
