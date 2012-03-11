; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__Triad_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, float) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare [7 x i64] @__WG.boundaries.Triad_original(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare <8 x i32> @__local.avx256.pcmpeq.d_original(<8 x i32>, <8 x i32>)

declare <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32>, <4 x i32>) nounwind readnone

declare <8 x i32> @__local.avx256.pcmpgt.d_original(<8 x i32>, <8 x i32>)

declare <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32>, <4 x i32>) nounwind readnone

define i1 @allOne(i1 %pred) nounwind readnone {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %pred) nounwind readnone {
entry:
  %t = xor i1 %pred, true
  ret i1 %t
}

define void @__Triad_separated_args(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* nocapture %memC, float %s, i8 addrspace(3)* %pLocalMem, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64* %pWGId, <{ [4 x i64] }>* %pBaseGlbId, <{ [4 x i64] }>* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
WGLoopsEntry:
  %0 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr <{ [4 x i64] }>* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 1
  %5 = load i64* %4, align 8
  %6 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 2
  %7 = load i64* %6, align 8
  %vector.size = ashr i64 %1, 2
  %num.vector.wi = and i64 %1, -4
  %max.vector.gid = add i64 %num.vector.wi, %3
  %scalar.size = sub i64 %1, %num.vector.wi
  %8 = icmp eq i64 %vector.size, 0
  br i1 %8, label %scalarIf, label %dim_2_vector_pre_head

dim_2_vector_pre_head:                            ; preds = %WGLoopsEntry
  %tempvector_func = insertelement <4 x float> undef, float %s, i32 0
  %vectorvector_func = shufflevector <4 x float> %tempvector_func, <4 x float> undef, <4 x i32> zeroinitializer
  br label %dim_1_vector_pre_head

dim_1_vector_pre_head:                            ; preds = %dim_1_vector_exit, %dim_2_vector_pre_head
  %dim_2_vector_ind_var = phi i64 [ 0, %dim_2_vector_pre_head ], [ %dim_2_vector_inc_ind_var, %dim_1_vector_exit ]
  br label %dim_0_vector_pre_head

dim_0_vector_pre_head:                            ; preds = %dim_0_vector_exit, %dim_1_vector_pre_head
  %dim_1_vector_ind_var = phi i64 [ 0, %dim_1_vector_pre_head ], [ %dim_1_vector_inc_ind_var, %dim_0_vector_exit ]
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %entryvector_func ]
  %dim_0_vector_tid = phi i64 [ %3, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_tid, %entryvector_func ]
  %extract.lhs.lhsvector_func = shl i64 %dim_0_vector_tid, 32
  %extractvector_func = ashr exact i64 %extract.lhs.lhsvector_func, 32
  %9 = getelementptr inbounds float addrspace(1)* %memA, i64 %extractvector_func
  %ptrTypeCastvector_func = bitcast float addrspace(1)* %9 to <4 x float> addrspace(1)*
  %10 = load <4 x float> addrspace(1)* %ptrTypeCastvector_func, align 4
  %11 = getelementptr inbounds float addrspace(1)* %memB, i64 %extractvector_func
  %ptrTypeCast6vector_func = bitcast float addrspace(1)* %11 to <4 x float> addrspace(1)*
  %12 = load <4 x float> addrspace(1)* %ptrTypeCast6vector_func, align 4
  %mul7vector_func = fmul <4 x float> %12, %vectorvector_func
  %add8vector_func = fadd <4 x float> %10, %mul7vector_func
  %13 = getelementptr inbounds float addrspace(1)* %memC, i64 %extractvector_func
  %ptrTypeCast9vector_func = bitcast float addrspace(1)* %13 to <4 x float> addrspace(1)*
  store <4 x float> %add8vector_func, <4 x float> addrspace(1)* %ptrTypeCast9vector_func, align 4
  %dim_0_vector_inc_ind_var = add i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim_0_vector_inc_tid = add i64 %dim_0_vector_tid, 4
  br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

dim_0_vector_exit:                                ; preds = %entryvector_func
  %dim_1_vector_inc_ind_var = add i64 %dim_1_vector_ind_var, 1
  %dim_1_vector_cmp.to.max = icmp eq i64 %dim_1_vector_inc_ind_var, %5
  br i1 %dim_1_vector_cmp.to.max, label %dim_1_vector_exit, label %dim_0_vector_pre_head

dim_1_vector_exit:                                ; preds = %dim_0_vector_exit
  %dim_2_vector_inc_ind_var = add i64 %dim_2_vector_ind_var, 1
  %dim_2_vector_cmp.to.max = icmp eq i64 %dim_2_vector_inc_ind_var, %7
  br i1 %dim_2_vector_cmp.to.max, label %scalarIf, label %dim_1_vector_pre_head

scalarIf:                                         ; preds = %dim_1_vector_exit, %WGLoopsEntry
  %14 = icmp eq i64 %1, %num.vector.wi
  br i1 %14, label %ret, label %dim_1_pre_head

dim_1_pre_head:                                   ; preds = %scalarIf, %dim_1_exit
  %dim_2_ind_var = phi i64 [ %dim_2_inc_ind_var, %dim_1_exit ], [ 0, %scalarIf ]
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %dim_0_exit, %dim_1_pre_head
  %dim_1_ind_var = phi i64 [ 0, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ 0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %max.vector.gid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  %sext = shl i64 %dim_0_tid, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds float addrspace(1)* %memA, i64 %idxprom
  %15 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %memB, i64 %idxprom
  %16 = load float addrspace(1)* %arrayidx2, align 4
  %mul = fmul float %16, %s
  %add = fadd float %15, %mul
  %arrayidx4 = getelementptr inbounds float addrspace(1)* %memC, i64 %idxprom
  store float %add, float addrspace(1)* %arrayidx4, align 4
  %dim_0_inc_ind_var = add i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %scalar.size
  %dim_0_inc_tid = add i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  %dim_1_inc_ind_var = add i64 %dim_1_ind_var, 1
  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %5
  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head

dim_1_exit:                                       ; preds = %dim_0_exit
  %dim_2_inc_ind_var = add i64 %dim_2_ind_var, 1
  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %7
  br i1 %dim_2_cmp.to.max, label %ret, label %dim_1_pre_head

ret:                                              ; preds = %scalarIf, %dim_1_exit
  ret void
}

