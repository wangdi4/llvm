; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check cloned function's linkage name in !dbg isn't changed.
;
;       test2
;         |
;       test
;         |
;        foo

; CHECK-DAG: distinct !DISubprogram(name: "test", scope:
; CHECK-DAG: distinct !DISubprogram(name: "foo", scope:
; CHECK-DAG: distinct !DISubprogram(name: "test2", scope:
; CHECK-DAG: distinct !DISubprogram(name: "foo", scope:
; CHECK-DAG: distinct !DISubprogram(name: "test", scope:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.j = internal addrspace(3) global i32 undef, align 4, !dbg !0

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i32 @foo() unnamed_addr #0 !dbg !20 {
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(3) @test.j, metadata !24, metadata !DIExpression()), !dbg !25
  %0 = load i32, ptr addrspace(3) @test.j, align 4, !dbg !26, !tbaa !27
  ret i32 %0, !dbg !31
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr #1 !dbg !2 !kernel_arg_addr_space !32 !kernel_arg_access_qual !33 !kernel_arg_type !34 !kernel_arg_base_type !34 !kernel_arg_type_qual !35 !kernel_arg_name !36 !kernel_arg_host_accessible !37 !kernel_arg_pipe_depth !38 !kernel_arg_pipe_io !35 !kernel_arg_buffer_location !35 !arg_type_null_val !51 {
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %results, metadata !11, metadata !DIExpression()), !dbg !39
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #4, !dbg !40
  call void @llvm.dbg.value(metadata i64 %call, metadata !12, metadata !DIExpression(DW_OP_LLVM_convert, 64, DW_ATE_unsigned, DW_OP_LLVM_convert, 32, DW_ATE_unsigned, DW_OP_stack_value)), !dbg !39
  %call1 = tail call fastcc i32 @foo() #5, !dbg !41
  %idxprom = and i64 %call, 4294967295, !dbg !42
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %results, i64 %idxprom, !dbg !42
  store i32 %call1, ptr addrspace(1) %arrayidx, align 4, !dbg !43, !tbaa !27
  ret void, !dbg !44
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define dso_local void @test2(ptr addrspace(1) noalias noundef align 4 %results) local_unnamed_addr #1 !dbg !45 !kernel_arg_addr_space !32 !kernel_arg_access_qual !33 !kernel_arg_type !34 !kernel_arg_base_type !34 !kernel_arg_type_qual !35 !kernel_arg_name !36 !kernel_arg_host_accessible !37 !kernel_arg_pipe_depth !38 !kernel_arg_pipe_io !35 !kernel_arg_buffer_location !35 !arg_type_null_val !51 {
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %results, metadata !47, metadata !DIExpression()), !dbg !48
  tail call void @test(ptr addrspace(1) noundef align 4 %results) #5, !dbg !49
  ret void, !dbg !50
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { convergent nounwind willreturn memory(none) }
attributes #5 = { convergent }

!llvm.dbg.cu = !{!8}
!llvm.module.flags = !{!16, !17}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!18}
!sycl.kernels = !{!19}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "j", scope: !2, file: !3, line: 7, type: !7, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "test", scope: !3, file: !3, line: 5, type: !4, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !10)
!3 = !DIFile(filename: "test.cl", directory: "/tmp")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null, !6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !9, splitDebugInlining: false, nameTableKind: None)
!9 = !{!0}
!10 = !{!11, !12}
!11 = !DILocalVariable(name: "results", arg: 1, scope: !2, file: !3, line: 5, type: !6)
!12 = !DILocalVariable(name: "tid", scope: !2, file: !3, line: 6, type: !13)
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !14, line: 105, baseType: !15)
!14 = !DIFile(filename: "opencl-c-base.h", directory: "")
!15 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!16 = !{i32 7, !"Dwarf Version", i32 4}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 2, i32 0}
!19 = !{ptr @test, ptr @test2}
!20 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 1, type: !21, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !23)
!21 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !22)
!22 = !{!7, !6}
!23 = !{!24}
!24 = !DILocalVariable(name: "j", arg: 1, scope: !20, file: !3, line: 1, type: !6)
!25 = !DILocation(line: 0, scope: !20)
!26 = !DILocation(line: 2, column: 10, scope: !20)
!27 = !{!28, !28, i64 0}
!28 = !{!"int", !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 2, column: 3, scope: !20)
!32 = !{i32 1}
!33 = !{!"none"}
!34 = !{!"int*"}
!35 = !{!""}
!36 = !{!"results"}
!37 = !{i1 false}
!38 = !{i32 0}
!39 = !DILocation(line: 0, scope: !2)
!40 = !DILocation(line: 6, column: 14, scope: !2)
!41 = !DILocation(line: 8, column: 18, scope: !2)
!42 = !DILocation(line: 8, column: 3, scope: !2)
!43 = !DILocation(line: 8, column: 16, scope: !2)
!44 = !DILocation(line: 9, column: 1, scope: !2)
!45 = distinct !DISubprogram(name: "test2", scope: !3, file: !3, line: 11, type: !4, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !46)
!46 = !{!47}
!47 = !DILocalVariable(name: "results", arg: 1, scope: !45, file: !3, line: 11, type: !6)
!48 = !DILocation(line: 0, scope: !45)
!49 = !DILocation(line: 12, column: 3, scope: !45)
!50 = !DILocation(line: 13, column: 1, scope: !45)
!51 = !{ptr addrspace(1) null}
