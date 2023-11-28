; RUN: opt -S -sycl-barrier-copy-instruction-threshold=3 -passes="sycl-kernel-reduce-cross-barrier-values" %s | FileCheck %s

; Check that cloned instruction has the same debug location as its insert point.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @test(i32 addrspace(1)* nocapture noundef writeonly align 4 %dst) local_unnamed_addr #0 !dbg !6 {
entry:
  call void @dummy_barrier.()
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %dst, metadata !12, metadata !DIExpression()), !dbg !17
  %call = tail call i64 @_Z13get_global_idj(i32 noundef 0) #5, !dbg !18
  call void @llvm.dbg.value(metadata i64 %call, metadata !13, metadata !DIExpression()), !dbg !17
  br label %Split.Barrier.BB, !dbg !19

Split.Barrier.BB:                                 ; preds = %entry
; CHECK-LABEL: Split.Barrier.BB:
; CHECK-NEXT: tail call void @_Z7barrierj(i32 noundef 1) #{{.*}}, !dbg
; CHECK-NEXT: %call.copy = tail call i64 @_Z13get_global_idj(i32 noundef 0) #{{.*}}, !dbg [[DBG:![0-9]+]]
; CHECK-NEXT: %conv = trunc i64 %call.copy to i32, !dbg [[DBG]]

  tail call void @_Z7barrierj(i32 noundef 1) #6, !dbg !19
  %conv = trunc i64 %call to i32, !dbg !20
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call, !dbg !21
  store i32 %conv, i32 addrspace(1)* %arrayidx, align 4, !dbg !22, !tbaa !23
  br label %Split.Barrier.BB9, !dbg !27

Split.Barrier.BB9:                                ; preds = %Split.Barrier.BB
  call void @_Z18work_group_barrierj(i32 1), !dbg !27
  ret void, !dbg !27
}

declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

declare void @_Z7barrierj(i32 noundef) local_unnamed_addr #2

declare void @llvm.dbg.value(metadata, metadata, metadata) #3

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32) #4

attributes #0 = { convergent norecurse nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent mustprogress nofree nounwind willreturn memory(none) }
attributes #2 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #3 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { convergent }
attributes #5 = { convergent nounwind willreturn memory(none) }
attributes #6 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" "kernel-uniform-call" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.spir.version = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "barrier.cl", directory: "/")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 3, i32 0}
!5 = !{void (i32 addrspace(1)*)* @test}
!6 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !11)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64, dwarfAddressSpace: 1)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12, !13}
!12 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!13 = !DILocalVariable(name: "gid", scope: !6, file: !1, line: 2, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !15, line: 137, baseType: !16)
!15 = !DIFile(filename: "opencl-c-base.h", directory: "/")
!16 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!17 = !DILocation(line: 0, scope: !6)
!18 = !DILocation(line: 2, column: 16, scope: !6)
!19 = !DILocation(line: 3, column: 3, scope: !6)
!20 = !DILocation(line: 4, column: 14, scope: !6)
!21 = !DILocation(line: 4, column: 3, scope: !6)
!22 = !DILocation(line: 4, column: 12, scope: !6)
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
!27 = !DILocation(line: 5, column: 1, scope: !6)
