target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind optnone noinline
define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %a, ptr addrspace(1) noundef align 4 %b) #0 !dbg !7 !kernel_arg_addr_space !20 !kernel_arg_access_qual !21 !kernel_arg_type !22 !kernel_arg_base_type !22 !kernel_arg_type_qual !23 !kernel_arg_name !24 !kernel_arg_host_accessible !25 !kernel_arg_pipe_depth !26 !kernel_arg_pipe_io !23 !kernel_arg_buffer_location !23 {
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %a, metadata !14, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata ptr addrspace(1) %b, metadata !15, metadata !DIExpression()), !dbg !27
  %call = call spir_func i64 @_Z13get_global_idj(i32 noundef 0) #3, !dbg !28
  call void @llvm.dbg.value(metadata i64 %call, metadata !16, metadata !DIExpression()), !dbg !27
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call, !dbg !29
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !dbg !29, !tbaa !30
  %mul = shl nsw i32 %0, 1, !dbg !34
  %arrayidx1 = getelementptr inbounds i32, ptr addrspace(1) %b, i64 %call, !dbg !35
  store i32 %mul, ptr addrspace(1) %arrayidx1, align 4, !dbg !36, !tbaa !30
  ret void, !dbg !37
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
!spirv.Source = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 2, i32 0}
!5 = !{!"-cl-std=CL2.0", !"-g"}
!6 = !{i32 4, i32 100000}
!7 = distinct !DISubprogram(name: "test", scope: !8, file: !8, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!8 = !DIFile(filename: "add.cl", directory: "")
!9 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !10)
!10 = !{null, !11, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15, !16}
!14 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !8, line: 1, type: !11)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !8, line: 1, type: !11)
!16 = !DILocalVariable(name: "gid", scope: !7, file: !8, line: 2, type: !17)
!17 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !18, line: 116, baseType: !19)
!18 = !DIFile(filename: "opencl-c-base.h", directory: "")
!19 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!20 = !{i32 1, i32 1}
!21 = !{!"none", !"none"}
!22 = !{!"int*", !"int*"}
!23 = !{!"", !""}
!24 = !{!"a", !"b"}
!25 = !{i1 false, i1 false}
!26 = !{i32 0, i32 0}
!27 = !DILocation(line: 0, scope: !7)
!28 = !DILocation(line: 2, column: 16, scope: !7)
!29 = !DILocation(line: 3, column: 12, scope: !7)
!30 = !{!31, !31, i64 0}
!31 = !{!"int", !32, i64 0}
!32 = !{!"omnipotent char", !33, i64 0}
!33 = !{!"Simple C/C++ TBAA"}
!34 = !DILocation(line: 3, column: 19, scope: !7)
!35 = !DILocation(line: 3, column: 3, scope: !7)
!36 = !DILocation(line: 3, column: 10, scope: !7)
!37 = !DILocation(line: 4, column: 1, scope: !7)
