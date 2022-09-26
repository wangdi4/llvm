; RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec %s | FileCheck %s
; RUN: opt -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noduplicate nounwind
define void @test_atomic_kernel(i32 %threadCount, i32 %numDestItems, i32 addrspace(1)* %finalDest, i32 addrspace(1)* %oldValues, i32 addrspace(3)* %destMemory, i32 addrspace(3)* %localValues) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !15 !vectorized_kernel !16 !no_barrier_path !17 !vectorized_width !18 !scalarized_kernel !19 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  %call1 = tail call i64 @_Z12get_local_idj(i32 0) #5
  %cmp = icmp eq i64 %call1, 0
  %cmp.not = xor i1 %cmp, true
  %cmp369 = icmp eq i32 %numDestItems, 0
  %or.cond = or i1 %cmp369, %cmp.not
  br i1 %or.cond, label %if.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv77 = phi i64 [ %indvars.iv.next78, %for.body ], [ 0, %entry ]
  %add.ptr = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %indvars.iv77
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %finalDest, i64 %indvars.iv77
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !20
  tail call void @_Z12atomic_storePU3AS3VU7_Atomicii(i32 addrspace(3)* %add.ptr, i32 %0) #6
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %wide.trip.count79 = zext i32 %numDestItems to i64
  %exitcond80 = icmp eq i64 %indvars.iv.next78, %wide.trip.count79
  br i1 %exitcond80, label %if.end, label %for.body

if.end:                                           ; preds = %entry, %for.body
  tail call void @_Z7barrierj(i32 1) #6
  %mul = mul i32 %conv, 3
  %mul13 = mul i64 %call1, 3
  %idxprom10 = zext i32 %mul to i64
  %arrayidx11 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10
  %1 = load i32, i32 addrspace(1)* %arrayidx11, align 4, !tbaa !20
  %arrayidx16 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %mul13
  store i32 %1, i32 addrspace(3)* %arrayidx16, align 4, !tbaa !20
  %add.1 = add i32 %mul, 1
  %idxprom10.1 = zext i32 %add.1 to i64
  %arrayidx11.1 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10.1
  %2 = load i32, i32 addrspace(1)* %arrayidx11.1, align 4, !tbaa !20
  %add15.1 = add i64 %mul13, 1
  %arrayidx16.1 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add15.1
  store i32 %2, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  %add.2 = add i32 %mul, 2
  %idxprom10.2 = zext i32 %add.2 to i64
  %arrayidx11.2 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10.2
  %3 = load i32, i32 addrspace(1)* %arrayidx11.2, align 4, !tbaa !20
  %add15.2 = add i64 %mul13, 2
  %arrayidx16.2 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add15.2
  store i32 %3, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  tail call void @_Z7barrierj(i32 1) #6
  %call21 = tail call i64 @_Z14get_local_sizej(i32 0) #5
  %4 = xor i64 %call1, -1
  %sub22 = add i64 %call21, %4
  %arrayidx28 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %call1
  store i32 1, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  tail call void @_Z21atomic_store_explicitPU3AS3VU7_Atomicii12memory_order12memory_scope(i32 addrspace(3)* %arrayidx28, i32 1, i32 3, i32 1) #6
  %arrayidx29.peel = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %sub22
  %call3063.peel = tail call i32 @_Z20atomic_load_explicitPU3AS3VU7_Atomici12memory_order12memory_scope(i32 addrspace(3)* %arrayidx29.peel, i32 0, i32 1) #6
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 1, i32 2, i32 1) #6
  %mul31.peel = mul i64 %sub22, 3
  %conv32.peel = sext i32 %call3063.peel to i64
  %add33.peel = add i64 %mul31.peel, %conv32.peel
  %arrayidx34.peel = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add33.peel
  %5 = load i32, i32 addrspace(3)* %arrayidx34.peel, align 4, !tbaa !20
  %add35.peel = add i64 %sub22, 1
  %rem.peel = urem i64 %add35.peel, %call21
  %cmp37.peel = icmp eq i32 %call3063.peel, %5
  br i1 %cmp37.peel, label %do.end, label %if.then43