define [7 x i64] @WG.boundaries.Triad(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)* %pLocalMem, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64* %pWGId, <{ [4 x i64] }>* %pBaseGlbId, <{ [4 x i64] }>* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) {
entry:
  %4 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr <{ [4 x i64] }>* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 1
  %9 = load i64* %8, align 8
  %10 = getelementptr <{ [4 x i64] }>* %pBaseGlbId, i64 0, i32 0, i64 1
  %11 = load i64* %10, align 8
  %12 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64 0, i32 3, i64 2
  %13 = load i64* %12, align 8
  %14 = getelementptr <{ [4 x i64] }>* %pBaseGlbId, i64 0, i32 0, i64 2
  %15 = load i64* %14, align 8
  %16 = insertvalue [7 x i64] undef, i64 %5, 2
  %17 = insertvalue [7 x i64] %16, i64 %7, 1
  %18 = insertvalue [7 x i64] %17, i64 %9, 4
  %19 = insertvalue [7 x i64] %18, i64 %11, 3
  %20 = insertvalue [7 x i64] %19, i64 %13, 6
  %21 = insertvalue [7 x i64] %20, i64 %15, 5
  %22 = insertvalue [7 x i64] %21, i64 1, 0
  ret [7 x i64] %22
}

define <8 x i32> @local.avx256.pcmpeq.d(<8 x i32>, <8 x i32>, i8 addrspace(3)* %pLocalMem, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64* %pWGId, <{ [4 x i64] }>* %pBaseGlbId, <{ [4 x i64] }>* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

define <8 x i32> @local.avx256.pcmpgt.d(<8 x i32>, <8 x i32>, i8 addrspace(3)* %pLocalMem, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %pWorkDim, i64* %pWGId, <{ [4 x i64] }>* %pBaseGlbId, <{ [4 x i64] }>* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

define void @Triad(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float*
  %10 = load float* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to <{ [4 x i64] }>**
  %16 = load <{ [4 x i64] }>** %15, align 8
  %17 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %18 = load i64* %17, align 8
  %19 = getelementptr <{ [4 x i64] }>* %16, i64 0, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 1
  %22 = load i64* %21, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 2
  %24 = load i64* %23, align 8
  %vector.size.i = ashr i64 %18, 2
  %num.vector.wi.i = and i64 %18, -4
  %max.vector.gid.i = add i64 %num.vector.wi.i, %20
  %scalar.size.i = sub i64 %18, %num.vector.wi.i
  %25 = icmp eq i64 %vector.size.i, 0
  br i1 %25, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %tempvector_func.i = insertelement <4 x float> undef, float %10, i32 0
  %vectorvector_func.i = shufflevector <4 x float> %tempvector_func.i, <4 x float> undef, <4 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %entryvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %entryvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %20, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %entryvector_func.i ]
  %extract.lhs.lhsvector_func.i = shl i64 %dim_0_vector_tid.i, 32
  %extractvector_func.i = ashr exact i64 %extract.lhs.lhsvector_func.i, 32
  %26 = getelementptr inbounds float addrspace(1)* %1, i64 %extractvector_func.i
  %ptrTypeCastvector_func.i = bitcast float addrspace(1)* %26 to <4 x float> addrspace(1)*
  %27 = load <4 x float> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %28 = getelementptr inbounds float addrspace(1)* %4, i64 %extractvector_func.i
  %ptrTypeCast6vector_func.i = bitcast float addrspace(1)* %28 to <4 x float> addrspace(1)*
  %29 = load <4 x float> addrspace(1)* %ptrTypeCast6vector_func.i, align 4
  %mul7vector_func.i = fmul <4 x float> %29, %vectorvector_func.i
  %add8vector_func.i = fadd <4 x float> %27, %mul7vector_func.i
  %30 = getelementptr inbounds float addrspace(1)* %7, i64 %extractvector_func.i
  %ptrTypeCast9vector_func.i = bitcast float addrspace(1)* %30 to <4 x float> addrspace(1)*
  store <4 x float> %add8vector_func.i, <4 x float> addrspace(1)* %ptrTypeCast9vector_func.i, align 4
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 4
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %entryvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %22
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %24
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %31 = icmp eq i64 %18, %num.vector.wi.i
  br i1 %31, label %__Triad_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %scalar_kernel_entry.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ]
  %sext.i = shl i64 %dim_0_tid.i, 32
  %idxprom.i = ashr exact i64 %sext.i, 32
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom.i
  %32 = load float addrspace(1)* %arrayidx.i, align 4
  %arrayidx2.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom.i
  %33 = load float addrspace(1)* %arrayidx2.i, align 4
  %mul.i = fmul float %33, %10
  %add.i = fadd float %32, %mul.i
  %arrayidx4.i = getelementptr inbounds float addrspace(1)* %7, i64 %idxprom.i
  store float %add.i, float addrspace(1)* %arrayidx4.i, align 4
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %scalar_kernel_entry.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %22
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %24
  br i1 %dim_2_cmp.to.max.i, label %__Triad_separated_args.exit, label %dim_1_pre_head.i

__Triad_separated_args.exit:                      ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__Triad_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"Triad"}
!4 = metadata !{void (i8*)* @Triad}
