; RUN: opt -passes=sycl-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; %3 is used for llvm.assume only, so it isn't hoisted.
; CHECK-LABEL: @not_hoist
; CHECK-NOT: strided.add

define void @not_hoist() local_unnamed_addr #0 {
WGLoopsEntry:
  %0 = call i64 @_Z14get_local_sizej(i32 0) #0
  %1 = call i64 @get_base_global_id.(i32 0) #0
  %2 = and i64 %0, -16
  %max.vector.gid = add i64 %2, %1
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %WGLoopsEntry
  %dim_0_vector_ind_var = phi i64 [ %dim_0_vector_inc_ind_var, %entryvector_func ], [ %1, %WGLoopsEntry ]
  %broadcast.splatinsertvector_func = insertelement <16 x i64> poison, i64 %dim_0_vector_ind_var, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i64> %broadcast.splatinsertvector_func, <16 x i64> poison, <16 x i32> zeroinitializer
  %3 = add nuw <16 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %4 = icmp ult <16 x i64> %3, <i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648>
  %.extract.15.vector_func = extractelement <16 x i1> %4, i64 15
  %.extract.14.vector_func = extractelement <16 x i1> %4, i64 14
  %.extract.13.vector_func = extractelement <16 x i1> %4, i64 13
  %.extract.12.vector_func = extractelement <16 x i1> %4, i64 12
  %.extract.11.vector_func = extractelement <16 x i1> %4, i64 11
  %.extract.10.vector_func = extractelement <16 x i1> %4, i64 10
  %.extract.9.vector_func = extractelement <16 x i1> %4, i64 9
  %.extract.8.vector_func = extractelement <16 x i1> %4, i64 8
  %.extract.7.vector_func = extractelement <16 x i1> %4, i64 7
  %.extract.6.vector_func = extractelement <16 x i1> %4, i64 6
  %.extract.5.vector_func = extractelement <16 x i1> %4, i64 5
  %.extract.4.vector_func = extractelement <16 x i1> %4, i64 4
  %.extract.3.vector_func = extractelement <16 x i1> %4, i64 3
  %.extract.2.vector_func = extractelement <16 x i1> %4, i64 2
  %.extract.1.vector_func = extractelement <16 x i1> %4, i64 1
  %.extract.0.vector_func = extractelement <16 x i1> %4, i64 0
  tail call void @llvm.assume(i1 %.extract.0.vector_func)
  tail call void @llvm.assume(i1 %.extract.1.vector_func)
  tail call void @llvm.assume(i1 %.extract.2.vector_func)
  tail call void @llvm.assume(i1 %.extract.3.vector_func)
  tail call void @llvm.assume(i1 %.extract.4.vector_func)
  tail call void @llvm.assume(i1 %.extract.5.vector_func)
  tail call void @llvm.assume(i1 %.extract.6.vector_func)
  tail call void @llvm.assume(i1 %.extract.7.vector_func)
  tail call void @llvm.assume(i1 %.extract.8.vector_func)
  tail call void @llvm.assume(i1 %.extract.9.vector_func)
  tail call void @llvm.assume(i1 %.extract.10.vector_func)
  tail call void @llvm.assume(i1 %.extract.11.vector_func)
  tail call void @llvm.assume(i1 %.extract.12.vector_func)
  tail call void @llvm.assume(i1 %.extract.13.vector_func)
  tail call void @llvm.assume(i1 %.extract.14.vector_func)
  tail call void @llvm.assume(i1 %.extract.15.vector_func)
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 16
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %max.vector.gid
  br i1 %dim_0_vector_cmp.to.max, label %exit, label %entryvector_func

exit:                                             ; preds = %entryvector_func
  ret void
}

; %3 has two users and one of them is used for llvm.assume only. %3 is hoisted.

