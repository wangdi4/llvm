target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind optnone noinline
define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %a, ptr addrspace(1) noundef align 4 %b) #0 !dbg !6 !kernel_arg_addr_space !19 !kernel_arg_access_qual !20 !kernel_arg_type !21 !kernel_arg_base_type !21 !kernel_arg_type_qual !22 !kernel_arg_name !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !22 !kernel_arg_buffer_location !22 {
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %a, metadata !13, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata ptr addrspace(1) %b, metadata !14, metadata !DIExpression()), !dbg !26
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #3, !dbg !27
  call void @llvm.dbg.value(metadata i64 %call, metadata !15, metadata !DIExpression()), !dbg !26
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call, !dbg !28
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !dbg !28, !tbaa !29
  %mul = shl nsw i32 %0, 1, !dbg !33
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call, !dbg !34
  store i32 %mul, ptr addrspace(1) %arrayidx1, align 4, !dbg !35, !tbaa !29
  ret void, !dbg !36
}

; Function Attrs: convergent nounwind readnone willreturn
declare spir_func i64 @_Z13get_global_idj(i32 noundef) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { convergent norecurse nounwind optnone noinline "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent nounwind readnone willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.compiler.options = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 2, i32 0}
!5 = !{!"-cl-std=CL2.0", !"-g"}
!6 = distinct !DISubprogram(name: "test", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !12)
!7 = !DIFile(filename: "add.cl", directory: "")
!8 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !9)
!9 = !{null, !10, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{!13, !14, !15}
!13 = !DILocalVariable(name: "a", arg: 1, scope: !6, file: !7, line: 1, type: !10)
!14 = !DILocalVariable(name: "b", arg: 2, scope: !6, file: !7, line: 1, type: !10)
!15 = !DILocalVariable(name: "gid", scope: !6, file: !7, line: 2, type: !16)
!16 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !17, line: 116, baseType: !18)
!17 = !DIFile(filename: "opencl-c-base.h", directory: "")
!18 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!19 = !{i32 1, i32 1}
!20 = !{!"none", !"none"}
!21 = !{!"int*", !"int*"}
!22 = !{!"", !""}
!23 = !{!"a", !"b"}
!24 = !{i1 false, i1 false}
!25 = !{i32 0, i32 0}
!26 = !DILocation(line: 0, scope: !6)
!27 = !DILocation(line: 2, column: 16, scope: !6)
!28 = !DILocation(line: 3, column: 12, scope: !6)
!29 = !{!30, !30, i64 0}
!30 = !{!"int", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C/C++ TBAA"}
!33 = !DILocation(line: 3, column: 19, scope: !6)
!34 = !DILocation(line: 3, column: 3, scope: !6)
!35 = !DILocation(line: 3, column: 10, scope: !6)
!36 = !DILocation(line: 4, column: 1, scope: !6)
