; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

; This test checks that get_*_id calls are replaced in O0 and -g mode in the
; case there is no kernels.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define dso_local void @test(ptr addrspace(1) noalias noundef %dst) #{{.*}} !dbg
; CHECK-NEXT: entry:
; CHECK-NEXT: %lid1.addr = alloca i64, align 8
; CHECK-NEXT: %lid2.addr = alloca i64, align 8
; CHECK-NEXT: %gid0.addr = alloca i64, align 8
; CHECK-NEXT: %gid1.addr = alloca i64, align 8
; CHECK-NEXT: %gid2.addr = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: %dst.addr = alloca ptr addrspace(1), align 8
; CHECK-NEXT: %lid0.addr = alloca i64, align 8
; CHECK-NEXT: %lid0 = load i64, ptr @__LocalIds, align 8
; CHECK-NEXT: store i64 %lid0, ptr %lid0.addr, align 8
; CHECK-NEXT: %lid1 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1), align 8
; CHECK-NEXT: store i64 %lid1, ptr %lid1.addr, align 8
; CHECK-NEXT: %lid2 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2), align 8
; CHECK-NEXT: store i64 %lid2, ptr %lid2.addr, align 8
; CHECK-NEXT: %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT: %gid0 = add i64 %lid0, %base.gid0
; CHECK-NEXT: store i64 %gid0, ptr %gid0.addr, align 8
; CHECK-NEXT: %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT: %gid1 = add i64 %lid1, %base.gid1
; CHECK-NEXT: store i64 %gid1, ptr %gid1.addr, align 8
; CHECK-NEXT: %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT: %gid2 = add i64 %lid2, %base.gid2
; CHECK-NEXT: store i64 %gid2, ptr %gid2.addr, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !{{[0-9]+}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !{{[0-9]+}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !{{[0-9]+}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %gid0.ld = load i64, ptr %gid0.addr, align 8
; CHECK-NEXT: store volatile i64 %gid0.ld, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %gid1.ld = load i64, ptr %gid1.addr, align 8
; CHECK-NEXT: store volatile i64 %gid1.ld, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %gid2.ld = load i64, ptr %gid2.addr, align 8
; CHECK-NEXT: store volatile i64 %gid2.ld, ptr %__ocl_dbg_gid2, align 8
; CHECK-NEXT: store ptr addrspace(1) %dst, ptr %dst.addr, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !{{[0-9]+}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %lid0.ld = load i64, ptr %lid0.addr, align 8, !dbg [[DBGLid:![0-9]+]]
; CHECK-NEXT: %conv = trunc i64 %lid0.ld to i32, !dbg [[DBGLid]]
; CHECK-NEXT: [[PTR:%[0-9]+]] = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg
; CHECK-NEXT: [[GIDLoad1:%gid0.ld[0-9]+]] = load i64, ptr %gid0.addr, align 8, !dbg [[DBGGid:![0-9]+]]
; CHECK-NEXT: %arrayidx = getelementptr inbounds i32, ptr addrspace(1) [[PTR]], i64 [[GIDLoad1]], !dbg

; CHECK: [[DBGLid]] = !DILocation(line: 2, column: 27, scope: !6)
; CHECK: [[DBGGid]] = !DILocation(line: 2, column: 7, scope: !6)

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef %dst) #0 !dbg !6 !kernel_arg_base_type !23 !arg_type_null_val !24 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !11, metadata !DIExpression()), !dbg !13
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !14, metadata !DIExpression()), !dbg !13
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !15, metadata !DIExpression()), !dbg !13
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !16, metadata !DIExpression()), !dbg !17
  %call = call i64 @_Z12get_local_idj(i32 noundef 0) #3, !dbg !18
  %conv = trunc i64 %call to i32, !dbg !18
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !19
  %call1 = call i64 @_Z13get_global_idj(i32 noundef 0) #3, !dbg !20
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call1, !dbg !19
  store i32 %conv, ptr addrspace(1) %arrayidx, align 4, !dbg !21
  ret void, !dbg !22
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z12get_local_idj(i32 noundef) #2

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent nounwind readnone willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{}
!6 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !5)
!7 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, line: 1, type: !12, flags: DIFlagArtificial)
!12 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!13 = !DILocation(line: 1, scope: !6)
!14 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, line: 1, type: !12, flags: DIFlagArtificial)
!15 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, line: 1, type: !12, flags: DIFlagArtificial)
!16 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!17 = !DILocation(line: 1, column: 23, scope: !6)
!18 = !DILocation(line: 2, column: 27, scope: !6)
!19 = !DILocation(line: 2, column: 3, scope: !6)
!20 = !DILocation(line: 2, column: 7, scope: !6)
!21 = !DILocation(line: 2, column: 25, scope: !6)
!22 = !DILocation(line: 3, column: 1, scope: !6)
!23 = !{!"int*"}
!24 = !{ptr addrspace(1) null}
