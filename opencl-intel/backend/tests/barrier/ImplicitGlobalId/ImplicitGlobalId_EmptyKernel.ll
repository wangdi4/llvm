; Compiled from:
; ----------------------------------------------------
; __constant uchar c = 6;
; __constant int b = 5;
; __constant int d = 4;
; __kernel void fourth_kernel()
; {
;  return;
; }
;
; __kernel void third_kernel()
; {
;  fourth_kernel();
;  return;
; }
;
; __kernel void second_kernel()
; {
;  third_kernel();
;  return;
; }
;
; __kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
; {
;  second_kernel();
;  return;
; }
; ----------------------------------------------------
; Compiled using: fpga_ioc_x -input=file.cl -bo='-g'
; and the LLVM IR was dumped by breaking at intel::ImplicitGlobalIdPass::runOnModule()
; and calling M.dump();
; ----------------------------------------------------
; RUN: %oclopt -B-ImplicitGlobalIdPass -verify -S %s
;
; This test checks that we are able to insert a gid_alloca at an
; empty kernel (very degenerative case), i.e. fourth_kernel, the
; kernel receives no parameters, doesn't return value and has no
; instructions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@c = addrspace(2) constant i8 6, align 1, !dbg !0
@b = addrspace(2) constant i32 5, align 4, !dbg !6
@d = addrspace(2) constant i32 4, align 4, !dbg !10

; Function Attrs: convergent noinline nounwind
define void @fourth_kernel() #0 !dbg !23 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 !kernel_execution_length !27 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !28
}

; Function Attrs: convergent noinline nounwind
define void @third_kernel() #0 !dbg !29 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 !kernel_execution_length !30 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  call void @__internal.fourth_kernel() #3, !dbg !31
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !32
}

; Function Attrs: convergent noinline nounwind
define void @second_kernel() #0 !dbg !33 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 !kernel_execution_length !30 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  call void @__internal.third_kernel() #3, !dbg !34
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !35
}

; Function Attrs: convergent noinline nounwind
define void @main_kernel(i8 addrspace(1)* %buf_in, i8 addrspace(1)* %buf_out) #0 !dbg !36 !kernel_arg_addr_space !40 !kernel_arg_access_qual !41 !kernel_arg_type !42 !kernel_arg_base_type !42 !kernel_arg_type_qual !43 !kernel_arg_host_accessible !44 !kernel_arg_pipe_depth !45 !kernel_arg_pipe_io !43 !kernel_arg_buffer_location !43 !kernel_arg_name !46 !use_fpga_pipes !26 !kernel_execution_length !47 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  %buf_in.addr = alloca i8 addrspace(1)*, align 8
  %buf_out.addr = alloca i8 addrspace(1)*, align 8
  store i8 addrspace(1)* %buf_in, i8 addrspace(1)** %buf_in.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(1)** %buf_in.addr, metadata !48, metadata !DIExpression()), !dbg !49
  store i8 addrspace(1)* %buf_out, i8 addrspace(1)** %buf_out.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(1)** %buf_out.addr, metadata !50, metadata !DIExpression()), !dbg !51
  call void @__internal.second_kernel() #3, !dbg !52
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !53
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent noinline nounwind
define void @__internal.fourth_kernel() #0 !dbg !54 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 {
entry:
  ret void, !dbg !55
}

; Function Attrs: convergent noinline nounwind
define void @__internal.third_kernel() #0 !dbg !56 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 {
entry:
  call void @__internal.fourth_kernel() #3, !dbg !57
  ret void, !dbg !58
}

; Function Attrs: convergent noinline nounwind
define void @__internal.second_kernel() #0 !dbg !59 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !4 !use_fpga_pipes !26 {
entry:
  call void @__internal.third_kernel() #3, !dbg !60
  ret void, !dbg !61
}

declare void @dummybarrier.()

; Function Attrs: noduplicate
declare void @_Z7barrierj(i32) #2

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp
-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noduplicate }
attributes #3 = { convergent "uniform-work-group-size"="true" }

