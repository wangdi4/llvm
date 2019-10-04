;RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring  -VPlanDriver -vplan-force-vf=2 < %s 2>&1 | FileCheck %s


; CHECK-LABEL: @_ZGVdN8u___block_fn_block_invoke_kernel
; CHECK: {{.*}} = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK: {{.*}} = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK: [[BLOCK_I_I_VEC:%.*]] = alloca [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>], align 8
; CHECK: {{.*}} = alloca %struct.ndrange_t.6, align 8
; CHECK: {{.*}} = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
; CHECK: {{.*}} = alloca %struct.ndrange_t.6, align 8
; CHECK: [[PRIV_BC_ADDRS:%.*]] = bitcast [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>]* [[BLOCK_I_I_VEC]] to <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*
; CHECK: [[GEP1:%.*]] = getelementptr <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* [[PRIV_BC_ADDRS]], <2 x i32> <i32 0, i32 1>
; CHECK: [[BC:%.*]] = bitcast <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[GEP1]] to <2 x i8*>
; CHECK: [[E2:%.*]] = extractelement <2 x i8*> [[BC]], i32 1
; CHECK: [[E1:%.*]] = extractelement <2 x i8*> [[BC]], i32 0
; CHECK: call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull [[E1]]) #6
; CHECK: call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull [[E2]]) #6
; CHECK: [[XOR:%.*]] = xor <2 x i1> {{.*}}, <i1 true, i1 true>
; CHECK: [[GEP2:%.*]] = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[GEP1]], <2 x i64> zeroinitializer, <2 x i32> zeroinitializer
; CHECK: call void @llvm.masked.scatter.v2i32.v2p0i32(<2 x i32> <i32 28, i32 28>, <2 x i32*> [[GEP2]], i32 8, <2 x i1> {{.*}})
; CHECK: [[GEP3:%.*]] = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[GEP1]], <2 x i64> zeroinitializer, <2 x i32> <i32 1, i32 1>
; CHECK: call void @llvm.masked.scatter.v2i32.v2p0i32(<2 x i32> <i32 8, i32 8>, <2 x i32*> [[GEP3]], i32 4, <2 x i1> <i1 true, i1 true>)
; CHECK: [[GEP4:%.*]] = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[GEP1]], <2 x i64> zeroinitializer, <2 x i32> <i32 2, i32 2>
; CHECK: call void @llvm.masked.scatter.v2p4i8.v2p0p4i8(<2 x i8 addrspace(4)*> <i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*)>, <2 x i8 addrspace(4)**> [[GEP4]], i32 8, <2 x i1> <i1 true, i1 true>)


; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.queue_t.5 = type opaque
%struct.ndrange_t.6 = type { i32, [3 x i64], [3 x i64], [3 x i64] }

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare %opencl.queue_t.5* @_Z17get_default_queuev() local_unnamed_addr #2

; Function Attrs: convergent
declare void @_Z10ndrange_1Dm(%struct.ndrange_t.6* sret, i64) local_unnamed_addr #2

; Function Attrs: convergent nounwind
define internal void @__block_fn_block_invoke(i8 addrspace(4)* %.block_descriptor) #3 !ocl_recommended_vector_length !7 {
entry:
  %ndrange.i = alloca %struct.ndrange_t.6, align 8
  %block.i = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp.i = alloca %struct.ndrange_t.6, align 8
  %block.capture.addr = getelementptr inbounds i8, i8 addrspace(4)* %.block_descriptor, i64 16
  %0 = bitcast i8 addrspace(4)* %block.capture.addr to i32 addrspace(1)* addrspace(4)*
  %1 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %0, align 8, !tbaa !8
  %block.capture.addr1 = getelementptr inbounds i8, i8 addrspace(4)* %.block_descriptor, i64 24
  %2 = bitcast i8 addrspace(4)* %block.capture.addr1 to i32 addrspace(4)*
  %3 = load i32, i32 addrspace(4)* %2, align 8, !tbaa !12
  %4 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %4)
  %5 = bitcast %struct.ndrange_t.6* %tmp.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %5)
  %call.i = tail call i64 @_Z13get_global_idj(i32 0) #7
  %call1.i = tail call %opencl.queue_t.5* @_Z17get_default_queuev() #8
  %6 = bitcast %struct.ndrange_t.6* %ndrange.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %6) #6
  call void @_Z10ndrange_1Dm(%struct.ndrange_t.6* nonnull sret %ndrange.i, i64 3) #8
  %cmp.i = icmp slt i32 %3, 1
  br i1 %cmp.i, label %block_fn.exit, label %if.end.i

