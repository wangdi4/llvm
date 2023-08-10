; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

; %i6.ascast.priv is used in the second loop.
; Check that %i6.ascast.priv's load from special buffer is inserted in exit of
; of the first loop, rather than in header of the first loop.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @__omp_offloading_811_2395d5b__Z14test_map_alloc_l40(ptr addrspace(1) %0, ptr addrspace(1) %1) #0 !dbg !11 !kernel_arg_addr_space !24 !kernel_arg_access_qual !25 !kernel_arg_type !26 !kernel_arg_base_type !26 !kernel_arg_type_qual !27 !arg_type_null_val !28 !no_barrier_path !29 !kernel_has_sub_groups !29 !kernel_has_global_sync !29 !kernel_execution_length !30 !spirv.ParameterDecorations !31 !recommended_vector_length !32 {
newFuncRoot:
  call void @dummy_barrier.()
  %.ascast = addrspacecast ptr addrspace(1) %0 to ptr addrspace(4), !dbg !33
  %.ascast1 = addrspacecast ptr addrspace(1) %1 to ptr addrspace(4), !dbg !33
  %d_sum.map.ptr.tmp.ascast.priv = alloca ptr addrspace(4), align 8, !dbg !33
  %h_array_h.map.ptr.tmp.ascast.priv = alloca ptr addrspace(4), align 8, !dbg !33
  %i6.ascast.priv = alloca i32, align 4, !dbg !33
  %i.ascast.priv = alloca i32, align 4, !dbg !33
  br label %DIR.OMP.TARGET.2.split, !dbg !33

