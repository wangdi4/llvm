; RUN: %oclopt -B-ImplicitGlobalIdPass -S %s
;
;; This test checks that we are able to insert a gid_alloca at an
;; empty kernel (very degenerative case), i.e. fourth_kernel, the
;; kernel receives no parameters, doesn't return value and has no
;; instructions.
;;
;; This test was generated using the following cl code:
;; __constant uchar c = 6;
;; __constant int b = 5;
;; __constant int d = 4;
;; __kernel void fourth_kernel()
;; {
;;  return;
;; }
;;
;; __kernel void third_kernel()
;; {
;;  fourth_kernel();
;;  return;
;; }
;;
;; __kernel void second_kernel()
;; {
;;  third_kernel();
;;  return;
;; }
;;
;; __kernel void main_kernel(__global uchar* buf_in, __global uchar* buf_out)
;; {
;;  second_kernel();
;;  return;
;; }
;;
;; and the LLVM IR was dumped by breaking at intel::ImplicitGlobalIdPass::runOnModule()
;; and calling M.dump();
;;
;;
;;
; ModuleID = 'main'
source_filename = "/home/ocohen11/.ocl_cpu_rt/ocl_cpu_rt/install/RH64/Debug/tests/debugger_test_type/cl_kernels/several_kernels_only_global_variables.cl"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@c = addrspace(2) constant i8 6, align 1, !dbg !0
@b = addrspace(2) constant i32 5, align 4, !dbg !6
@d = addrspace(2) constant i32 4, align 4, !dbg !9

; Function Attrs: noinline nounwind
define void @fourth_kernel() #0 !dbg !22 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 !kernel_execution_length !25 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !27
}

; Function Attrs: noinline nounwind
define void @third_kernel() #0 !dbg !28 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 !kernel_execution_length !29 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  call void @__internal.fourth_kernel(), !dbg !30
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !31
}

; Function Attrs: noinline nounwind
define void @second_kernel() #0 !dbg !32 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 !kernel_execution_length !29 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  call void @__internal.third_kernel(), !dbg !33
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !34
}

; Function Attrs: noinline nounwind
define void @main_kernel(i8 addrspace(1)* %buf_in, i8 addrspace(1)* %buf_out) #0 !dbg !35 !kernel_arg_addr_space !39 !kernel_arg_access_qual !40 !kernel_arg_type !41 !kernel_arg_base_type !41 !kernel_arg_type_qual !42 !kernel_arg_name !43 !kernel_execution_length !44 !kernel_has_barrier !26 !kernel_has_global_sync !26 {
entry:
  call void @dummybarrier.()
  %buf_in.addr = alloca i8 addrspace(1)*, align 8
  %buf_out.addr = alloca i8 addrspace(1)*, align 8
  store i8 addrspace(1)* %buf_in, i8 addrspace(1)** %buf_in.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(1)** %buf_in.addr, metadata !45, metadata !46), !dbg !47
  store i8 addrspace(1)* %buf_out, i8 addrspace(1)** %buf_out.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(1)** %buf_out.addr, metadata !48, metadata !46), !dbg !49
  call void @__internal.second_kernel(), !dbg !50
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !51
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define void @__internal.fourth_kernel() #0 !dbg !52 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 {
entry:
  ret void, !dbg !53
}

; Function Attrs: noinline nounwind
define void @__internal.third_kernel() #0 !dbg !54 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 {
entry:
  call void @__internal.fourth_kernel(), !dbg !55
  ret void, !dbg !56
}

; Function Attrs: noinline nounwind
define void @__internal.second_kernel() #0 !dbg !57 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 {
entry:
  call void @__internal.third_kernel(), !dbg !58
  ret void, !dbg !59
}

declare void @dummybarrier.()