if.end.i:                                         ; preds = %entry
  %dec.i = add nsw i32 %3, -1
  %block.size.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 0
  store i32 28, i32* %block.size.i, align 8
  %block.align.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 1
  store i32 8, i32* %block.align.i, align 4
  %block.invoke.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i, align 8
  %block.captured.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 3
  store i32 addrspace(1)* %1, i32 addrspace(1)** %block.captured.i, align 8, !tbaa !8
  %block.captured2.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 4
  store i32 %dec.i, i32* %block.captured2.i, align 8, !tbaa !12
  %cmp3.i = icmp eq i64 %call.i, 1
  br i1 %cmp3.i, label %if.then4.i, label %block_fn.exit

if.then4.i:                                       ; preds = %if.end.i
  %arrayidx.i = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 1
  %7 = load i32, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  %inc.i = add nsw i32 %7, 1
  store i32 %inc.i, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %5, i8* nonnull align 8 %6, i64 80, i1 false) #6, !tbaa.struct !14
  %8 = bitcast i8* %4 to i8*
  %9 = addrspacecast i8* %8 to i8 addrspace(4)*
  %10 = call i32 @__enqueue_kernel_basic(%opencl.queue_t.5* %call1.i, i32 1, %struct.ndrange_t.6* byval nonnull %tmp.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* nonnull %9) #6
  %cmp5.i = icmp eq i32 %10, 0
  br i1 %cmp5.i, label %block_fn.exit, label %if.then6.i

if.then6.i:                                       ; preds = %if.then4.i
  store i32 -1, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  br label %block_fn.exit

block_fn.exit:                                    ; preds = %entry, %if.end.i, %if.then4.i, %if.then6.i
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %6) #6
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %4)
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %5)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #0

; Function Attrs: nounwind
define dso_local void @__block_fn_block_invoke_kernel(i8 addrspace(4)*) #4 !block_literal_size !16 !vectorized_kernel !17 !no_barrier_path !18 !vectorized_width !19 !scalarized_kernel !20 {
entry:
  %ndrange.i.i = alloca %struct.ndrange_t.6, align 8
  %block.i.i = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp.i.i = alloca %struct.ndrange_t.6, align 8
  %block.capture.addr.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 16
  %1 = bitcast i8 addrspace(4)* %block.capture.addr.i to i32 addrspace(1)* addrspace(4)*
  %2 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %1, align 8, !tbaa !8
  %block.capture.addr1.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 24
  %3 = bitcast i8 addrspace(4)* %block.capture.addr1.i to i32 addrspace(4)*
  %4 = load i32, i32 addrspace(4)* %3, align 8, !tbaa !12
  %5 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %5) #6
  %6 = bitcast %struct.ndrange_t.6* %tmp.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %6) #6
  %call.i.i = tail call i64 @_Z13get_global_idj(i32 0) #7
  %call1.i.i = tail call %opencl.queue_t.5* @_Z17get_default_queuev() #8
  %7 = bitcast %struct.ndrange_t.6* %ndrange.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #6
  call void @_Z10ndrange_1Dm(%struct.ndrange_t.6* nonnull sret %ndrange.i.i, i64 3) #8
  %cmp.i.i = icmp slt i32 %4, 1
  br i1 %cmp.i.i, label %__block_fn_block_invoke.exit, label %if.end.i.i

if.end.i.i:                                       ; preds = %entry
  %dec.i.i = add nsw i32 %4, -1
  %block.size.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 0
  store i32 28, i32* %block.size.i.i, align 8
  %block.align.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 1
  store i32 8, i32* %block.align.i.i, align 4
  %block.invoke.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i.i, align 8
  %block.captured.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 3
  store i32 addrspace(1)* %2, i32 addrspace(1)** %block.captured.i.i, align 8, !tbaa !8
  %block.captured2.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 4
  store i32 %dec.i.i, i32* %block.captured2.i.i, align 8, !tbaa !12
  %cmp3.i.i = icmp eq i64 %call.i.i, 1
  br i1 %cmp3.i.i, label %if.then4.i.i, label %__block_fn_block_invoke.exit

if.then4.i.i:                                     ; preds = %if.end.i.i
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %2, i64 1
  %8 = load i32, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  %inc.i.i = add nsw i32 %8, 1
  store i32 %inc.i.i, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %6, i8* nonnull align 8 %7, i64 80, i1 false) #6, !tbaa.struct !14
  %9 = bitcast i8* %5 to i8*
  %10 = addrspacecast i8* %9 to i8 addrspace(4)*
  %11 = call i32 @__enqueue_kernel_basic(%opencl.queue_t.5* %call1.i.i, i32 1, %struct.ndrange_t.6* byval nonnull %tmp.i.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* nonnull %10) #6
  %cmp5.i.i = icmp eq i32 %11, 0
  br i1 %cmp5.i.i, label %__block_fn_block_invoke.exit, label %if.then6.i.i

