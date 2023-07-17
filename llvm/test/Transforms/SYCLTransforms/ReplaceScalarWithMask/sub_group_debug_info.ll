; Check that debug info is correctly handled during inlining masked kernel to
; scalar kernel. IR is dump before ReplaceScalarWithMask pass from kernel:
;   #pragma OPENCL EXTENSION cl_intel_subgroups: enable
;   kernel void test(global int* src) {
;     src[get_global_id(0)] = get_sub_group_size();
;   }
;
; RUN: opt -passes=sycl-kernel-replace-scalar-with-mask -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-replace-scalar-with-mask -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(ptr addrspace(1) noalias %src) local_unnamed_addr #0 !dbg !9 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_host_accessible !21 !kernel_arg_pipe_depth !22 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !kernel_arg_name !23 !vectorized_kernel !24 !vectorized_masked_kernel !25 !kernel_has_sub_groups !26 !vectorized_width !17 !scalar_kernel !27 !kernel_execution_length !28 !no_barrier_path !21 !kernel_has_global_sync !21 !arg_type_null_val !57 {
; CHECK: call i64 @_Z13get_global_idj(i32 0) #[[Attr:[0-9]+]], !dbg ![[DILoc:[0-9]+]]
entry:
  call void @llvm.dbg.value(metadata ptr addrspace(1) %src, metadata !16, metadata !DIExpression()), !dbg !29
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %0
  %subgroup.size = zext i1 %2 to i32
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !30
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !31
  store i32 %subgroup.size, ptr addrspace(1) %ptridx, align 4, !dbg !32, !tbaa !33
  ret void, !dbg !37
}

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_test(ptr addrspace(1) noalias %src) local_unnamed_addr #0 !dbg !38 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_host_accessible !21 !kernel_arg_pipe_depth !22 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !kernel_arg_name !23 !vectorized_kernel !27 !kernel_has_sub_groups !26 !recommended_vector_length !41 !vectorized_width !41 !vectorization_dimension !22 !scalar_kernel !8 !can_unite_workgroups !21 !kernel_execution_length !42 !no_barrier_path !21 !kernel_has_global_sync !21 {
entry:
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !43
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert2 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !44
  %broadcast.splat3 = shufflevector <16 x i32> %broadcast.splatinsert2, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !44
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !45
  store <16 x i32> %broadcast.splat3, ptr addrspace(1) %scalar.gep, align 4, !dbg !46
  ret void, !dbg !47
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeM16u_test(ptr addrspace(1) noalias %src, <16 x i32> %mask) local_unnamed_addr #0 !dbg !48 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_host_accessible !21 !kernel_arg_pipe_depth !22 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !kernel_arg_name !23 !vectorized_kernel !27 !kernel_has_sub_groups !26 !recommended_vector_length !41 !vectorized_width !41 !vectorization_dimension !22 !scalar_kernel !8 !can_unite_workgroups !21 !kernel_execution_length !51 !no_barrier_path !21 !kernel_has_global_sync !21 {
entry:
  %vec.mask = alloca <16 x i32>, align 64
  store <16 x i32> %mask, ptr %vec.mask, align 64
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !52
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert3 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !53
  %broadcast.splat4 = shufflevector <16 x i32> %broadcast.splatinsert3, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !53
  %5 = icmp ne <16 x i32> %mask, zeroinitializer
  %scalar.gep2 = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !54
  call void @llvm.masked.store.v16i32.p1(<16 x i32> %broadcast.splat4, ptr addrspace(1) %scalar.gep2, i32 4, <16 x i1> %5), !dbg !55
  ret void, !dbg !56
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.masked.store.v16i32.p1(<16 x i32>, ptr addrspace(1), i32 immarg, <16 x i1>) #5

declare i64 @_Z14get_local_sizej(i32)

declare i64 @_Z12get_local_idj(i32)

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable willreturn }
attributes #4 = { nounwind }
attributes #5 = { argmemonly nounwind willreturn }
attributes #6 = { convergent nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!sycl.kernels = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "subgroup.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{ptr @test}
!9 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 2, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !15)
!10 = !DIFile(filename: "subgroup.cl", directory: "")
!11 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !12)
!12 = !{null, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{!16}
!16 = !DILocalVariable(name: "src", arg: 1, scope: !9, file: !10, line: 2, type: !13)
!17 = !{i32 1}
!18 = !{!"none"}
!19 = !{!"int*"}
!20 = !{!""}
!21 = !{i1 false}
!22 = !{i32 0}
!23 = !{!"src"}
!24 = !{ptr @_ZGVeN16u_test}
!25 = !{ptr @_ZGVeM16u_test}
!26 = !{i1 true}
!27 = !{null}
!28 = !{i32 9}
!29 = !DILocation(line: 0, scope: !9)
!30 = !DILocation(line: 4, column: 9, scope: !9)
!31 = !DILocation(line: 4, column: 5, scope: !9)
!32 = !DILocation(line: 4, column: 27, scope: !9)
!33 = !{!34, !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !DILocation(line: 5, column: 1, scope: !9)
!38 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 2, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !39)
!39 = !{!40}
!40 = !DILocalVariable(name: "src", arg: 1, scope: !38, file: !10, line: 2, type: !13)
!41 = !{i32 16}
!42 = !{i32 14}
!43 = !DILocation(line: 4, column: 9, scope: !38)
!44 = !DILocation(line: 0, scope: !38)
!45 = !DILocation(line: 4, column: 5, scope: !38)
!46 = !DILocation(line: 4, column: 27, scope: !38)
!47 = !DILocation(line: 5, column: 1, scope: !38)
!48 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 2, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !49)
!49 = !{!50}
!50 = !DILocalVariable(name: "src", arg: 1, scope: !48, file: !10, line: 2, type: !13)
!51 = !{i32 17}
!52 = !DILocation(line: 4, column: 9, scope: !48)
!53 = !DILocation(line: 0, scope: !48)
!54 = !DILocation(line: 4, column: 5, scope: !48)
!55 = !DILocation(line: 4, column: 27, scope: !48)
!56 = !DILocation(line: 5, column: 1, scope: !48)
!57 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
