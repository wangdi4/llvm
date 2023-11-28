; RUN: opt -passes=licm,loop-idiom -S < %s | FileCheck %s

%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

;CHECK: define void @_Test
;CHECK: WGLoopsEntry:
;CHECK: %.sroa.03.0.copyload = load i64, ptr %3, align 8
;CHECK-NEXT: %13 = getelementptr inbounds double, ptr addrspace(1) %0, i64 %.sroa.03.0.copyload
;CHECK-NEXT: %.sroa.0.0.copyload = load i64, ptr %7, align 8
;CHECK-NEXT: %14 = getelementptr inbounds double, ptr addrspace(1) %4, i64 %.sroa.0.0.copyload
;CHECK: call void @llvm.memcpy
;CHECK: scalar_kernel_entry
;CHECK-NOT: %.sroa.03.0.copyload = load i64, ptr %.sroa.03.0..sroa_idx, align 8
;CHECK-NOT: %13 = getelementptr inbounds double, ptr addrspace(1) %0, i64 %.sroa.03.0.copyload
;CHECK-NOT: %.sroa.0.0.copyload = load i64, ptr %.sroa.0.0..sroa_idx, align 8
;CHECK-NOT: %14 = getelementptr inbounds double, ptr addrspace(1) %4, i64 %.sroa.0.0.copyload
;CHECK: declare void @llvm.memcpy

; Function Attrs: nounwind
define void @_Test(ptr addrspace(1) noalias %0,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %1,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %2,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %3,
        ptr addrspace(1) noalias %4,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %5,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %6,
        ptr byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") %7,
        ptr addrspace(3) noalias %pLocalMemBase,
        ptr noalias %pWorkDim,
        ptr noalias %pWGId, [4 x i64] %BaseGlbId,
        ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) local_unnamed_addr !kernel_arg_base_type !1 !arg_type_null_val !2 {
WGLoopsEntry:
  %BaseGlobalID_0.i = extractvalue [4 x i64] %BaseGlbId, 0
  %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 4, i64 0
  %NumGroups_0.i = load i64, ptr %8, align 8
  %GroupID_0.i = load i64, ptr %pWGId, align 8
  %9 = add nsw i64 %GroupID_0.i, 1
  %10 = icmp eq i64 %NumGroups_0.i, %9
  %11 = zext i1 %10 to i64
  %12 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 3, i64 %11, i64 0
  %LocalSize_0.i = load i64, ptr %12, align 8
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %WGLoopsEntry
  %dim_0_ind_var = phi i64 [ 0, %WGLoopsEntry ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %BaseGlobalID_0.i, %WGLoopsEntry ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  %.sroa.03.0.copyload = load i64, ptr %3, align 8
  %13 = getelementptr inbounds double, ptr addrspace(1) %0, i64 %.sroa.03.0.copyload
  %.sroa.0.0.copyload = load i64, ptr %7, align 8
  %14 = getelementptr inbounds double, ptr addrspace(1) %4, i64 %.sroa.0.0.copyload
  %15 = getelementptr inbounds double, ptr addrspace(1) %14, i64 %dim_0_tid
  %16 = load i64, ptr addrspace(1) %15, align 8
  %17 = getelementptr inbounds double, ptr addrspace(1) %13, i64 %dim_0_tid
  store i64 %16, ptr addrspace(1) %17, align 8
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %LocalSize_0.i
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  ret void
}

!sycl.kernels = !{!0}
!0 = !{ptr @_Test}
!1 = !{!"double*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"double*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!2 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr addrspace(1) null, ptr null, ptr null, ptr null}
