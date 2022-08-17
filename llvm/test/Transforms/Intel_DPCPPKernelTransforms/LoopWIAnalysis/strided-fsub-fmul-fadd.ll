; RUN: opt -passes='print<dpcpp-kernel-loop-wi-analysis>' -S %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -dpcpp-kernel-loop-wi-analysis -analyze -enable-new-pm=0 -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-DAG: STR    %dim_0_vector_ind_var = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %entryvector_func ]
; CHECK-DAG: STR    %dim0__tid = phi i64 [ %0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
; CHECK-DAG: STR    %3 = trunc i64 %dim0__tid to i32
; CHECK-DAG: RND    %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %3, i32 0
; CHECK-DAG: STR    %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK-DAG: STR    %4 = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-DAG: UNI  i64 32
; CHECK-DAG: STR    %sextvector_func = shl i64 %dim0__tid, 32
; CHECK-DAG: RND    %.extract.0.vector_func = ashr exact i64 %sextvector_func, 32
; CHECK-DAG: STR    %5 = sitofp <8 x i32> %4 to <8 x float>
; CHECK-DAG: UNI  <8 x float> <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
; CHECK-DAG: STR    %6 = fsub <8 x float> %5, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
; CHECK-DAG: UNI  <8 x float> <float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000>
; CHECK-DAG: STR    %7 = fmul <8 x float> %6, <float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000>
; CHECK-DAG: UNI  <8 x float> <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
; CHECK-DAG: STR    %8 = fadd <8 x float> %7, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
; CHECK-DAG: RND    %scalar.gepvector_func = getelementptr inbounds float, float addrspace(1)* %kb, i64 %.extract.0.vector_func
; CHECK-DAG: RND    %9 = bitcast float addrspace(1)* %scalar.gepvector_func to <8 x float> addrspace(1)*
; CHECK-DAG: RND    store <8 x float> %8, <8 x float> addrspace(1)* %9, align 4
; CHECK-DAG: UNI  i64 1
; CHECK-DAG: STR    %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
; CHECK-DAG: RND    %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
; CHECK-DAG: UNI  i64 8
; CHECK-DAG: STR    %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8
; CHECK-DAG: RND    br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

define dso_local void @test(float addrspace(1)* %kb) local_unnamed_addr #0 {
vect_if:
  %0 = call i64 @get_base_global_id.(i32 0)
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %vector.size = ashr i64 %1, 3
  %2 = icmp ne i64 %vector.size, 0
  br label %dim_0_vector_pre_head

dim_0_vector_pre_head:                            ; preds = %vect_if
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %entryvector_func ]
  %dim0__tid = phi i64 [ %0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
  %3 = trunc i64 %dim0__tid to i32
  %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %3, i32 0
  %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> poison, <8 x i32> zeroinitializer
  %4 = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %sextvector_func = shl i64 %dim0__tid, 32
  %.extract.0.vector_func = ashr exact i64 %sextvector_func, 32
  %5 = sitofp <8 x i32> %4 to <8 x float>
  %6 = fsub <8 x float> %5, <float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00, float -2.000000e+00>
  %7 = fmul <8 x float> %6, <float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000>
  %8 = fadd <8 x float> %7, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  %scalar.gepvector_func = getelementptr inbounds float, float addrspace(1)* %kb, i64 %.extract.0.vector_func
  %9 = bitcast float addrspace(1)* %scalar.gepvector_func to <8 x float> addrspace(1)*
  store <8 x float> %8, <8 x float> addrspace(1)* %9, align 4
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8
  br i1 %dim_0_vector_cmp.to.max, label %dim_0_vector_exit, label %entryvector_func

dim_0_vector_exit:                                ; preds = %entryvector_func
  br label %ret

ret:                                              ; preds = %dim_0_vector_exit
  ret void
}

declare i64 @get_base_global_id.(i32)

declare i64 @_Z14get_local_sizej(i32)

attributes #0 = { convergent norecurse nounwind }

!sycl.kernels = !{!0}

!0 = !{void (float addrspace(1)*)* @test}
