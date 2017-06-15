; RUN: opt -add-implicit-args -verify -S %s | FileCheck %s
; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -verify -S %s | FileCheck %s

;; This test was generated using the following cl code with this command:
;;  clang.exe -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited  -O0 -D__OPENCL_C_VERSION__=200 -o - ker.cl -I llvm\install\include\cclang -include opencl-c.h -include opencl-c-intel.h
;;
;;==========================================
;;int func(__generic int* pInt)
;;{
;;    return *pInt;
;;}
;;__kernel void invoke_func()
;;{
;;    int integer = 42;
;;    integer = func(&integer);
;;}
;;==========================================
;;
; The test checks that GenericAddressStaticResolution pass copies debug info along with resolved func.
;
; CHECK: @func{{.*}} !dbg !4
; CHECK: @invoke_func{{.*}} !dbg !10

; ModuleID = '/tmp/ker.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_func i32 @func(i32 addrspace(4)* %pInt) #0 !dbg !4 {
entry:
  %pInt.addr = alloca i32 addrspace(4)*, align 8
  store i32 addrspace(4)* %pInt, i32 addrspace(4)** %pInt.addr, align 8
  call void @llvm.dbg.declare(metadata i32 addrspace(4)** %pInt.addr, metadata !23, metadata !24), !dbg !25
  %0 = load i32 addrspace(4)*, i32 addrspace(4)** %pInt.addr, align 8, !dbg !26
  %1 = load i32, i32 addrspace(4)* %0, align 4, !dbg !26
  ret i32 %1, !dbg !26
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
define spir_kernel void @invoke_func() #0 !dbg !10 {
entry:
  %integer = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %integer, metadata !27, metadata !24), !dbg !28
  store i32 42, i32* %integer, align 4, !dbg !28
  %0 = addrspacecast i32* %integer to i32 addrspace(4)*, !dbg !29
  %call = call spir_func i32 @func(i32 addrspace(4)* %0), !dbg !29
  store i32 %call, i32* %integer, align 4, !dbg !29
  ret void, !dbg !30
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!13}
!llvm.module.flags = !{!19, !20}
!opencl.enable.FP_CONTRACT = !{}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!21}
!opencl.spir.version = !{!22, !22}
!opencl.ocl.version = !{!22, !22}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.8.1", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2)
!1 = !DIFile(filename: "/tmp/<stdin>", directory: "/tmp")
!2 = !{}
!3 = !{!4, !10}
!4 = distinct !DISubprogram(name: "func", scope: !5, file: !5, line: 1, type: !6, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!5 = !DIFile(filename: "/tmp/ker.cl", directory: "/tmp")
!6 = !DISubroutineType(types: !7)
!7 = !{!8, !9}
!8 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !8, size: 64, align: 64)
!10 = distinct !DISubprogram(name: "invoke_func", scope: !5, file: !5, line: 5, type: !11, isLocal: false, isDefinition: true, scopeLine: 6, isOptimized: false, unit: !0, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{null}
!13 = !{void ()* @invoke_func, !14, !15, !16, !17, !18}
!14 = !{!"kernel_arg_addr_space"}
!15 = !{!"kernel_arg_access_qual"}
!16 = !{!"kernel_arg_type"}
!17 = !{!"kernel_arg_base_type"}
!18 = !{!"kernel_arg_type_qual"}
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{!"clang version 3.8.1"}
!22 = !{i32 2, i32 0}
!23 = !DILocalVariable(name: "pInt", arg: 1, scope: !4, file: !5, line: 1, type: !9)
!24 = !DIExpression()
!25 = !DILocation(line: 1, scope: !4)
!26 = !DILocation(line: 3, scope: !4)
!27 = !DILocalVariable(name: "integer", scope: !10, file: !5, line: 7, type: !8)
!28 = !DILocation(line: 7, scope: !10)
!29 = !DILocation(line: 8, scope: !10)
!30 = !DILocation(line: 9, scope: !10)
