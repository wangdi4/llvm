; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK-NONOPAQUE %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK-NONOPAQUE %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK-OPAQUE %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK-OPAQUE %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define i32 @add(i32 %A, i32 %B) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %B, %A
  ret i32 %add
}

define i32 @sub(i32 %A, i32 %B) local_unnamed_addr #0 {
entry:
  %sub = sub nsw i32 %A, %B
  ret i32 %sub
}

; Function Attrs: nounwind
define void @test() local_unnamed_addr #1 {
entry:
  %0 = call i64 @get_base_global_id.(i32 0)
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %vector.size = ashr i64 %1, 3
  %2 = icmp ne i64 %vector.size, 0
  br i1 %2, label %entryvector_func, label %ret

entryvector_func:                                 ; preds = %entryvector_func, %entry
  %dim_0_vector_ind_var = phi i64 [ 0, %entry ], [ %dim_0_vector_inc_ind_var, %entryvector_func ]
  %dim0__tid = phi i64 [ %0, %entry ], [ %dim0_inc_tid, %entryvector_func ]
  %3 = and i64 %dim0__tid, 1
  %4 = icmp eq i64 %3, 0
  %broadcast.splatinsert8vector_func = insertelement <2 x i1> poison, i1 %4, i32 0
  %broadcast.splat9vector_func = shufflevector <2 x i1> %broadcast.splatinsert8vector_func, <2 x i1> poison, <2 x i32> zeroinitializer

; CHECK-NONOPAQUE:  = select <2 x i1> %broadcast.splat9vector_func, <2 x i32 (i32, i32)*> <i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @add to i32 (i32, i32)*), i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @add to i32 (i32, i32)*)>, <2 x i32 (i32, i32)*> <i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @sub to i32 (i32, i32)*), i32 (i32, i32)* bitcast (i32 (i32, i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @sub to i32 (i32, i32)*)>
; CHECK-OPAQUE:  = select <2 x i1> %broadcast.splat9vector_func, <2 x ptr> <ptr @add, ptr @add>, <2 x ptr> <ptr @sub, ptr @sub>

  %5 = select <2 x i1> %broadcast.splat9vector_func, <2 x i32 (i32, i32)*> <i32 (i32, i32)* @add, i32 (i32, i32)* @add>, <2 x i32 (i32, i32)*> <i32 (i32, i32)* @sub, i32 (i32, i32)* @sub>
  %.extract.1.vector_func = extractelement <2 x i32 (i32, i32)*> %5, i32 1
  %.extract.0.vector_func = extractelement <2 x i32 (i32, i32)*> %5, i32 0
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 1
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim_0_vector_inc_ind_var, %vector.size
  %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 2
  br i1 %dim_0_vector_cmp.to.max, label %ret, label %entryvector_func

ret:                                              ; preds = %entryvector_func, %entry
  ret void
}

declare i64 @get_base_global_id.(i32)

declare i64 @_Z14get_local_sizej(i32)

attributes #0 = { nofree norecurse nosync nounwind readnone willreturn mustprogress "referenced-indirectly" }
attributes #1 = { nounwind }

!sycl.kernels = !{!0}

!0 = distinct !{null, void ()* @test}

; DEBUGIFY-NOT: WARNING