do.end:                                           ; preds = %if.end
  store i32 2, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  tail call void @_Z21atomic_store_explicitPU3AS3VU7_Atomicii12memory_order12memory_scope(i32 addrspace(3)* %arrayidx28, i32 2, i32 3, i32 1) #6
  %arrayidx29 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %rem.peel
  %call3063 = tail call i32 @_Z20atomic_load_explicitPU3AS3VU7_Atomici12memory_order12memory_scope(i32 addrspace(3)* %arrayidx29, i32 0, i32 1) #6
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 1, i32 2, i32 1) #6
  %add35 = add i64 %rem.peel, 1
  %rem = urem i64 %add35, %call21
  %mul31 = mul i64 %rem.peel, 3
  %conv32 = sext i32 %call3063 to i64
  %add33 = add i64 %mul31, %conv32
  %arrayidx34 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add33
  %6 = load i32, i32 addrspace(3)* %arrayidx34, align 4, !tbaa !20
  %cmp37 = icmp eq i32 %call3063, %6
  br i1 %cmp37, label %if.end82, label %if.then43

if.then43:                                        ; preds = %if.end, %do.end
  %rem.lcssa88 = phi i64 [ %rem, %do.end ], [ %rem.peel, %if.end ]
  %add26.lcssa87 = phi i64 [ %add15.2, %do.end ], [ %add15.1, %if.end ]
  %myValue.0.lcssa86 = phi i32 [ 1, %do.end ], [ 0, %if.end ]
  tail call void @_Z12atomic_storePU3AS3VU7_Atomicii(i32 addrspace(3)* %arrayidx28, i32 %myValue.0.lcssa86) #6
  %cmp52 = icmp eq i32 %myValue.0.lcssa86, 0
  br i1 %cmp52, label %if.then54, label %if.end82

if.then54:                                        ; preds = %if.then43
  %add47 = add i64 %call21, -1
  %sub48 = add i64 %add47, %rem.lcssa88
  %rem50 = urem i64 %sub48, %call21
  %conv55 = trunc i64 %rem50 to i32
  %add59 = add i64 %add26.lcssa87, 1
  %arrayidx60 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add59
  store i32 %conv55, i32 addrspace(3)* %arrayidx60, align 4, !tbaa !20
  br label %if.end82

if.end82:                                         ; preds = %if.then54, %if.then43, %do.end
  tail call void @_Z7barrierj(i32 1) #6
  %7 = load i32, i32 addrspace(3)* %arrayidx16, align 4, !tbaa !20
  store i32 %7, i32 addrspace(1)* %arrayidx11, align 4, !tbaa !20
  %8 = load i32, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  store i32 %8, i32 addrspace(1)* %arrayidx11.1, align 4, !tbaa !20
  %9 = load i32, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  store i32 %9, i32 addrspace(1)* %arrayidx11.2, align 4, !tbaa !20
  tail call void @_Z7barrierj(i32 1) #6
  %cmp.not89 = xor i1 %cmp, true
  %cmp10765 = icmp eq i32 %numDestItems, 0
  %or.cond90 = or i1 %cmp10765, %cmp.not89
  br i1 %or.cond90, label %if.end119, label %for.body110

for.body110:                                      ; preds = %if.end82, %for.body110
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body110 ], [ 0, %if.end82 ]
  %add.ptr112 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %indvars.iv
  %call11364 = tail call i32 @_Z11atomic_loadPU3AS3VU7_Atomici(i32 addrspace(3)* %add.ptr112) #6
  %arrayidx115 = getelementptr inbounds i32, i32 addrspace(1)* %finalDest, i64 %indvars.iv
  store i32 %call11364, i32 addrspace(1)* %arrayidx115, align 4, !tbaa !20
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %wide.trip.count = zext i32 %numDestItems to i64
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %if.end119, label %for.body110