define void @hoist_two_users() local_unnamed_addr #0 {
WGLoopsEntry:
; CHECK-LABEL: @hoist_two_users
; CHECK-LABEL: WGLoopsEntry:
; CHECK:      %broadcast.splatinsertvector_func = insertelement <4 x i64> poison, i64 {{.*}}, i64 0
; CHECK-NEXT: %broadcast.splatvector_func = shufflevector <4 x i64> %broadcast.splatinsertvector_func, <4 x i64> poison, <4 x i32> zeroinitializer
; CHECK-NEXT: add nuw <4 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3>

  %0 = call i64 @_Z14get_local_sizej(i32 0) #0
  %1 = call i64 @get_base_global_id.(i32 0) #0
  %2 = and i64 %0, -4
  %max.vector.gid = add i64 %2, %1
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %WGLoopsEntry
; CHECK-LABEL: entryvector_func:
; CHECK: phi <4 x i64> [ {{.*}}, %WGLoopsEntry ], [ %strided.add, %entryvector_func ]
  %dim_0_vector_ind_var = phi i64 [ %dim_0_vector_inc_ind_var, %entryvector_func ], [ %1, %WGLoopsEntry ]
  %broadcast.splatinsertvector_func = insertelement <4 x i64> poison, i64 %dim_0_vector_ind_var, i64 0
  %broadcast.splatvector_func = shufflevector <4 x i64> %broadcast.splatinsertvector_func, <4 x i64> poison, <4 x i32> zeroinitializer
  %3 = add nuw <4 x i64> %broadcast.splatvector_func, <i64 0, i64 1, i64 2, i64 3>
  %4 = icmp ult <4 x i64> %3, <i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648>
  %.extract.3.vector_func = extractelement <4 x i1> %4, i64 3
  %.extract.2.vector_func = extractelement <4 x i1> %4, i64 2
  %.extract.1.vector_func = extractelement <4 x i1> %4, i64 1
  %.extract.0.vector_func = extractelement <4 x i1> %4, i64 0
  tail call void @llvm.assume(i1 %.extract.0.vector_func)
  tail call void @llvm.assume(i1 %.extract.1.vector_func)
  tail call void @llvm.assume(i1 %.extract.2.vector_func)
  tail call void @llvm.assume(i1 %.extract.3.vector_func)
  %5 = trunc <4 x i64> %3 to <4 x i32>
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 4
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %max.vector.gid
  br i1 %dim_0_vector_cmp.to.max, label %exit, label %entryvector_func

exit:                                             ; preds = %entryvector_func
  ret void
}

; %4 has two users and one of them is used for llvm.assume only. %4 is hoisted.

define void @hoist_trunc_two_users() local_unnamed_addr #0 {
WGLoopsEntry:
; CHECK-LABEL: hoist_trunc_two_users
; CHECK-LABEL: WGLoopsEntry:
; CHECK:      %broadcast.splatinsertvector_func = insertelement <4 x i32> poison, i32 %3, i64 0
; CHECK-NEXT: %broadcast.splatvector_func = shufflevector <4 x i32> %broadcast.splatinsertvector_func, <4 x i32> poison, <4 x i32> zeroinitializer
; CHECK-NEXT: %4 = add nuw <4 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3>

  %0 = call i64 @_Z14get_local_sizej(i32 0) #0
  %1 = call i64 @get_base_global_id.(i32 0) #0
  %2 = and i64 %0, -4
  %max.vector.gid = add i64 %2, %1
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %WGLoopsEntry
; CHECK-LABEL: entryvector_func:
; CHECK: phi <4 x i32> [ {{.*}}, %WGLoopsEntry ], [ %strided.add, %entryvector_func ]

  %dim_0_vector_ind_var = phi i64 [ %dim_0_vector_inc_ind_var, %entryvector_func ], [ %1, %WGLoopsEntry ]
  %3 = trunc i64 %dim_0_vector_ind_var to i32
  %broadcast.splatinsertvector_func = insertelement <4 x i32> poison, i32 %3, i64 0
  %broadcast.splatvector_func = shufflevector <4 x i32> %broadcast.splatinsertvector_func, <4 x i32> poison, <4 x i32> zeroinitializer
  %4 = add nuw <4 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3>
  %5 = sext <4 x i32> %4 to <4 x i64>
  %6 = icmp ult <4 x i64> %5, <i64 2147483648, i64 2147483648, i64 2147483648, i64 2147483648>
  %.extract.3.vector_func = extractelement <4 x i1> %6, i64 3
  %.extract.2.vector_func = extractelement <4 x i1> %6, i64 2
  %.extract.1.vector_func = extractelement <4 x i1> %6, i64 1
  %.extract.0.vector_func = extractelement <4 x i1> %6, i64 0
  tail call void @llvm.assume(i1 %.extract.0.vector_func)
  tail call void @llvm.assume(i1 %.extract.1.vector_func)
  tail call void @llvm.assume(i1 %.extract.2.vector_func)
  tail call void @llvm.assume(i1 %.extract.3.vector_func)
  %7 = add <4 x i32> %4, %4
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 4
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %max.vector.gid
  br i1 %dim_0_vector_cmp.to.max, label %exit, label %entryvector_func

exit:                                             ; preds = %entryvector_func
  ret void
}

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { nounwind }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }

!spirv.MemoryModel = !{!0}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.optional.core.features = !{!4}
!spirv.Generator = !{!5}
!sycl.kernels = !{}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{!"cl_doubles"}
!5 = !{i16 6, i16 14}

; DEBUGIFY-NOT: WARNING
