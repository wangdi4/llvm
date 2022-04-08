target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test() #0 !dbg !6 !kernel_arg_addr_space !10 !kernel_arg_access_qual !10 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !10 !kernel_arg_name !10 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 {
entry:
  ret void, !dbg !11
}

attributes #0 = { convergent norecurse nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.compiler.options = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "/export/users.shared/wenjuhe/cnf/conformance-tests/exp/linux.x64/_build_cl/empty.cl", directory: "/export/users.shared/wenjuhe/cnf/conformance-tests/exp/linux.x64/_build_cl")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 2, i32 0}
!5 = !{!"-cl-std=CL2.0", !"-g"}
!6 = distinct !DISubprogram(name: "test", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !10)
!7 = !DIFile(filename: "empty.cl", directory: "/export/users.shared/wenjuhe/cnf/conformance-tests/exp/linux.x64/_build_cl")
!8 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !9)
!9 = !{null}
!10 = !{}
!11 = !DILocation(line: 2, column: 1, scope: !6)
