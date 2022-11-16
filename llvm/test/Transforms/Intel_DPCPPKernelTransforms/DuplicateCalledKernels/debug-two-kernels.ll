; RUN: opt -passes=dpcpp-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check cloned functions' linkage name in !dbg are changed.
;
;       test2
;         |
;       test
;         |
;        foo

; CHECK-DAG: distinct !DISubprogram(name: "foo", linkageName: "foo.clone",
; CHECK-DAG: distinct !DISubprogram(name: "test", linkageName: "test.clone",

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.j = internal addrspace(3) global i32 undef, align 4, !dbg !0

; Function Attrs: convergent noinline norecurse nounwind
define internal fastcc i32 @foo() unnamed_addr #0 !dbg !21 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(3)* @test.j, metadata !25, metadata !DIExpression()), !dbg !26
  %0 = load i32, i32 addrspace(3)* @test.j, align 4, !dbg !27, !tbaa !28
  ret i32 %0, !dbg !32
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 addrspace(1)* noalias noundef align 4 %results) local_unnamed_addr #1 !dbg !2 !kernel_arg_addr_space !33 !kernel_arg_access_qual !34 !kernel_arg_type !35 !kernel_arg_base_type !35 !kernel_arg_type_qual !36 !kernel_arg_name !37 !kernel_arg_host_accessible !38 !kernel_arg_pipe_depth !39 !kernel_arg_pipe_io !36 !kernel_arg_buffer_location !36 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %results, metadata !12, metadata !DIExpression()), !dbg !40
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #4, !dbg !41
  call void @llvm.dbg.value(metadata i64 %call, metadata !13, metadata !DIExpression(DW_OP_LLVM_convert, 64, DW_ATE_unsigned, DW_OP_LLVM_convert, 32, DW_ATE_unsigned, DW_OP_stack_value)), !dbg !40
  %call1 = tail call fastcc i32 @foo() #5, !dbg !42
  %idxprom = and i64 %call, 4294967295, !dbg !43
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom, !dbg !43
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4, !dbg !44, !tbaa !28
  ret void, !dbg !45
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define dso_local void @test2(i32 addrspace(1)* noalias noundef align 4 %results) local_unnamed_addr #1 !dbg !46 !kernel_arg_addr_space !33 !kernel_arg_access_qual !34 !kernel_arg_type !35 !kernel_arg_base_type !35 !kernel_arg_type_qual !36 !kernel_arg_name !37 !kernel_arg_host_accessible !38 !kernel_arg_pipe_depth !39 !kernel_arg_pipe_io !36 !kernel_arg_buffer_location !36 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %results, metadata !48, metadata !DIExpression()), !dbg !49
  tail call void @test(i32 addrspace(1)* noundef align 4 %results) #5, !dbg !50
  ret void, !dbg !51
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { convergent mustprogress nofree nounwind readnone willreturn }
attributes #3 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent }

!llvm.dbg.cu = !{!8}
!llvm.module.flags = !{!17, !18}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!19}
!sycl.kernels = !{!20}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "j", scope: !2, file: !3, line: 7, type: !7, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "test", scope: !3, file: !3, line: 5, type: !4, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !11)
!3 = !DIFile(filename: "test.cl", directory: "/tmp")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null, !6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !9, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !10, splitDebugInlining: false, nameTableKind: None)
!9 = !DIFile(filename: "test.cl", directory: "/tmp")
!10 = !{!0}
!11 = !{!12, !13}
!12 = !DILocalVariable(name: "results", arg: 1, scope: !2, file: !3, line: 5, type: !6)
!13 = !DILocalVariable(name: "tid", scope: !2, file: !3, line: 6, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !15, line: 105, baseType: !16)
!15 = !DIFile(filename: "opencl-c-base.h", directory: "")
!16 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!17 = !{i32 7, !"Dwarf Version", i32 4}
!18 = !{i32 2, !"Debug Info Version", i32 3}
!19 = !{i32 2, i32 0}
!20 = !{void (i32 addrspace(1)*)* @test, void (i32 addrspace(1)*)* @test2}
!21 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 1, type: !22, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !24)
!22 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !23)
!23 = !{!7, !6}
!24 = !{!25}
!25 = !DILocalVariable(name: "j", arg: 1, scope: !21, file: !3, line: 1, type: !6)
!26 = !DILocation(line: 0, scope: !21)
!27 = !DILocation(line: 2, column: 10, scope: !21)
!28 = !{!29, !29, i64 0}
!29 = !{!"int", !30, i64 0}
!30 = !{!"omnipotent char", !31, i64 0}
!31 = !{!"Simple C/C++ TBAA"}
!32 = !DILocation(line: 2, column: 3, scope: !21)
!33 = !{i32 1}
!34 = !{!"none"}
!35 = !{!"int*"}
!36 = !{!""}
!37 = !{!"results"}
!38 = !{i1 false}
!39 = !{i32 0}
!40 = !DILocation(line: 0, scope: !2)
!41 = !DILocation(line: 6, column: 14, scope: !2)
!42 = !DILocation(line: 8, column: 18, scope: !2)
!43 = !DILocation(line: 8, column: 3, scope: !2)
!44 = !DILocation(line: 8, column: 16, scope: !2)
!45 = !DILocation(line: 9, column: 1, scope: !2)
!46 = distinct !DISubprogram(name: "test2", scope: !3, file: !3, line: 11, type: !4, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !8, retainedNodes: !47)
!47 = !{!48}
!48 = !DILocalVariable(name: "results", arg: 1, scope: !46, file: !3, line: 11, type: !6)
!49 = !DILocation(line: 0, scope: !46)
!50 = !DILocation(line: 12, column: 3, scope: !46)
!51 = !DILocation(line: 13, column: 1, scope: !46)