if.end119:                                        ; preds = %if.end82, %for.body110
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent noduplicate
declare void @_Z7barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent nounwind readnone
declare i64 @_Z14get_local_sizej(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32, i32, i32) local_unnamed_addr #3

; Function Attrs: convergent
declare void @_Z12atomic_storePU3AS3VU7_Atomicii(i32 addrspace(3)*, i32) local_unnamed_addr #3

; Function Attrs: convergent
declare void @_Z21atomic_store_explicitPU3AS3VU7_Atomicii12memory_order12memory_scope(i32 addrspace(3)*, i32, i32, i32) local_unnamed_addr #3

; Function Attrs: convergent
declare i32 @_Z20atomic_load_explicitPU3AS3VU7_Atomici12memory_order12memory_scope(i32 addrspace(3)*, i32, i32) local_unnamed_addr #3

; Function Attrs: convergent
declare i32 @_Z11atomic_loadPU3AS3VU7_Atomici(i32 addrspace(3)*) local_unnamed_addr #3

; Function Attrs: convergent noduplicate nounwind
define void @_ZGVdN8uuuuuu_test_atomic_kernel(i32 %threadCount, i32 %numDestItems, i32 addrspace(1)* %finalDest, i32 addrspace(1)* %oldValues, i32 addrspace(3)* %destMemory, i32 addrspace(3)* %localValues) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !13 !kernel_arg_pipe_io !14 !kernel_arg_buffer_location !14 !kernel_arg_name !15 !vectorized_kernel !19 !no_barrier_path !17 !ocl_recommended_vector_length !24 !vectorized_width !24 !vectorization_dimension !6 !scalarized_kernel !5 !can_unite_workgroups !17 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %call1 = tail call i64 @_Z12get_local_idj(i32 0) #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 %threadCount, i32 %numDestItems), "QUAL.OMP.UNIFORM:TYPED"(i32 addrspace(1)* %finalDest, i32 0, i32 1), "QUAL.OMP.UNIFORM:TYPED"(i32 addrspace(1)* %oldValues, i32 0, i32 1), "QUAL.OMP.UNIFORM:TYPED"(i32 addrspace(3)* %destMemory, i32 0, i32 1), "QUAL.OMP.UNIFORM:TYPED"(i32 addrspace(3)* %localValues, i32 0, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add1 = add nuw i64 %0, %call1
  %1 = sext i32 %index to i64
  %add = add nuw i64 %1, %call
  %conv = trunc i64 %add to i32
  %cmp = icmp eq i64 %add1, 0
  %cmp.not = xor i1 %cmp, true
  %cmp369 = icmp eq i32 %numDestItems, 0
  %or.cond = or i1 %cmp369, %cmp.not
  br i1 %or.cond, label %if.end, label %for.body

for.body:                                         ; preds = %for.body, %simd.loop
  %indvars.iv77 = phi i64 [ %indvars.iv.next78, %for.body ], [ 0, %simd.loop ]
  %add.ptr = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %indvars.iv77
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %finalDest, i64 %indvars.iv77
  %2 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !20
  tail call void @_Z12atomic_storePU3AS3VU7_Atomicii(i32 addrspace(3)* %add.ptr, i32 %2) #6
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %wide.trip.count79 = zext i32 %numDestItems to i64
  %exitcond80 = icmp eq i64 %indvars.iv.next78, %wide.trip.count79
  br i1 %exitcond80, label %if.end, label %for.body

