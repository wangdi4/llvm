; Compiled from:
; ----------------------------------------------------
; int func(__generic int* pInt)
; {
;     return *pInt;
; }
; __kernel void invoke_func()
; {
;     int integer = 42;
;     integer = func(&integer);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited -O0 -D__OPENCL_C_VERSION__=200 -finclude-default-header -disable-O0-optnone
; Optimizer options:
;   %oclopt -spir-materializer -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -generic-addr-static-resolution -verify -S %s | FileCheck %s

; The test checks that GenericAddressStaticResolution pass copies debug info along with resolved func.
;
; CHECK-DAG: [[SRCDIPL:.*]] = distinct !DISubprogram([[SRCDI:name: "func".*]])
; CHECK-DAG: [[CLONDIPL:.*]] = distinct !DISubprogram([[SRCDI]])
; CHECK-DAG: !DILocation({{.*}} scope: [[SRCDIPL]])
; CHECK-DAG: !DILocation({{.*}} scope: [[CLONDIPL]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent noinline nounwind
define i32 @func(i32 addrspace(4)* %pInt) #0 !dbg !8 {
entry:
  %pInt.addr = alloca i32 addrspace(4)*, align 8
  store i32 addrspace(4)* %pInt, i32 addrspace(4)** %pInt.addr, align 8
  call void @llvm.dbg.declare(metadata i32 addrspace(4)** %pInt.addr, metadata !14, metadata !DIExpression()), !dbg !15
  %0 = load i32 addrspace(4)*, i32 addrspace(4)** %pInt.addr, align 8, !dbg !16
  %1 = load i32, i32 addrspace(4)* %0, align 4, !dbg !16
  ret i32 %1, !dbg !16
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent noinline nounwind
define void @invoke_func() #2 !dbg !17 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 {
entry:
  %integer = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %integer, metadata !20, metadata !DIExpression()), !dbg !21
  store i32 42, i32* %integer, align 4, !dbg !21
  %0 = addrspacecast i32* %integer to i32 addrspace(4)*, !dbg !22
  %call = call i32 @func(i32 addrspace(4)* %0) #3, !dbg !22
  store i32 %call, i32* %integer, align 4, !dbg !22
  ret void, !dbg !23
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "/tmp/<stdin>", directory: "/tmp/tests2")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 2, i32 0}
!6 = !{!"clang version 7.0.0 "}
!7 = !{void ()* @invoke_func}
!8 = distinct !DISubprogram(name: "func", scope: !9, file: !9, line: 1, type: !10, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
!9 = !DIFile(filename: "/tmp/1.cl", directory: "/tmp/tests2")
!10 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !11)
!11 = !{!12, !13}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!14 = !DILocalVariable(name: "pInt", arg: 1, scope: !8, file: !9, line: 1, type: !13)
!15 = !DILocation(line: 1, scope: !8)
!16 = !DILocation(line: 3, scope: !8)
!17 = distinct !DISubprogram(name: "invoke_func", scope: !9, file: !9, line: 5, type: !18, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
!18 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !19)
!19 = !{null}
!20 = !DILocalVariable(name: "integer", scope: !17, file: !9, line: 7, type: !12)
!21 = !DILocation(line: 7, scope: !17)
!22 = !DILocation(line: 8, scope: !17)
!23 = !DILocation(line: 9, scope: !17)
