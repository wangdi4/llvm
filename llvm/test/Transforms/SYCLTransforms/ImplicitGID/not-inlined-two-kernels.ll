; RUN: opt -implicit-gid-handle-barrier=false -passes=sycl-kernel-implicit-gid -S %s | FileCheck %s

; This test checks that implicit gids are only added to non-inlined function
; that is used in kernel without barrier path.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @foo(ptr addrspace(1) noalias noundef %dst, i64 noundef %gid) #0 !dbg !6 !kernel_arg_base_type !68 !arg_type_null_val !69 {
entry:
; CHECK-LABEL: @foo
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata [[fooDbgMD0:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata [[fooDbgMD1:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata [[fooDbgMD2:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid.addr = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !17, metadata !DIExpression()), !dbg !18
  store i64 %gid, ptr %gid.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %gid.addr, metadata !19, metadata !DIExpression()), !dbg !20
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !21
  %1 = load i64, ptr %gid.addr, align 8, !dbg !22
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %1, !dbg !21
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !23
  %2 = load i32, ptr %retval, align 4, !dbg !24
  ret i32 %2, !dbg !24
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @bar(ptr addrspace(1) noalias noundef %dst, i64 noundef %gid) #0 !dbg !25 !kernel_arg_base_type !68 !arg_type_null_val !69 {
entry:
; CHECK-LABEL: @bar
; CHECK-NOT: %__ocl_dbg_gid{{[0-9]}} = alloca i64

  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid.addr = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !26, metadata !DIExpression()), !dbg !27
  store i64 %gid, ptr %gid.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %gid.addr, metadata !28, metadata !DIExpression()), !dbg !29
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !30
  %1 = load i64, ptr %gid.addr, align 8, !dbg !31
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %1, !dbg !30
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !32
  %2 = load i32, ptr %retval, align 4, !dbg !33
  ret i32 %2, !dbg !33
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #2 !dbg !34 !kernel_arg_addr_space !37 !kernel_arg_access_qual !38 !kernel_arg_type !39 !kernel_arg_base_type !39 !kernel_arg_type_qual !40 !kernel_arg_name !41 !kernel_arg_host_accessible !42 !kernel_arg_pipe_depth !43 !kernel_arg_pipe_io !44 !kernel_arg_buffer_location !44 !no_barrier_path !45 !kernel_has_sub_groups !42 !kernel_execution_length !46 !kernel_has_global_sync !42 !recommended_vector_length !37 !arg_type_null_val !70 {
entry:
; CHECK-LABEL: @test(
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata [[DbgMD0:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata [[DbgMD1:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata [[DbgMD2:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  %dst.addr = alloca ptr addrspace(1), align 8
  %gid = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !47, metadata !DIExpression()), !dbg !48
  call void @llvm.dbg.declare(metadata ptr %gid, metadata !49, metadata !DIExpression()), !dbg !50
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #6, !dbg !51
  store i64 %call, ptr %gid, align 8, !dbg !50
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !52
  %1 = load i64, ptr %gid, align 8, !dbg !53
  %call1 = call i32 @foo(ptr addrspace(1) noundef %0, i64 noundef %1) #7, !dbg !54
  ret void, !dbg !55
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #3

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test_barrier(ptr addrspace(1) noalias noundef align 4 %dst) #4 !dbg !56 !kernel_arg_addr_space !37 !kernel_arg_access_qual !38 !kernel_arg_type !39 !kernel_arg_base_type !39 !kernel_arg_type_qual !40 !kernel_arg_name !41 !kernel_arg_host_accessible !42 !kernel_arg_pipe_depth !43 !kernel_arg_pipe_io !44 !kernel_arg_buffer_location !44 !no_barrier_path !42 !kernel_has_sub_groups !42 !kernel_execution_length !57 !kernel_has_global_sync !42 !recommended_vector_length !37 !arg_type_null_val !70 {
entry:
; CHECK-LABEL: @test_barrier
; CHECK-NOT: %__ocl_dbg_gid{{[0-9]}} = alloca i64

  %dst.addr = alloca ptr addrspace(1), align 8
  %gid = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !58, metadata !DIExpression()), !dbg !59
  call void @llvm.dbg.declare(metadata ptr %gid, metadata !60, metadata !DIExpression()), !dbg !61
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #6, !dbg !62
  store i64 %call, ptr %gid, align 8, !dbg !61
  call void @_Z7barrierj(i32 noundef 1) #8, !dbg !63
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !64
  %1 = load i64, ptr %gid, align 8, !dbg !65
  %call1 = call i32 @bar(ptr addrspace(1) noundef %0, i64 noundef %1) #7, !dbg !66
  ret void, !dbg !67
}

; CHECK: [[fooDbgMD0]] = !DILocalVariable(name: "__ocl_dbg_gid0",
; CHECK: [[fooDbgMD1]] = !DILocalVariable(name: "__ocl_dbg_gid1",
; CHECK: [[fooDbgMD2]] = !DILocalVariable(name: "__ocl_dbg_gid2",

; CHECK: [[DbgMD0]] = !DILocalVariable(name: "__ocl_dbg_gid0",
; CHECK: [[DbgMD1]] = !DILocalVariable(name: "__ocl_dbg_gid1",
; CHECK: [[DbgMD2]] = !DILocalVariable(name: "__ocl_dbg_gid2",

; Function Attrs: convergent
declare void @_Z7barrierj(i32 noundef) #5

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #3 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #5 = { convergent "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #6 = { convergent nounwind readnone willreturn }
attributes #7 = { convergent }
attributes #8 = { convergent "kernel-call-once" "kernel-convergent-call" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{ptr @test, ptr @test_barrier}
!6 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!7 = !DIFile(filename: "test.cl", directory: "")
!8 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !9)
!9 = !{!10, !11, !13}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !12)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !14, line: 116, baseType: !15)
!14 = !DIFile(filename: "opencl-c-base.h", directory: "")
!15 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!16 = !{}
!17 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !7, line: 1, type: !11)
!18 = !DILocation(line: 1, column: 57, scope: !6)
!19 = !DILocalVariable(name: "gid", arg: 2, scope: !6, file: !7, line: 1, type: !13)
!20 = !DILocation(line: 1, column: 69, scope: !6)
!21 = !DILocation(line: 2, column: 3, scope: !6)
!22 = !DILocation(line: 2, column: 7, scope: !6)
!23 = !DILocation(line: 2, column: 12, scope: !6)
!24 = !DILocation(line: 3, column: 1, scope: !6)
!25 = distinct !DISubprogram(name: "bar", scope: !7, file: !7, line: 5, type: !8, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!26 = !DILocalVariable(name: "dst", arg: 1, scope: !25, file: !7, line: 5, type: !11)
!27 = !DILocation(line: 5, column: 57, scope: !25)
!28 = !DILocalVariable(name: "gid", arg: 2, scope: !25, file: !7, line: 5, type: !13)
!29 = !DILocation(line: 5, column: 69, scope: !25)
!30 = !DILocation(line: 6, column: 3, scope: !25)
!31 = !DILocation(line: 6, column: 7, scope: !25)
!32 = !DILocation(line: 6, column: 12, scope: !25)
!33 = !DILocation(line: 7, column: 1, scope: !25)
!34 = distinct !DISubprogram(name: "test", scope: !7, file: !7, line: 9, type: !35, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!35 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !36)
!36 = !{null, !11}
!37 = !{i32 1}
!38 = !{!"none"}
!39 = !{!"int*"}
!40 = !{!"restrict"}
!41 = !{!"dst"}
!42 = !{i1 false}
!43 = !{i32 0}
!44 = !{!""}
!45 = !{i1 true}
!46 = !{i32 11}
!47 = !DILocalVariable(name: "dst", arg: 1, scope: !34, file: !7, line: 9, type: !11)
!48 = !DILocation(line: 9, column: 40, scope: !34)
!49 = !DILocalVariable(name: "gid", scope: !34, file: !7, line: 10, type: !13)
!50 = !DILocation(line: 10, column: 10, scope: !34)
!51 = !DILocation(line: 10, column: 16, scope: !34)
!52 = !DILocation(line: 11, column: 7, scope: !34)
!53 = !DILocation(line: 11, column: 12, scope: !34)
!54 = !DILocation(line: 11, column: 3, scope: !34)
!55 = !DILocation(line: 12, column: 1, scope: !34)
!56 = distinct !DISubprogram(name: "test_barrier", scope: !7, file: !7, line: 14, type: !35, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!57 = !{i32 12}
!58 = !DILocalVariable(name: "dst", arg: 1, scope: !56, file: !7, line: 14, type: !11)
!59 = !DILocation(line: 14, column: 48, scope: !56)
!60 = !DILocalVariable(name: "gid", scope: !56, file: !7, line: 15, type: !13)
!61 = !DILocation(line: 15, column: 10, scope: !56)
!62 = !DILocation(line: 15, column: 16, scope: !56)
!63 = !DILocation(line: 16, column: 3, scope: !56)
!64 = !DILocation(line: 17, column: 7, scope: !56)
!65 = !DILocation(line: 17, column: 12, scope: !56)
!66 = !DILocation(line: 17, column: 3, scope: !56)
!67 = !DILocation(line: 18, column: 1, scope: !56)
!68 = !{!"int*", !"long"}
!69 = !{ptr addrspace(1) null, i64 0}
!70 = !{ptr addrspace(1) null}
