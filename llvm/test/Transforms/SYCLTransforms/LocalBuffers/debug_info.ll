; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

; transfer debug info from global variables to pLocalMemBase

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; Original global variables are removed
; CHECK-NOT: @a.__local = internal addrspace(3) global
; CHECK-NOT: @b.__local = internal addrspace(3) global
; CHECK-NOT: @a.clone.__local = internal addrspace(3) global
; CHECK-NOT: @b.clone.__local = internal addrspace(3) global

@a.__local = internal addrspace(3) global [4 x i32] undef, align 4, !dbg !0
@b.__local = internal addrspace(3) global [4 x i64] undef, align 8, !dbg !17
@a.clone.__local = internal addrspace(3) global [4 x i32] undef, align 4, !dbg !0
@b.clone.__local = internal addrspace(3) global [4 x i64] undef, align 8, !dbg !17

; CHECK-LABEL: define void @main_kernel
define void @main_kernel() !dbg !2 {
entry:
; DebugInfo of @a.__local is transferred to pLocalMemBase, with offset 0 (omitted)
; CHECK: [[GEP0:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0, !dbg
; CHECK-NEXT: call void @llvm.dbg.value(metadata ptr addrspace(3) [[GEP0]],
; CHECK-SAME: metadata ![[#A_LOCAL:]],
; CHECK-SAME: metadata !DIExpression(DW_OP_deref)
  store i32 0, ptr addrspace(3) @a.__local, align 4, !dbg !37

; DebugInfo of @b.__local is transferred to pLocalMemBase, with offset 16 (size of [4 x i32])
; CHECK: [[GEP16:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 16, !dbg
; CHECK-NEXT: call void @llvm.dbg.value(metadata ptr addrspace(3) [[GEP16]],
; CHECK-SAME: metadata ![[#B_LOCAL:]],
; CHECK-SAME: metadata !DIExpression(DW_OP_deref)
  store i64 0, ptr addrspace(3) @b.__local, align 8, !dbg !40
  ret void
}

; CHECK-LABEL: define void @cloned.main_kernel
define void @cloned.main_kernel() !dbg !41 {
entry:
; Test that the optimizer doesn't crash if multiple functions use the same __local GV.
  store i32 0, ptr addrspace(3) @a.clone.__local, align 4
  store i64 0, ptr addrspace(3) @b.clone.__local, align 8
  ret void
}

!llvm.dbg.cu = !{!10}
!llvm.linker.options = !{}
!llvm.module.flags = !{!25, !26}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!27}
!opencl.spir.version = !{!27}
!opencl.used.extensions = !{!11}
!opencl.used.optional.core.features = !{!11}
!opencl.compiler.options = !{!28}
!llvm.ident = !{!29}
!opencl.stat.type = !{!30}
!opencl.stat.exec_time = !{!31}
!opencl.stat.run_time_version = !{!32}
!opencl.stat.workload_name = !{!33}
!opencl.stat.module_name = !{!34}
!sycl.kernels = !{!35}
!opencl.stats.InstCounter.CanVect = !{!36}

; Make sure original GlobalVariable DebugInfos are removed
; CHECK-NOT: !DIGlobalVariableExpression
; CHECK-NOT: !DIGlobalVariable(name: "a"
; CHECK-NOT: !DIGlobalVariable(name: "b"

; Newly created local variable DebugInfos:
; CHECK-DAG: ![[#A_LOCAL]] = !DILocalVariable(name: "a"
; CHECK-DAG: ![[#B_LOCAL]] = !DILocalVariable(name: "b"

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 2, type: !23, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "main_kernel", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !10, retainedNodes: !11)
!3 = !DIFile(filename: "1", directory: "/")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null, !6, !6}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIDerivedType(tag: DW_TAG_typedef, name: "uchar", file: !8, line: 63, baseType: !9)
!8 = !DIFile(filename: "opencl-c-common.h", directory: "/")
!9 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!10 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !11, retainedTypes: !12, globals: !16, splitDebugInlining: false, nameTableKind: None)
!11 = !{}
!12 = !{!13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !8, line: 73, baseType: !15)
!15 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!16 = !{!0, !17}
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 3, type: !19, isLocal: true, isDefinition: true)
!19 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 256, elements: !21)
!20 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!21 = !{!22}
!22 = !DISubrange(count: 4)
!23 = !DICompositeType(tag: DW_TAG_array_type, baseType: !24, size: 128, elements: !21)
!24 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!25 = !{i32 7, !"Dwarf Version", i32 4}
!26 = !{i32 2, !"Debug Info Version", i32 3}
!27 = !{i32 1, i32 2}
!28 = !{!"-g", !"-cl-opt-disable"}
!29 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!30 = !{!""}
!31 = !{!"2021-05-27 12:45:42"}
!32 = !{!"2021.12.5.0"}
!33 = !{!"debugger_test_type"}
!34 = !{!"debugger_test_type1"}
!35 = !{ptr @main_kernel}
!36 = !{!"Code is vectorizable"}
!37 = !DILocation(line: 28, column: 18, scope: !38)
!38 = distinct !DILexicalBlock(scope: !39, file: !3, line: 27, column: 29)
!39 = distinct !DILexicalBlock(scope: !2, file: !3, line: 27, column: 7)
!40 = !DILocation(line: 33, column: 23, scope: !38)
!41 = distinct !DISubprogram(name: "main_kernel", scope: !3, file: !3, line: 1, type: !4, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !10, retainedNodes: !11)