if.then6.i.i:                                     ; preds = %if.then4.i.i
  store i32 -1, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  br label %__block_fn_block_invoke.exit

__block_fn_block_invoke.exit:                     ; preds = %entry, %if.end.i.i, %if.then4.i.i, %if.then6.i.i
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #6
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %5) #6
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %6) #6
  ret void
}

declare i32 @__enqueue_kernel_basic(%opencl.queue_t.5*, i32, %struct.ndrange_t.6*, i8 addrspace(4)*, i8 addrspace(4)*) local_unnamed_addr

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: convergent nounwind
define void @enqueue_nested_blocks_single(i32 addrspace(1)* noalias %res, i32 %level) local_unnamed_addr #5 !kernel_arg_addr_space !21 !kernel_arg_access_qual !22 !kernel_arg_type !23 !kernel_arg_base_type !23 !kernel_arg_type_qual !24 !kernel_arg_host_accessible !25 !kernel_arg_pipe_depth !26 !kernel_arg_pipe_io !24 !kernel_arg_buffer_location !24 !kernel_arg_name !27 !vectorized_kernel !28 !no_barrier_path !18 !vectorized_width !19 !scalarized_kernel !20 {
entry:
  %ndrange.i = alloca %struct.ndrange_t.6, align 8
  %block.i = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp.i = alloca %struct.ndrange_t.6, align 8
  %0 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %0)
  %1 = bitcast %struct.ndrange_t.6* %tmp.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %1)
  %call.i = tail call i64 @_Z13get_global_idj(i32 0) #7
  %call1.i = tail call %opencl.queue_t.5* @_Z17get_default_queuev() #8
  %2 = bitcast %struct.ndrange_t.6* %ndrange.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %2) #6
  call void @_Z10ndrange_1Dm(%struct.ndrange_t.6* nonnull sret %ndrange.i, i64 3) #8
  %cmp.i = icmp slt i32 %level, 1
  br i1 %cmp.i, label %block_fn.exit, label %if.end.i

if.end.i:                                         ; preds = %entry
  %dec.i = add nsw i32 %level, -1
  %block.size.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 0
  store i32 28, i32* %block.size.i, align 8
  %block.align.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 1
  store i32 8, i32* %block.align.i, align 4
  %block.invoke.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i, align 8
  %block.captured.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 3
  store i32 addrspace(1)* %res, i32 addrspace(1)** %block.captured.i, align 8, !tbaa !8
  %block.captured2.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 4
  store i32 %dec.i, i32* %block.captured2.i, align 8, !tbaa !12
  %cmp3.i = icmp eq i64 %call.i, 1
  br i1 %cmp3.i, label %if.then4.i, label %block_fn.exit

if.then4.i:                                       ; preds = %if.end.i
  %arrayidx.i = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 1
  %3 = load i32, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  %inc.i = add nsw i32 %3, 1
  store i32 %inc.i, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %1, i8* nonnull align 8 %2, i64 80, i1 false) #6, !tbaa.struct !14
  %4 = bitcast i8* %0 to i8*
  %5 = addrspacecast i8* %4 to i8 addrspace(4)*
  %6 = call i32 @__enqueue_kernel_basic(%opencl.queue_t.5* %call1.i, i32 1, %struct.ndrange_t.6* byval nonnull %tmp.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* nonnull %5) #6
  %cmp5.i = icmp eq i32 %6, 0
  br i1 %cmp5.i, label %block_fn.exit, label %if.then6.i

if.then6.i:                                       ; preds = %if.then4.i
  store i32 -1, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  br label %block_fn.exit

block_fn.exit:                                    ; preds = %entry, %if.end.i, %if.then4.i, %if.then6.i
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %2) #6
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %0)
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %1)
  ret void
}

