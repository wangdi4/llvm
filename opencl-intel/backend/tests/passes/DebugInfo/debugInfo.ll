; Compiled from:
; ----------------------------------------------------
; void f1(__local char* a) {
; }
;
; __kernel void mykernel() {
;   __local char x[100];
;   f1(x);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited -O0 -D__OPENCL_C_VERSION__=200 -finclude-default-header -disable-O0-optnone
; ----------------------------------------------------
; RUN: %oclopt -debug-info -S %s | FileCheck %s

; The test checks that debugInfo pass adds metadata to variables passed to dbg_declare_global.
; We have 2 functions, so the call should appear twice.
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

@mykernel.x = internal addrspace(3) global [100 x i8] undef, align 1, !dbg !0

; Function Attrs: convergent noinline nounwind
define spir_func void @f1(i8 addrspace(3)* %a) #0 !dbg !18 {
entry:
  %a.addr = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(3)* %a, i8 addrspace(3)** %a.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(3)** %a.addr, metadata !22, metadata !DIExpression()), !dbg !23
  ret void, !dbg !24
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent noinline nounwind
define spir_kernel void @mykernel() #2 !dbg !2 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 {
entry:
  call spir_func void @f1(i8 addrspace(3)* getelementptr inbounds ([100 x i8], [100 x i8] addrspace(3)* @mykernel.x, i32 0, i32 0)) #3, !dbg !25
  ret void, !dbg !26
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }

!llvm.dbg.cu = !{!6}
!llvm.module.flags = !{!14, !15}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!16}
!opencl.spir.version = !{!16}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "x", scope: !2, file: !3, line: 5, type: !10, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "mykernel", scope: !3, file: !3, line: 4, type: !4, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !6, retainedNodes: !8)
!3 = !DIFile(filename: "/tmp/1.cl", directory: "/tmp/tests2")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_C99, file: !7, producer: "clang version 7.0.0 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !8, globals: !9)
!7 = !DIFile(filename: "/tmp/<stdin>", directory: "/tmp/tests2")
!8 = !{}
!9 = !{!0}
!10 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 800, elements: !12)
!11 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!12 = !{!13}
!13 = !DISubrange(count: 100)
!14 = !{i32 2, !"Debug Info Version", i32 3}
!15 = !{i32 1, !"wchar_size", i32 4}
!16 = !{i32 2, i32 0}
!17 = !{!"clang version 7.0.0 "}
!18 = distinct !DISubprogram(name: "f1", scope: !3, file: !3, line: 1, type: !19, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !6, retainedNodes: !8)
!19 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !20)
!20 = !{null, !21}
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!22 = !DILocalVariable(name: "a", arg: 1, scope: !18, file: !3, line: 1, type: !21)
!23 = !DILocation(line: 1, scope: !18)
!24 = !DILocation(line: 2, scope: !18)
!25 = !DILocation(line: 6, scope: !2)
!26 = !DILocation(line: 7, scope: !2)