if.end:                                           ; preds = %for.body, %simd.loop
  tail call void @_Z7barrierj(i32 1) #6
  %mul = mul i32 %conv, 3
  %mul13 = mul i64 %add1, 3
  %idxprom10 = zext i32 %mul to i64
  %arrayidx11 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10
  %3 = load i32, i32 addrspace(1)* %arrayidx11, align 4, !tbaa !20
  %arrayidx16 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %mul13
  store i32 %3, i32 addrspace(3)* %arrayidx16, align 4, !tbaa !20
  %add.1 = add i32 %mul, 1
  %idxprom10.1 = zext i32 %add.1 to i64
  %arrayidx11.1 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10.1
  %4 = load i32, i32 addrspace(1)* %arrayidx11.1, align 4, !tbaa !20
  %add15.1 = add i64 %mul13, 1
  %arrayidx16.1 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add15.1
  store i32 %4, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  %add.2 = add i32 %mul, 2
  %idxprom10.2 = zext i32 %add.2 to i64
  %arrayidx11.2 = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %idxprom10.2
  %5 = load i32, i32 addrspace(1)* %arrayidx11.2, align 4, !tbaa !20
  %add15.2 = add i64 %mul13, 2
  %arrayidx16.2 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add15.2
  store i32 %5, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  tail call void @_Z7barrierj(i32 1) #6
  %call21 = tail call i64 @_Z14get_local_sizej(i32 0) #5
  %6 = xor i64 %add1, -1
  %sub22 = add i64 %call21, %6
  %arrayidx28 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %add1
  store i32 1, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  tail call void @_Z21atomic_store_explicitPU3AS3VU7_Atomicii12memory_order12memory_scope(i32 addrspace(3)* %arrayidx28, i32 1, i32 3, i32 1) #6
  %arrayidx29.peel = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %sub22
  %call3063.peel = tail call i32 @_Z20atomic_load_explicitPU3AS3VU7_Atomici12memory_order12memory_scope(i32 addrspace(3)* %arrayidx29.peel, i32 0, i32 1) #6
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 1, i32 2, i32 1) #6
  %mul31.peel = mul i64 %sub22, 3
  %conv32.peel = sext i32 %call3063.peel to i64
  %add33.peel = add i64 %mul31.peel, %conv32.peel
  %arrayidx34.peel = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add33.peel
  %7 = load i32, i32 addrspace(3)* %arrayidx34.peel, align 4, !tbaa !20
  %add35.peel = add i64 %sub22, 1
  %rem.peel = urem i64 %add35.peel, %call21
  %cmp37.peel = icmp eq i32 %call3063.peel, %7
  br i1 %cmp37.peel, label %do.end, label %if.then43

do.end:                                           ; preds = %if.end
  store i32 2, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  tail call void @_Z21atomic_store_explicitPU3AS3VU7_Atomicii12memory_order12memory_scope(i32 addrspace(3)* %arrayidx28, i32 2, i32 3, i32 1) #6
  %arrayidx29 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %rem.peel
  %call3063 = tail call i32 @_Z20atomic_load_explicitPU3AS3VU7_Atomici12memory_order12memory_scope(i32 addrspace(3)* %arrayidx29, i32 0, i32 1) #6
  tail call void @_Z22atomic_work_item_fencej12memory_order12memory_scope(i32 1, i32 2, i32 1) #6
  %add35 = add i64 %rem.peel, 1
  %rem = urem i64 %add35, %call21
  %mul31 = mul i64 %rem.peel, 3
  %conv32 = sext i32 %call3063 to i64
  %add33 = add i64 %mul31, %conv32
  %arrayidx34 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add33
  %8 = load i32, i32 addrspace(3)* %arrayidx34, align 4, !tbaa !20
  %cmp37 = icmp eq i32 %call3063, %8
  br i1 %cmp37, label %if.end82, label %if.then43

if.then43:                                        ; preds = %do.end, %if.end
  %rem.lcssa88 = phi i64 [ %rem, %do.end ], [ %rem.peel, %if.end ]
  %add26.lcssa87 = phi i64 [ %add15.2, %do.end ], [ %add15.1, %if.end ]
  %myValue.0.lcssa86 = phi i32 [ 1, %do.end ], [ 0, %if.end ]
  tail call void @_Z12atomic_storePU3AS3VU7_Atomicii(i32 addrspace(3)* %arrayidx28, i32 %myValue.0.lcssa86) #6
  %cmp52 = icmp eq i32 %myValue.0.lcssa86, 0
  br i1 %cmp52, label %if.then54, label %if.end82

