; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S %s 2>&1 | FileCheck %s

; The IR originates from the following pseudo-code:
;
; predicate = true;
; while (work_group_any(predicate)) {
;   predicated = false;
;   barrier();
; }
;
; where the return value of `work_group_any` crosses the barrier.

; For a uniform WG function (e.g. work_group_any), GroupBuiltin pass will
; create a `__finalize_` version to finalize the calculation result.
; The result should always be uniform in scope of the work group. In other
; words, the value should be classified as [Group-B.2] instead of [Group-B.1].

; CHECK: Group-B.1 Values
; CHECK-NOT: CallFinalizeWG
; CHECK: Group-B.2 Values
; CHECK-DAG: CallWGForItem
; CHECK-DAG: CallFinalizeWG

%"class.cl::sycl::id" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

define void @test(ptr addrspace(1) align 4 %_arg_r_acc, ptr byval(%"class.cl::sycl::id") align 8 %_arg_r_acc3) !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  call void @dummy_barrier.()
  %AllocaWGResult = alloca <4 x i32>, align 16
  store <4 x i32> zeroinitializer, ptr %AllocaWGResult, align 16
  br label %Split.Barrier.BB18

Split.Barrier.BB18:                               ; preds = %entry
  call void @dummy_barrier.()
  %i = tail call i64 @_Z13get_global_idj(i32 0)
  %i1 = getelementptr inbounds %"class.cl::sycl::id", ptr %_arg_r_acc3, i64 0, i32 0, i32 0, i64 0
  %i2 = load i64, ptr %i1, align 8
  %add.ptr.i = getelementptr inbounds i32, ptr addrspace(1) %_arg_r_acc, i64 %i2
  br label %VPlannedBB4

VPlannedBB4:                                      ; preds = %Split.Barrier.BB16, %Split.Barrier.BB18
  %uni.phi5 = phi i32 [ 1, %Split.Barrier.BB18 ], [ 0, %Split.Barrier.BB16 ]
  %broadcast.splatinsert6 = insertelement <4 x i32> poison, i32 %uni.phi5, i64 0
  %broadcast.splat7 = shufflevector <4 x i32> %broadcast.splatinsert6, <4 x i32> poison, <4 x i32> zeroinitializer
  %CallWGForItem = call <4 x i32> @_Z14work_group_anyDv4_iPS_(<4 x i32> %broadcast.splat7, ptr %AllocaWGResult)
  br label %Split.Barrier.BB

Split.Barrier.BB:                                 ; preds = %VPlannedBB4
  call void @_Z18work_group_barrierj(i32 1)
  %LoadWGFinalResult = load <4 x i32>, ptr %AllocaWGResult, align 16
  %CallFinalizeWG = call <4 x i32> @_Z25__finalize_work_group_anyDv4_i(<4 x i32> %LoadWGFinalResult)
  store <4 x i32> zeroinitializer, ptr %AllocaWGResult, align 16
  br label %Split.Barrier.BB17

Split.Barrier.BB17:                               ; preds = %Split.Barrier.BB
  call void @dummy_barrier.()
  %.copy = tail call i64 @_Z13get_global_idj(i32 0)
  %scalar.gep.copy = getelementptr inbounds i32, ptr addrspace(1) %add.ptr.i, i64 %.copy
  %.extract.0.8 = extractelement <4 x i32> %CallFinalizeWG, i64 0
  %i5 = icmp eq i32 %.extract.0.8, 0
  br i1 %i5, label %VPlannedBB12, label %VPlannedBB9

VPlannedBB9:                                      ; preds = %Split.Barrier.BB17
  %i6 = bitcast ptr addrspace(1) %scalar.gep.copy to ptr addrspace(1)
  store <4 x i32> <i32 1, i32 1, i32 1, i32 1>, ptr addrspace(1) %i6, align 4
  br label %Split.Barrier.BB16

Split.Barrier.BB16:                               ; preds = %VPlannedBB9
  tail call void @_Z18work_group_barrierj12memory_scope(i32 3, i32 1)
  br label %VPlannedBB4

VPlannedBB12:                                     ; preds = %Split.Barrier.BB17
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare i64 @_Z13get_global_idj(i32)

declare void @_Z18work_group_barrierj(i32)

declare void @dummy_barrier.()

declare <4 x i32> @_Z25__finalize_work_group_anyDv4_i(<4 x i32> noundef)

declare <4 x i32> @_Z14work_group_anyDv4_iPS_(<4 x i32> noundef, ptr nocapture noundef)

declare void @_Z18work_group_barrierj12memory_scope(i32, i32)

!0 = !{!"int*", !"class.cl::sycl::id"}
!1 = !{ptr addrspace(1) null, ptr null}