!llvm.dbg.cu = !{!2}
!llvm.linker.options = !{}
!llvm.module.flags = !{!15, !16, !17}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!18}
!opencl.spir.version = !{!18}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!19}
!llvm.ident = !{!20}
!opencl.kernels = !{!21}
!opencl.global_variable_total_size = !{!22}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !8, line: 1, type: !12, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 8.0.0 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "/tmp/tests/<stdin>", directory: "/tmp/tests")
!4 = !{}
!5 = !{!0, !6, !10}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !8, line: 2, type: !9, isLocal: false, isDefinition: true)
!8 = !DIFile(filename: "1", directory: "/tmp/tests")
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !DIGlobalVariableExpression(var: !11, expr: !DIExpression())
!11 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !8, line: 3, type: !9, isLocal: false, isDefinition: true)
!12 = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar", file: !13, line: 28, baseType: !14)
!13 = !DIFile(filename: "opencl-c-common.h", directory: "/tmp/tests")
!14 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!15 = !{i32 2, !"Dwarf Version", i32 4}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, !"wchar_size", i32 4}
!18 = !{i32 1, i32 2}
!19 = !{!"-g", !"-cl-opt-disable"}
!20 = !{!"clang version 8.0.0 "}
!21 = !{void ()* @fourth_kernel, void ()* @third_kernel, void ()* @second_kernel, void (i8 addrspace(1)*, i8 addrspace(1)*)* @main_kernel}
!22 = !{i32 0}
!23 = distinct !DISubprogram(name: "fourth_kernel", scope: !8, file: !8, line: 4, type: !24, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!24 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !25)
!25 = !{null}
!26 = !{i1 false}
!27 = !{i32 1}
!28 = !DILocation(line: 6, column: 2, scope: !23)
!29 = distinct !DISubprogram(name: "third_kernel", scope: !8, file: !8, line: 9, type: !24, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!30 = !{i32 2}
!31 = !DILocation(line: 11, column: 2, scope: !29)
!32 = !DILocation(line: 12, column: 2, scope: !29)
!33 = distinct !DISubprogram(name: "second_kernel", scope: !8, file: !8, line: 15, type: !24, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!34 = !DILocation(line: 17, column: 2, scope: !33)
!35 = !DILocation(line: 18, column: 2, scope: !33)
!36 = distinct !DISubprogram(name: "main_kernel", scope: !8, file: !8, line: 21, type: !37, isLocal: false, isDefinition: true, scopeLine: 22, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!37 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !38)
!38 = !{null, !39, !39}
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!40 = !{i32 1, i32 1}
!41 = !{!"none", !"none"}
!42 = !{!"uchar*", !"uchar*"}
!43 = !{!"", !""}
!44 = !{i1 false, i1 false}
!45 = !{i32 0, i32 0}
!46 = !{!"buf_in", !"buf_out"}
!47 = !{i32 8}
!48 = !DILocalVariable(name: "buf_in", arg: 1, scope: !36, file: !8, line: 21, type: !39)
!49 = !DILocation(line: 21, column: 43, scope: !36)
!50 = !DILocalVariable(name: "buf_out", arg: 2, scope: !36, file: !8, line: 21, type: !39)
!51 = !DILocation(line: 21, column: 67, scope: !36)
!52 = !DILocation(line: 23, column: 2, scope: !36)
!53 = !DILocation(line: 24, column: 2, scope: !36)
!54 = distinct !DISubprogram(name: "fourth_kernel", scope: !8, file: !8, line: 4, type: !24, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!55 = !DILocation(line: 6, column: 2, scope: !54)
!56 = distinct !DISubprogram(name: "third_kernel", scope: !8, file: !8, line: 9, type: !24, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!57 = !DILocation(line: 11, column: 2, scope: !56)
!58 = !DILocation(line: 12, column: 2, scope: !56)
!59 = distinct !DISubprogram(name: "second_kernel", scope: !8, file: !8, line: 15, type: !24, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !2, retainedNodes: !4)
!60 = !DILocation(line: 17, column: 2, scope: !59)
!61 = !DILocation(line: 18, column: 2, scope: !59)