if.then54:                                        ; preds = %if.then43
  %add47 = add i64 %call21, -1
  %sub48 = add i64 %add47, %rem.lcssa88
  %rem50 = urem i64 %sub48, %call21
  %conv55 = trunc i64 %rem50 to i32
  %add59 = add i64 %add26.lcssa87, 1
  %arrayidx60 = getelementptr inbounds i32, i32 addrspace(3)* %localValues, i64 %add59
  store i32 %conv55, i32 addrspace(3)* %arrayidx60, align 4, !tbaa !20
  br label %if.end82

if.end82:                                         ; preds = %if.then54, %if.then43, %do.end
  tail call void @_Z7barrierj(i32 1) #6
  %9 = load i32, i32 addrspace(3)* %arrayidx16, align 4, !tbaa !20
  store i32 %9, i32 addrspace(1)* %arrayidx11, align 4, !tbaa !20
  %10 = load i32, i32 addrspace(3)* %arrayidx16.1, align 4, !tbaa !20
  store i32 %10, i32 addrspace(1)* %arrayidx11.1, align 4, !tbaa !20
  %11 = load i32, i32 addrspace(3)* %arrayidx16.2, align 4, !tbaa !20
  store i32 %11, i32 addrspace(1)* %arrayidx11.2, align 4, !tbaa !20
  tail call void @_Z7barrierj(i32 1) #6
  %cmp.not89 = xor i1 %cmp, true
  %cmp10765 = icmp eq i32 %numDestItems, 0
  %or.cond90 = or i1 %cmp10765, %cmp.not89
  br i1 %or.cond90, label %if.end119, label %for.body110

for.body110:                                      ; preds = %for.body110, %if.end82
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body110 ], [ 0, %if.end82 ]
  %add.ptr112 = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %indvars.iv
  %call11364 = tail call i32 @_Z11atomic_loadPU3AS3VU7_Atomici(i32 addrspace(3)* %add.ptr112) #6
  %arrayidx115 = getelementptr inbounds i32, i32 addrspace(1)* %finalDest, i64 %indvars.iv
  store i32 %call11364, i32 addrspace(1)* %arrayidx115, align 4, !tbaa !20
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %wide.trip.count = zext i32 %numDestItems to i64
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %if.end119, label %for.body110

if.end119:                                        ; preds = %for.body110, %if.end82
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %if.end119
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !25

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"(), "DIR.QUAL.LIST.END"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { convergent noduplicate nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uuuuuu_test_atomic_kernel" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noduplicate "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 73a7cd4b8b270182f03b0d325c3fd4cd6e6dbf56) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bee4537ea28bde70841c48e6a4811ac4f86f36d9)"}
!5 = !{void (i32, i32, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(3)*, i32 addrspace(3)*)* @test_atomic_kernel}
!6 = !{i32 0}
!7 = !{i32 0, i32 0, i32 1, i32 1, i32 3, i32 3}
!8 = !{!"none", !"none", !"none", !"none", !"none", !"none"}
!9 = !{!"uint", !"uint", !"int*", !"int*", !"atomic_int*", !"int*"}
!10 = !{!"uint", !"uint", !"int*", !"int*", !"_Atomic(int)*", !"int*"}
!11 = !{!"", !"", !"", !"", !"volatile", !""}
!12 = !{i1 false, i1 false, i1 false, i1 false, i1 false, i1 false}
!13 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!14 = !{!"", !"", !"", !"", !"", !""}
!15 = !{!"threadCount", !"numDestItems", !"finalDest", !"oldValues", !"destMemory", !"localValues"}
!16 = !{void (i32, i32, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(3)*, i32 addrspace(3)*)* @_ZGVdN8uuuuuu_test_atomic_kernel}
!17 = !{i1 false}
!18 = !{i32 1}
!19 = !{null}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !22, i64 0}
!22 = !{!"omnipotent char", !23, i64 0}
!23 = !{!"Simple C/C++ TBAA"}
!24 = !{i32 8}
!25 = distinct !{!25, !26}
!26 = !{!"llvm.loop.unroll.disable"}