DIR.OMP.TARGET.2.split:                           ; preds = %newFuncRoot
  call void @llvm.dbg.declare(metadata ptr %d_sum.map.ptr.tmp.ascast.priv, metadata !15, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !34
  call void @llvm.dbg.declare(metadata ptr %h_array_h.map.ptr.tmp.ascast.priv, metadata !21, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !35
  call void @llvm.dbg.declare(metadata ptr %i6.ascast.priv, metadata !22, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !36
  call void @llvm.dbg.declare(metadata ptr %i.ascast.priv, metadata !23, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !37
  %2 = call i64 @_Z12get_local_idj(i32 0) #2, !dbg !33
  %3 = icmp eq i64 %2, 0, !dbg !33
  %4 = call i64 @_Z12get_local_idj(i32 1) #2, !dbg !33
  %5 = icmp eq i64 %4, 0, !dbg !33
  %6 = and i1 %3, %5, !dbg !33
  %7 = call i64 @_Z12get_local_idj(i32 2) #2, !dbg !33
  %8 = icmp eq i64 %7, 0, !dbg !33
  %is.master.thread = and i1 %6, %8, !dbg !33
  br label %Split.Barrier.BB, !dbg !33

Split.Barrier.BB:                                 ; preds = %DIR.OMP.TARGET.2.split
  call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1) #3, !dbg !33
  br i1 %is.master.thread, label %master.thread.code, label %exit, !dbg !33

master.thread.code:                               ; preds = %Split.Barrier.BB
  br label %DIR.OMP.TARGET.5, !dbg !33

DIR.OMP.TARGET.5:                                 ; preds = %master.thread.code
  store ptr addrspace(4) %.ascast, ptr %h_array_h.map.ptr.tmp.ascast.priv, align 8, !dbg !38
  store ptr addrspace(4) %.ascast1, ptr %d_sum.map.ptr.tmp.ascast.priv, align 8, !dbg !38
  store i32 0, ptr %i.ascast.priv, align 4, !dbg !39
  br label %for.cond, !dbg !42

for.cond:                                         ; preds = %for.inc, %DIR.OMP.TARGET.5
  %9 = load i32, ptr %i.ascast.priv, align 4, !dbg !43
  %cmp3 = icmp slt i32 %9, 1000, !dbg !45
  br i1 %cmp3, label %for.body, label %for.end, !dbg !46

for.body:                                         ; preds = %for.cond
  %10 = load ptr addrspace(4), ptr %h_array_h.map.ptr.tmp.ascast.priv, align 8, !dbg !47
  %11 = load i32, ptr %i.ascast.priv, align 4, !dbg !48
  %idxprom = sext i32 %11 to i64, !dbg !47
  %arrayidx4 = getelementptr inbounds i32, ptr addrspace(4) %10, i64 %idxprom, !dbg !47
  store i32 1, ptr addrspace(4) %arrayidx4, align 4, !dbg !49
  br label %for.inc, !dbg !47

for.inc:                                          ; preds = %for.body
  %12 = load i32, ptr %i.ascast.priv, align 4, !dbg !50
  %inc = add nsw i32 %12, 1, !dbg !50
  store i32 %inc, ptr %i.ascast.priv, align 4, !dbg !50
  br label %for.cond, !dbg !51

for.end:                                          ; preds = %for.cond
; CHECK-LABEL: for.end:
; CHECK-NEXT: [[Index:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex, align 8
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %i6.ascast.priv.addr, align 8
; CHECK-NEXT: [[LOAD:%[0-9]+]] = load ptr, ptr %i6.ascast.priv.addr, align 8
; CHECK: store i32 0, ptr [[LOAD]], align 4

  %13 = load ptr addrspace(4), ptr %d_sum.map.ptr.tmp.ascast.priv, align 8, !dbg !52
  %arrayidx5 = getelementptr inbounds i32, ptr addrspace(4) %13, i64 0, !dbg !52
  store i32 0, ptr addrspace(4) %arrayidx5, align 4, !dbg !53
  store i32 0, ptr %i6.ascast.priv, align 4, !dbg !54
  br label %for.cond7, !dbg !56

for.cond7:                                        ; preds = %for.inc13, %for.end
; CHECK-LABEL: for.cond7:
; CHECK-NEXT: load i32, ptr [[LOAD]], align 4

  %14 = load i32, ptr %i6.ascast.priv, align 4, !dbg !57
  %cmp8 = icmp slt i32 %14, 1000, !dbg !59
  br i1 %cmp8, label %for.body9, label %for.end15, !dbg !60

for.body9:                                        ; preds = %for.cond7
; CHECK-LABEL: for.body9:
; CHECK: load i32, ptr [[LOAD]], align 4

  %15 = load ptr addrspace(4), ptr %h_array_h.map.ptr.tmp.ascast.priv, align 8, !dbg !61
  %16 = load i32, ptr %i6.ascast.priv, align 4, !dbg !62
  %idxprom10 = sext i32 %16 to i64, !dbg !61
  %arrayidx11 = getelementptr inbounds i32, ptr addrspace(4) %15, i64 %idxprom10, !dbg !61
  %17 = load i32, ptr addrspace(4) %arrayidx11, align 4, !dbg !61
  %18 = load ptr addrspace(4), ptr %d_sum.map.ptr.tmp.ascast.priv, align 8, !dbg !63
  %arrayidx12 = getelementptr inbounds i32, ptr addrspace(4) %18, i64 0, !dbg !63
  %19 = load i32, ptr addrspace(4) %arrayidx12, align 4, !dbg !64
  %add = add nsw i32 %19, %17, !dbg !64
  store i32 %add, ptr addrspace(4) %arrayidx12, align 4, !dbg !64
  br label %for.inc13, !dbg !63

for.inc13:                                        ; preds = %for.body9
; CHECK-LABEL: for.inc13:
; CHECK-NEXT: load i32, ptr [[LOAD]], align 4
; CHECK: store i32 %inc14, ptr [[LOAD]], align 4

  %20 = load i32, ptr %i6.ascast.priv, align 4, !dbg !65
  %inc14 = add nsw i32 %20, 1, !dbg !65
  store i32 %inc14, ptr %i6.ascast.priv, align 4, !dbg !65
  br label %for.cond7, !dbg !66

for.end15:                                        ; preds = %for.cond7
  br label %exit

exit:                                             ; preds = %for.end15, %Split.Barrier.BB
  ret void
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i64 @_Z12get_local_idj(i32) #2

declare void @_Z18work_group_barrierj12memory_scope(i32, i32) #3

declare i64 @_Z13get_global_idj(i32)

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32) #4

attributes #0 = { convergent noinline nounwind optnone "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nounwind }
attributes #3 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #4 = { convergent }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}
!spirv.MemoryModel = !{!5}
!spirv.Source = !{!6}
!opencl.used.extensions = !{!7}
!opencl.used.optional.core.features = !{!8}
!spirv.Generator = !{!9}
!sycl.kernels = !{!10}

!0 = !{i32 7, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4)
!3 = !DIFile(filename: "test", directory: "/")
!4 = !{}
!5 = !{i32 2, i32 2}
!6 = !{i32 4, i32 200000}
!7 = !{!"cl_khr_int64_extended_atomics", !"cl_khr_subgroups"}
!8 = !{!"cl_doubles"}
!9 = !{i16 6, i16 14}
!10 = distinct !{ptr @__omp_offloading_811_2395d5b__Z14test_map_alloc_l40}
!11 = distinct !DISubprogram(name: "test", scope: null, file: !3, line: 40, type: !12, scopeLine: 40, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagMainSubprogram, unit: !2, templateParams: !4, retainedNodes: !14)
!12 = !DISubroutineType(types: !13)
!13 = !{null}
!14 = !{!15, !21, !22, !23}
!15 = !DILocalVariable(name: "d_sum", scope: !16, file: !3, line: 30, type: !19)
!16 = distinct !DILexicalBlock(scope: !17, file: !3, line: 40, column: 1)
!17 = distinct !DILexicalBlock(scope: !18, file: !3, line: 39, column: 3)
!18 = distinct !DILexicalBlock(scope: !11, file: !3, line: 38, column: 1)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64, dwarfAddressSpace: 4)
!20 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!21 = !DILocalVariable(name: "h_array_h", scope: !16, file: !3, line: 25, type: !19)
!22 = !DILocalVariable(name: "i", scope: !16, file: !3, line: 47, type: !20)
!23 = !DILocalVariable(name: "i", scope: !16, file: !3, line: 42, type: !20)
!24 = !{i32 1, i32 1}
!25 = !{!"none", !"none"}
!26 = !{!"int*", !"int*"}
!27 = !{!"", !""}
!28 = !{ptr addrspace(1) null, ptr addrspace(1) null}
!29 = !{i1 false}
!30 = !{i32 345}
!31 = !{!4, !4}
!32 = !{i32 1}
!33 = !DILocation(line: 40, column: 1, scope: !16)
!34 = !DILocation(line: 30, column: 8, scope: !16)
!35 = !DILocation(line: 25, column: 8, scope: !16)
!36 = !DILocation(line: 47, column: 16, scope: !16)
!37 = !DILocation(line: 42, column: 16, scope: !16)
!38 = !DILocation(line: 40, column: 1, scope: !17)
!39 = !DILocation(line: 42, column: 16, scope: !40)
!40 = distinct !DILexicalBlock(scope: !41, file: !3, line: 42, column: 7)
!41 = distinct !DILexicalBlock(scope: !16, file: !3, line: 41, column: 5)
!42 = !DILocation(line: 42, column: 12, scope: !40)
!43 = !DILocation(line: 42, column: 23, scope: !44)
!44 = distinct !DILexicalBlock(scope: !40, file: !3, line: 42, column: 7)
!45 = !DILocation(line: 42, column: 25, scope: !44)
!46 = !DILocation(line: 42, column: 7, scope: !40)
!47 = !DILocation(line: 43, column: 9, scope: !44)
!48 = !DILocation(line: 43, column: 19, scope: !44)
!49 = !DILocation(line: 43, column: 22, scope: !44)
!50 = !DILocation(line: 42, column: 30, scope: !44)
!51 = !DILocation(line: 42, column: 7, scope: !44)
!52 = !DILocation(line: 46, column: 7, scope: !41)
!53 = !DILocation(line: 46, column: 16, scope: !41)
!54 = !DILocation(line: 47, column: 16, scope: !55)
!55 = distinct !DILexicalBlock(scope: !41, file: !3, line: 47, column: 7)
!56 = !DILocation(line: 47, column: 12, scope: !55)
!57 = !DILocation(line: 47, column: 23, scope: !58)
!58 = distinct !DILexicalBlock(scope: !55, file: !3, line: 47, column: 7)
!59 = !DILocation(line: 47, column: 25, scope: !58)
!60 = !DILocation(line: 47, column: 7, scope: !55)
!61 = !DILocation(line: 48, column: 21, scope: !58)
!62 = !DILocation(line: 48, column: 31, scope: !58)
!63 = !DILocation(line: 48, column: 9, scope: !58)
!64 = !DILocation(line: 48, column: 18, scope: !58)
!65 = !DILocation(line: 47, column: 30, scope: !58)
!66 = !DILocation(line: 47, column: 7, scope: !58)