; Function Attrs: nounwind
define dso_local void @_ZGVdN8u___block_fn_block_invoke_kernel(i8 addrspace(4)*) #4 !block_literal_size !16 !vectorized_kernel !20 !no_barrier_path !18 !ocl_recommended_vector_length !7 !vectorized_width !7 !vectorization_dimension !6 !scalarized_kernel !29 !can_unite_workgroups !18 {
entry:
  %ndrange.i.i = alloca %struct.ndrange_t.6, align 8
  %block.i.i = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp.i.i = alloca %struct.ndrange_t.6, align 8
  %call.i.i = tail call i64 @_Z13get_global_idj(i32 0) #7
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i8 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(%struct.ndrange_t.6* %ndrange.i.i, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, %struct.ndrange_t.6* %tmp.i.i), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %1 = sext i32 %index to i64
  %add = add nuw i64 %1, %call.i.i
  %block.capture.addr.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 16
  %2 = bitcast i8 addrspace(4)* %block.capture.addr.i to i32 addrspace(1)* addrspace(4)*
  %3 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %2, align 8, !tbaa !8
  %block.capture.addr1.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 24
  %4 = bitcast i8 addrspace(4)* %block.capture.addr1.i to i32 addrspace(4)*
  %5 = load i32, i32 addrspace(4)* %4, align 8, !tbaa !12
  %6 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %6) #6
  %7 = bitcast %struct.ndrange_t.6* %tmp.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7) #6
  %call1.i.i = tail call %opencl.queue_t.5* @_Z17get_default_queuev() #8
  %8 = bitcast %struct.ndrange_t.6* %ndrange.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %8) #6
  call void @_Z10ndrange_1Dm(%struct.ndrange_t.6* nonnull sret %ndrange.i.i, i64 3) #8
  %cmp.i.i = icmp slt i32 %5, 1
  br i1 %cmp.i.i, label %__block_fn_block_invoke.exit, label %if.end.i.i

if.end.i.i:                                       ; preds = %simd.loop
  %dec.i.i = add nsw i32 %5, -1
  %block.size.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 0
  store i32 28, i32* %block.size.i.i, align 8
  %block.align.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 1
  store i32 8, i32* %block.align.i.i, align 4
  %block.invoke.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i.i, align 8
  %block.captured.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 3
  store i32 addrspace(1)* %3, i32 addrspace(1)** %block.captured.i.i, align 8, !tbaa !8
  %block.captured2.i.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i.i, i64 0, i32 4
  store i32 %dec.i.i, i32* %block.captured2.i.i, align 8, !tbaa !12
  %cmp3.i.i = icmp eq i64 %add, 1
  br i1 %cmp3.i.i, label %if.then4.i.i, label %__block_fn_block_invoke.exit

if.then4.i.i:                                     ; preds = %if.end.i.i
  %arrayidx.i.i = getelementptr inbounds i32, i32 addrspace(1)* %3, i64 1
  %9 = load i32, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  %inc.i.i = add nsw i32 %9, 1
  store i32 %inc.i.i, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %7, i8* nonnull align 8 %8, i64 80, i1 false) #6, !tbaa.struct !14
  %10 = bitcast i8* %6 to i8*
  %11 = addrspacecast i8* %10 to i8 addrspace(4)*
  %12 = call i32 @__enqueue_kernel_basic(%opencl.queue_t.5* %call1.i.i, i32 1, %struct.ndrange_t.6* byval nonnull %tmp.i.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* nonnull %11) #6
  %cmp5.i.i = icmp eq i32 %12, 0
  br i1 %cmp5.i.i, label %__block_fn_block_invoke.exit, label %if.then6.i.i

if.then6.i.i:                                     ; preds = %if.then4.i.i
  store i32 -1, i32 addrspace(1)* %arrayidx.i.i, align 4, !tbaa !12
  br label %__block_fn_block_invoke.exit

__block_fn_block_invoke.exit:                     ; preds = %if.then6.i.i, %if.then4.i.i, %if.end.i.i, %simd.loop
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %8) #6
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %6) #6
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7) #6
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %__block_fn_block_invoke.exit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !30

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #6

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #6

; Function Attrs: convergent nounwind
define void @_ZGVdN8uu_enqueue_nested_blocks_single(i32 addrspace(1)* noalias %res, i32 %level) local_unnamed_addr #5 !kernel_arg_addr_space !21 !kernel_arg_access_qual !22 !kernel_arg_type !23 !kernel_arg_base_type !23 !kernel_arg_type_qual !24 !kernel_arg_host_accessible !25 !kernel_arg_pipe_depth !26 !kernel_arg_pipe_io !24 !kernel_arg_buffer_location !24 !kernel_arg_name !27 !vectorized_kernel !20 !no_barrier_path !18 !ocl_recommended_vector_length !7 !vectorized_width !7 !vectorization_dimension !6 !scalarized_kernel !32 !can_unite_workgroups !18 {
entry:
  %ndrange.i = alloca %struct.ndrange_t.6, align 8
  %block.i = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp.i = alloca %struct.ndrange_t.6, align 8
  %call.i = tail call i64 @_Z13get_global_idj(i32 0) #7
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i32 addrspace(1)* %res, i32 %level), "QUAL.OMP.PRIVATE"(%struct.ndrange_t.6* %ndrange.i, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, %struct.ndrange_t.6* %tmp.i), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call.i
  %1 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %1)
  %2 = bitcast %struct.ndrange_t.6* %tmp.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %2)
  %call1.i = tail call %opencl.queue_t.5* @_Z17get_default_queuev() #8
  %3 = bitcast %struct.ndrange_t.6* %ndrange.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %3) #6
  call void @_Z10ndrange_1Dm(%struct.ndrange_t.6* nonnull sret %ndrange.i, i64 3) #8
  %cmp.i = icmp slt i32 %level, 1
  br i1 %cmp.i, label %block_fn.exit, label %if.end.i