; Function Attrs: noduplicate
declare void @_Z7barrierj(i32) #2

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noduplicate }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!17}
!opencl.spir.version = !{!17}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!18}
!llvm.ident = !{!19}
!opencl.kernels = !{!20}
!opencl.global_variable_total_size = !{!21}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 1, type: !11, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "/home/ocohen11/.ocl_cpu_rt/ocl_cpu_rt/install/RH64/Debug/tests/debugger_test_type/cl_kernels/several_kernels_only_global_variables.cl", directory: "/home/ocohen11/.ocl_cpu_rt/ocl_cpu_rt/install/RH64/Debug/tests/debugger_test_type")
!4 = !{}
!5 = !{!0, !6, !9}
!6 = !DIGlobalVariableExpression(var: !7)
!7 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DIGlobalVariableExpression(var: !10)
!10 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar", file: !12, line: 31, baseType: !13)
!12 = !DIFile(filename: "opencl-c-common.h", directory: "/home/ocohen11/.ocl_cpu_rt/ocl_cpu_rt/install/RH64/Debug/tests/debugger_test_type")
!13 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!14 = !{i32 6, !"Linker Options", !4}
!15 = !{i32 2, !"Dwarf Version", i32 4}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, i32 2}
!18 = !{!"-g", !"-cl-opt-disable"}
!19 = !{!"clang version 4.0.1 "}
!20 = !{void ()* @fourth_kernel, void ()* @third_kernel, void ()* @second_kernel, void (i8 addrspace(1)*, i8 addrspace(1)*)* @main_kernel}
!21 = !{i32 0}
!22 = distinct !DISubprogram(name: "fourth_kernel", scope: !3, file: !3, line: 4, type: !23, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: false, unit: !2, variables: !4)
!23 = !DISubroutineType(types: !24)
!24 = !{null}
!25 = !{i32 1}
!26 = !{i1 false}
!27 = !DILocation(line: 6, column: 2, scope: !22)
!28 = distinct !DISubprogram(name: "third_kernel", scope: !3, file: !3, line: 9, type: !23, isLocal: false, isDefinition: true, scopeLine: 10, isOptimized: false, unit: !2, variables: !4)
!29 = !{i32 2}
!30 = !DILocation(line: 11, column: 2, scope: !28)
!31 = !DILocation(line: 12, column: 2, scope: !28)
!32 = distinct !DISubprogram(name: "second_kernel", scope: !3, file: !3, line: 15, type: !23, isLocal: false, isDefinition: true, scopeLine: 16, isOptimized: false, unit: !2, variables: !4)
!33 = !DILocation(line: 17, column: 2, scope: !32)
!34 = !DILocation(line: 18, column: 2, scope: !32)
!35 = distinct !DISubprogram(name: "main_kernel", scope: !3, file: !3, line: 21, type: !36, isLocal: false, isDefinition: true, scopeLine: 22, flags: DIFlagPrototyped, isOptimized: false, unit: !2, variables: !4)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !38, !38}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!39 = !{i32 1, i32 1}
!40 = !{!"none", !"none"}
!41 = !{!"uchar*", !"uchar*"}
!42 = !{!"", !""}
!43 = !{!"buf_in", !"buf_out"}
!44 = !{i32 8}
!45 = !DILocalVariable(name: "buf_in", arg: 1, scope: !35, file: !3, line: 21, type: !38)
!46 = !DIExpression()
!47 = !DILocation(line: 21, column: 43, scope: !35)
!48 = !DILocalVariable(name: "buf_out", arg: 2, scope: !35, file: !3, line: 21, type: !38)
!49 = !DILocation(line: 21, column: 67, scope: !35)
!50 = !DILocation(line: 23, column: 2, scope: !35)
!51 = !DILocation(line: 24, column: 2, scope: !35)
!52 = distinct !DISubprogram(name: "fourth_kernel", scope: !3, file: !3, line: 4, type: !23, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: false, unit: !2, variables: !4)
!53 = !DILocation(line: 6, column: 2, scope: !52)
!54 = distinct !DISubprogram(name: "third_kernel", scope: !3, file: !3, line: 9, type: !23, isLocal: false, isDefinition: true, scopeLine: 10, isOptimized: false, unit: !2, variables: !4)
!55 = !DILocation(line: 11, column: 2, scope: !54)
!56 = !DILocation(line: 12, column: 2, scope: !54)
!57 = distinct !DISubprogram(name: "second_kernel", scope: !3, file: !3, line: 15, type: !23, isLocal: false, isDefinition: true, scopeLine: 16, isOptimized: false, unit: !2, variables: !4)
!58 = !DILocation(line: 17, column: 2, scope: !57)
!59 = !DILocation(line: 18, column: 2, scope: !57)

