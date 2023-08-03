; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel with option "-g -cl-opt-disable":
; kernel void sample_kernel(read_only image3d_t input) {
;   get_image_width(input);
;   get_image_height(input);
;   get_image_depth(input);
;   int4 dim = get_image_dim(input);
;   get_image_channel_data_type(input);
;   get_image_channel_order(input);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local spir_kernel void @sample_kernel(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %input) #0 !dbg !6 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_name !15 !kernel_arg_host_accessible !16 !kernel_arg_pipe_depth !17 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @sample_kernel(ptr addrspace(1) %input)
; CHECK: %input.addr = alloca ptr addrspace(1), align 8
; CHECK: store ptr addrspace(1) %input, ptr %input.addr, align 8
; CHECK: call void @llvm.dbg.declare(metadata ptr %input.addr, metadata !{{[0-9]+}}, metadata !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef)), !dbg
; CHECK: load ptr addrspace(1), ptr %input.addr, align 8, !dbg
; CHECK: call spir_func i32 @_Z15get_image_width14ocl_image3d_ro(ptr addrspace(1)
; CHECK: call spir_func i32 @_Z16get_image_height14ocl_image3d_ro(ptr addrspace(1)
; CHECK: call spir_func i32 @_Z15get_image_depth14ocl_image3d_ro(ptr addrspace(1)
; CHECK: call spir_func <4 x i32> @_Z13get_image_dim14ocl_image3d_ro(ptr addrspace(1)
; CHECK: call spir_func i32 @_Z27get_image_channel_data_type14ocl_image3d_ro(ptr addrspace(1)
; CHECK: call spir_func i32 @_Z23get_image_channel_order14ocl_image3d_ro(ptr addrspace(1)

  %input.addr = alloca target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), align 8
  %dim = alloca <4 x i32>, align 16
  store target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %input, ptr %input.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %input.addr, metadata !18, metadata !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef)), !dbg !19
  %0 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !20
  %call = call spir_func i32 @_Z15get_image_width14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %0) #2, !dbg !21
  %1 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !22
  %call1 = call spir_func i32 @_Z16get_image_height14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %1) #2, !dbg !23
  %2 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !24
  %call2 = call spir_func i32 @_Z15get_image_depth14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %2) #2, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %dim, metadata !26, metadata !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef)), !dbg !33
  %3 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !34
  %call3 = call spir_func <4 x i32> @_Z13get_image_dim14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %3) #2, !dbg !35
  store <4 x i32> %call3, ptr %dim, align 16, !dbg !33
  %4 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !36
  %call4 = call spir_func i32 @_Z27get_image_channel_data_type14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %4) #2, !dbg !37
  %5 = load target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), ptr %input.addr, align 8, !dbg !38
  %call5 = call spir_func i32 @_Z23get_image_channel_order14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %5) #2, !dbg !39
  ret void, !dbg !40
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; CHECK-DAG: declare spir_func i32 @_Z15get_image_width14ocl_image3d_ro(ptr addrspace(1))
; CHECK-DAG: declare spir_func i32 @_Z16get_image_height14ocl_image3d_ro(ptr addrspace(1))
; CHECK-DAG: declare spir_func i32 @_Z15get_image_depth14ocl_image3d_ro(ptr addrspace(1))
; CHECK-DAG: declare spir_func <4 x i32> @_Z13get_image_dim14ocl_image3d_ro(ptr addrspace(1))
; CHECK-DAG: declare spir_func i32 @_Z27get_image_channel_data_type14ocl_image3d_ro(ptr addrspace(1))
; CHECK-DAG: declare spir_func i32 @_Z23get_image_channel_order14ocl_image3d_ro(ptr addrspace(1))

declare spir_func i32 @_Z15get_image_width14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

declare spir_func i32 @_Z16get_image_height14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

declare spir_func i32 @_Z15get_image_depth14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

declare spir_func <4 x i32> @_Z13get_image_dim14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

declare spir_func i32 @_Z27get_image_channel_data_type14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

declare spir_func i32 @_Z23get_image_channel_order14ocl_image3d_ro(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) #2

attributes #0 = { convergent noinline norecurse nounwind optnone }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { convergent nounwind willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.compiler.options = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "image-methods.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 3, i32 0}
!5 = !{}
!6 = distinct !DISubprogram(name: "sample_kernel", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !5)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "opencl_image3d_ro_t", file: !1, flags: DIFlagFwdDecl)
!11 = !{i32 1}
!12 = !{!"read_only"}
!13 = !{!"image3d_t"}
!14 = !{!""}
!15 = !{!"input"}
!16 = !{i1 false}
!17 = !{i32 0}
!18 = !DILocalVariable(name: "input", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!19 = !DILocation(line: 1, column: 47, scope: !6)
!20 = !DILocation(line: 2, column: 19, scope: !6)
!21 = !DILocation(line: 2, column: 3, scope: !6)
!22 = !DILocation(line: 3, column: 20, scope: !6)
!23 = !DILocation(line: 3, column: 3, scope: !6)
!24 = !DILocation(line: 4, column: 19, scope: !6)
!25 = !DILocation(line: 4, column: 3, scope: !6)
!26 = !DILocalVariable(name: "dim", scope: !6, file: !1, line: 6, type: !27)
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "int4", file: !28, line: 184, baseType: !29)
!28 = !DIFile(filename: "opencl-c-base.h", directory: "")
!29 = !DICompositeType(tag: DW_TAG_array_type, baseType: !30, size: 128, flags: DIFlagVector, elements: !31)
!30 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!31 = !{!32}
!32 = !DISubrange(count: 4)
!33 = !DILocation(line: 6, column: 8, scope: !6)
!34 = !DILocation(line: 6, column: 28, scope: !6)
!35 = !DILocation(line: 6, column: 14, scope: !6)
!36 = !DILocation(line: 8, column: 31, scope: !6)
!37 = !DILocation(line: 8, column: 3, scope: !6)
!38 = !DILocation(line: 9, column: 27, scope: !6)
!39 = !DILocation(line: 9, column: 3, scope: !6)
!40 = !DILocation(line: 10, column: 1, scope: !6)
