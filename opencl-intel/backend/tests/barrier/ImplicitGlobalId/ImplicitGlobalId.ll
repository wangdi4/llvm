; Compiled from:
; ----------------------------------------------------
;__kernel void foo()
;{
;}
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown -debug-info-kind=limited -O2 -disable-llvm-passes -x cl
; ----------------------------------------------------
; RUN: %oclopt -B-ImplicitGlobalIdPass -verify -S %s | FileCheck %s
;

; This test checks that we have a gid_alloca and a corresponding llvm.dbg.declare
; (with correct metadata, e.g. DILocation)

; CHECK: define {{.*}} void @foo
; CHECK: %__ocl_dbg_gid0 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0{{.*}}
; CHECK: %__ocl_dbg_gid1 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1{{.*}}
; CHECK: %__ocl_dbg_gid2 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2{{.*}}
; CHECK: ret void
; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid0", {{.*}}
; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid1", {{.*}}
; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid2", {{.*}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #0 !dbg !8 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 {
entry:
  ret void, !dbg !12
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!6}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "/tmp/<stdin>", directory: "/data/xmain/ics-ws/xmain/llvm/projects/opencl")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 1, i32 0}
!6 = !{i32 1, i32 2}
!7 = !{!"clang version 8.0.0 "}
!8 = distinct !DISubprogram(name: "foo", scope: !9, file: !9, line: 1, type: !10, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !2)
!9 = !DIFile(filename: "/tmp/1.cl", directory: "/data/xmain/ics-ws/xmain/llvm/projects/opencl")
!10 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !11)
!11 = !{null}
!12 = !DILocation(line: 3, scope: !8)
