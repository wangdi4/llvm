; Compiled from: (-g -cl-opt-disable)
; ----------------------------------------------------
; int foo(x) {
;     return sub_group_scan_inclusive_add(x);
; }
; __kernel void main_kernel() {
;     int lid = get_local_id(0);
;     int x = foo(lid);
; }
; ----------------------------------------------------
; Check whether implicit GIDs work with subgroup emulation

; RUN: %oclopt -B-ImplicitGlobalIdPass -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -B-ImplicitGlobalIdPass -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Checks that when a function has sg_barrier/dummy_sg_barrier without any
; barrier/dummy_barrier, alloca and llvm.dbg.declare instructions are placed
; after leading dummy_sg_barrier in entry block. And that GID stores are
; only inserted after sg_barrier/dummy_sg_barrier.

; CHECK-LABEL: define i32 @foo
; Function Attrs: convergent noinline norecurse nounwind
define i32 @foo(i32 %x) #0 !dbg !9 {
; CHECK: entry:
; CHECK-NEXT: call void @dummy_sg_barrier()

; CHECK-NEXT: %__ocl_dbg_gid0 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2

; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2

; CHECK-NEXT: %x.addr = alloca i32, align 4
entry:
  call void @dummy_sg_barrier()
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %x.addr, metadata !14, metadata !DIExpression()), !dbg !15
  %0 = load i32, i32* %x.addr, align 4, !dbg !16
  br label %sg.barrier.bb.

; CHECK: sg.barrier.bb.:
; CHECK-NEXT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: call i32 @_Z28sub_group_scan_inclusive_addi
sg.barrier.bb.:                                   ; preds = %entry
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z28sub_group_scan_inclusive_addi(i32 %0) #6, !dbg !17
  br label %sg.dummy.bb.

; CHECK: sg.dummy.bb.:
; CHECK-NEXT: call void @dummy_sg_barrier()
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: ret i32 %call
sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret i32 %call, !dbg !18
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent
declare i32 @_Z28sub_group_scan_inclusive_addi(i32) #2

; Checks that when a kernel has both barrier/dummy_barrier and
; sg_barrier/dummy_sg_barrier, alloca and llvm.dbg.declare instructions are
; placed after leading dummybarrier in entry block. And that GID stores are
; only inserted after sg_barrier/dummy_sg_barrier.

; CHECK-LABEL: define void @main_kernel
; Function Attrs: convergent noinline norecurse nounwind
define void @main_kernel() #3 !dbg !19 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !no_barrier_path !22 !kernel_has_sub_groups !23 !intel_reqd_sub_group_size !24 !kernel_execution_length !25 !kernel_has_barrier !22 !kernel_has_global_sync !22 !sg_emu_size !24 {
; CHECK: entry:
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NEXT: %__ocl_dbg_gid0 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64
; CHECK-NEXT: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2
; No stores here, since it's dummybarrier - dummy_sg_barrier region
; CHECK-NOT: store volatile i64 {{.*}}, i64* %__ocl_dbg_gid
; CHECK-NEXT: br label %sg.dummy.bb.2
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.2

; CHECK: sg.dummy.bb.2:
; CHECK-NEXT: call void @dummy_sg_barrier()
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: %lid = alloca i32, align 4
sg.dummy.bb.2:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %lid = alloca i32, align 4
  %x = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %lid, metadata !26, metadata !DIExpression()), !dbg !27
  %call = call i64 @_Z12get_local_idj(i32 0) #7, !dbg !28
  %conv = trunc i64 %call to i32, !dbg !28
  store i32 %conv, i32* %lid, align 4, !dbg !27
  call void @llvm.dbg.declare(metadata i32* %x, metadata !29, metadata !DIExpression()), !dbg !30
  %0 = load i32, i32* %lid, align 4, !dbg !31
  br label %sg.barrier.bb.

; CHECK: sg.barrier.bb.:
; CHECK-NEXT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: call i32 @foo(i32 %0)
sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.2
  call void @_Z17sub_group_barrierj(i32 1)
  %call1 = call i32 @foo(i32 %0) #5, !dbg !32
  br label %sg.dummy.bb.

; CHECK: sg.dummy.bb.:
; CHECK-NEXT: call void @dummy_sg_barrier()
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: store i32 %call1, i32* %x, align 4
sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  store i32 %call1, i32* %x, align 4, !dbg !30
  br label %sg.barrier.bb.1

; CHECK: sg.barrier.bb.1:
; CHECK-NEXT: call void @_Z17sub_group_barrierj(i32 1)
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: call void @_Z7barrierj(i32 1)
; CHECK-NOT: store volatile i64 {{.*}}, i64* %__ocl_dbg_gid
; CHECK-NEXT: br label %sg.dummy.bb.3
sg.barrier.bb.1:                                  ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.3

; CHECK: sg.dummy.bb.3:
; CHECK-NEXT: call void @dummy_sg_barrier()
; CHECK-NEXT: [[GID0:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 [[GID0]], i64* %__ocl_dbg_gid0
; CHECK-NEXT: [[GID1:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 [[GID1]], i64* %__ocl_dbg_gid1
; CHECK-NEXT: [[GID2:%GlobalID.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 [[GID2]], i64* %__ocl_dbg_gid2
; CHECK-NEXT: ret void
sg.dummy.bb.3:                                    ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  ret void, !dbg !33
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #4

declare void @dummybarrier.()

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #5

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM16v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv16_iDv16_j)" }
attributes #3 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent }
attributes #6 = { convergent "has-vplan-mask" }
attributes #7 = { convergent nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!sycl.kernels = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "source.cl", directory: "/")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, i32 2}
!6 = !{!"-g", !"-cl-opt-disable"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!8 = !{void ()* @main_kernel}
!9 = distinct !DISubprogram(name: "foo", scope: !10, file: !10, line: 1, type: !11, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DIFile(filename: "source.cl", directory: "/")
!11 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !12)
!12 = !{!13, !13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocalVariable(name: "x", arg: 1, scope: !9, file: !10, line: 1, type: !13)
!15 = !DILocation(line: 1, column: 9, scope: !9)
!16 = !DILocation(line: 2, column: 41, scope: !9)
!17 = !DILocation(line: 2, column: 12, scope: !9)
!18 = !DILocation(line: 2, column: 5, scope: !9)
!19 = distinct !DISubprogram(name: "main_kernel", scope: !10, file: !10, line: 6, type: !20, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!20 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !21)
!21 = !{null}
!22 = !{i1 false}
!23 = !{i1 true}
!24 = !{i32 16}
!25 = !{i32 11}
!26 = !DILocalVariable(name: "lid", scope: !19, file: !10, line: 7, type: !13)
!27 = !DILocation(line: 7, column: 9, scope: !19)
!28 = !DILocation(line: 7, column: 15, scope: !19)
!29 = !DILocalVariable(name: "x", scope: !19, file: !10, line: 8, type: !13)
!30 = !DILocation(line: 8, column: 9, scope: !19)
!31 = !DILocation(line: 8, column: 17, scope: !19)
!32 = !DILocation(line: 8, column: 13, scope: !19)
!33 = !DILocation(line: 9, column: 1, scope: !19)

; DEBUGIFY-NOT: WARNING
