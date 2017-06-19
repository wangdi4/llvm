; RUN: opt -debug-info -S %s | FileCheck %s
; XFAIL: *
;
;; This test was generated using the following cl code with this command:
;;
;; clang -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited  -O0 -D__OPENCL_C_VERSION__=200 -include opencl-c.h -include opencl-c-intel.h debugInfo.cl
;;
;;void f1(__local char* a) {
;;}
;;
;;__kernel void mykernel() {
;;  __local char x[100];
;;  f1(x);
;;}

; The test checks that debugInfo pass adds metadata to variables passed to dbg_declare_global.
; We have 2 functions, so the call should appear twice.
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]

; ModuleID = '../debugInfo.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

@mykernel.x = internal addrspace(3) global [100 x i8] undef, align 1

; Function Attrs: nounwind
define spir_func void @f1(i8 addrspace(3)* %a) #0 !dbg !4 {
entry:
  %a.addr = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(3)* %a, i8 addrspace(3)** %a.addr, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(3)** %a.addr, metadata !28, metadata !29), !dbg !30
  ret void, !dbg !31
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
define spir_kernel void @mykernel() #0 !dbg !10 {
entry:
  call spir_func void @f1(i8 addrspace(3)* getelementptr inbounds ([100 x i8], [100 x i8] addrspace(3)* @mykernel.x, i32 0, i32 0)), !dbg !32
  ret void, !dbg !33
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!18}
!llvm.module.flags = !{!24, !25}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!26}
!opencl.spir.version = !{!26}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!27}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.8.1 ", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, globals: !13)
!1 = !DIFile(filename: "../<stdin>", directory: "/home/chbessonova/repos/src")
!2 = !{}
!3 = !{!4, !10}
!4 = distinct !DISubprogram(name: "f1", scope: !5, file: !5, line: 1, type: !6, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!5 = !DIFile(filename: "../debugInfo.cl", directory: "/home/chbessonova/repos/src")
!6 = !DISubroutineType(types: !7)
!7 = !{null, !8}
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64)
!9 = !DIBasicType(name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!10 = distinct !DISubprogram(name: "mykernel", scope: !5, file: !5, line: 4, type: !11, isLocal: false, isDefinition: true, scopeLine: 4, isOptimized: false, unit: !0, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{null}
!13 = !{!14}
!14 = !DIGlobalVariable(name: "x", scope: !10, file: !5, line: 5, type: !15, isLocal: true, isDefinition: true, variable: [100 x i8] addrspace(3)* @mykernel.x)
!15 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 800, align: 8, elements: !16)
!16 = !{!17}
!17 = !DISubrange(count: 100)
!18 = !{void ()* @mykernel, !19, !20, !21, !22, !23}
!19 = !{!"kernel_arg_addr_space"}
!20 = !{!"kernel_arg_access_qual"}
!21 = !{!"kernel_arg_type"}
!22 = !{!"kernel_arg_base_type"}
!23 = !{!"kernel_arg_type_qual"}
!24 = !{i32 2, !"Dwarf Version", i32 4}
!25 = !{i32 2, !"Debug Info Version", i32 3}
!26 = !{i32 2, i32 0}
!27 = !{!"clang version 3.8.1 "}
!28 = !DILocalVariable(name: "a", arg: 1, scope: !4, file: !5, line: 1, type: !8)
!29 = !DIExpression()
!30 = !DILocation(line: 1, scope: !4)
!31 = !DILocation(line: 2, scope: !4)
!32 = !DILocation(line: 6, scope: !10)
!33 = !DILocation(line: 7, scope: !10)