if.end.i:                                         ; preds = %simd.loop
  %dec.i = add nsw i32 %level, -1
  %block.size.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 0
  store i32 28, i32* %block.size.i, align 8
  %block.align.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 1
  store i32 8, i32* %block.align.i, align 4
  %block.invoke.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 2
  store i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke to i8*) to i8 addrspace(4)*), i8 addrspace(4)** %block.invoke.i, align 8
  %block.captured.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 3
  store i32 addrspace(1)* %res, i32 addrspace(1)** %block.captured.i, align 8, !tbaa !8
  %block.captured2.i = getelementptr inbounds <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block.i, i64 0, i32 4
  store i32 %dec.i, i32* %block.captured2.i, align 8, !tbaa !12
  %cmp3.i = icmp eq i64 %add, 1
  br i1 %cmp3.i, label %if.then4.i, label %block_fn.exit

if.then4.i:                                       ; preds = %if.end.i
  %arrayidx.i = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 1
  %4 = load i32, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  %inc.i = add nsw i32 %4, 1
  store i32 %inc.i, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %2, i8* nonnull align 8 %3, i64 80, i1 false) #6, !tbaa.struct !14
  %5 = bitcast i8* %1 to i8*
  %6 = addrspacecast i8* %5 to i8 addrspace(4)*
  %7 = call i32 @__enqueue_kernel_basic(%opencl.queue_t.5* %call1.i, i32 1, %struct.ndrange_t.6* byval nonnull %tmp.i, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* nonnull %6) #6
  %cmp5.i = icmp eq i32 %7, 0
  br i1 %cmp5.i, label %block_fn.exit, label %if.then6.i

if.then6.i:                                       ; preds = %if.then4.i
  store i32 -1, i32 addrspace(1)* %arrayidx.i, align 4, !tbaa !12
  br label %block_fn.exit

block_fn.exit:                                    ; preds = %if.then6.i, %if.then4.i, %if.end.i, %simd.loop
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %3) #6
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %1)
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %2)
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %block_fn.exit
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !33

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

attributes #0 = { argmemonly nounwind }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "vector-variants"="_ZGVdN8u___block_fn_block_invoke_kernel" }
attributes #5 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8uu_enqueue_nested_blocks_single" }
attributes #6 = { nounwind }
attributes #7 = { convergent nounwind readnone }
attributes #8 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!5 = !{void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel, void (i32 addrspace(1)*, i32)* @enqueue_nested_blocks_single}
!6 = !{i32 0}
!7 = !{i32 8}
!8 = !{!9, !9, i64 0}
!9 = !{!"any pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}
!14 = !{i64 0, i64 4, !12, i64 8, i64 24, !15, i64 32, i64 24, !15, i64 56, i64 24, !15}
!15 = !{!10, !10, i64 0}
!16 = !{i32 28}
!17 = !{void (i8 addrspace(4)*)* @_ZGVdN8u___block_fn_block_invoke_kernel}
!18 = !{i1 false}
!19 = !{i32 1}
!20 = !{null}
!21 = !{i32 1, i32 0}
!22 = !{!"none", !"none"}
!23 = !{!"int*", !"int"}
!24 = !{!"", !""}
!25 = !{i1 false, i1 false}
!26 = !{i32 0, i32 0}
!27 = !{!"res", !"level"}
!28 = !{void (i32 addrspace(1)*, i32)* @_ZGVdN8uu_enqueue_nested_blocks_single}
!29 = !{void (i8 addrspace(4)*)* @__block_fn_block_invoke_kernel}
!30 = distinct !{!30, !31}
!31 = !{!"llvm.loop.unroll.disable"}
!32 = !{void (i32 addrspace(1)*, i32)* @enqueue_nested_blocks_single}
!33 = distinct !{!33, !31}
