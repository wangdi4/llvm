; RUN: %oclopt -ocl-loop-idiom < %s -S | FileCheck %s

target triple = "x86_64-pc-linux"

%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

define void @Test1(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
; CHECK-LABEL: wrapper_entry:
; CHECK: call void @llvm.memcpy.p1i8.p1i8.i64
; CHECK-NEXT: br i1 false
; CHECK-LABEL: scalar_kernel_entry.i:
; CHECK-NOT: %.sroa.04.0.copyload.i = load i64, i64* %18, align 8
wrapper_entry:
  %explicit_76 = alloca %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", align 8
  %explicit_33 = alloca %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", align 8
  %0 = bitcast i8* %pUniformArgs to double addrspace(1)**
  %explicit_0 = load double addrspace(1)*, double addrspace(1)** %0, align 8
  %1 = getelementptr i8, i8* %pUniformArgs, i64 24
  %2 = getelementptr i8, i8* %pUniformArgs, i64 32
  %3 = bitcast i8* %2 to double addrspace(1)**
  %explicit_4 = load double addrspace(1)*, double addrspace(1)** %3, align 8
  %4 = getelementptr i8, i8* %pUniformArgs, i64 56
  %5 = getelementptr i8, i8* %pUniformArgs, i64 64
  %pWorkDim = bitcast i8* %5 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*
  %6 = getelementptr i8, i8* %pUniformArgs, i64 120
  %7 = bitcast i8* %6 to i64*
  %LocalSize_0 = load i64, i64* %7, align 8
  %8 = getelementptr i8, i8* %pUniformArgs, i64 72
  %9 = bitcast i8* %8 to i64*
  %GlobalOffset_0 = load i64, i64* %9, align 8
  %GroupID_0 = load i64, i64* %pWGId, align 8
  %10 = mul i64 %LocalSize_0, %GroupID_0
  %11 = add i64 %10, %GlobalOffset_0
  %12 = bitcast %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_33 to i8*
  %13 = bitcast %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_76 to i8*
  %14 = bitcast i8* %4 to i64*
  %15 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_76, i64 0, i32 0, i32 0, i64 0
  %16 = load i64, i64* %14, align 1
  store i64 %16, i64* %15, align 8
  %17 = bitcast i8* %1 to i64*
  %18 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_33, i64 0, i32 0, i32 0, i64 0
  %19 = load i64, i64* %17, align 1
  store i64 %19, i64* %18, align 8
  %20 = getelementptr i8, i8* %pUniformArgs, i64 168
  %21 = bitcast i8* %20 to i64*
  %NumGroups_0.i.i = load i64, i64* %21, align 8
  %GroupID_0.i.i = load i64, i64* %pWGId, align 8
  %22 = add nsw i64 %GroupID_0.i.i, 1
  %23 = icmp eq i64 %NumGroups_0.i.i, %22
  %24 = zext i1 %23 to i64
  %25 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %24, i64 0
  %LocalSize_0.i.i = load i64, i64* %25, align 8
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:
  %dim_0_ind_var.i = phi i64 [ 0, %wrapper_entry ], [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ]
  %dim_0_tid.i = phi i64 [ %11, %wrapper_entry ], [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ]
  %.sroa.04.0.copyload.i = load i64, i64* %18, align 8
  %26 = getelementptr inbounds double, double addrspace(1)* %explicit_0, i64 %.sroa.04.0.copyload.i
  %.sroa.01.0.copyload.i = load i64, i64* %15, align 8
  %27 = getelementptr inbounds double, double addrspace(1)* %explicit_4, i64 %.sroa.01.0.copyload.i
  %28 = getelementptr inbounds double, double addrspace(1)* %27, i64 %dim_0_tid.i
  %29 = bitcast double addrspace(1)* %28 to i64 addrspace(1)*
  %30 = load i64, i64 addrspace(1)* %29, align 8
  %31 = getelementptr inbounds double, double addrspace(1)* %26, i64 %dim_0_tid.i
  %32 = bitcast double addrspace(1)* %31 to i64 addrspace(1)*
  store i64 %30, i64 addrspace(1)* %32, align 8
  %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
  %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %___ZTS4CopyIdE_separated_args.exit, label %scalar_kernel_entry.i

___ZTS4CopyIdE_separated_args.exit:
  ret void
}

define void @Test2(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
; CHECK-LABEL: wrapper_entry:
; CHECK: %23 = add i64 %20, 0
; CHECK: call void @llvm.memcpy.p1i8.p1i8.i64
; CHECK-NEXT: br i1 false
; CHECK-LABEL: scalar_kernel_entry.i:
; CHECK-NOT: %.sroa.04.0.copyload.i = load i64, i64* %18, align 8

wrapper_entry:
  %explicit_76 = alloca %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", align 8
  %explicit_33 = alloca %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", align 8
  %0 = bitcast i8* %pUniformArgs to double addrspace(1)**
  %explicit_0 = load double addrspace(1)*, double addrspace(1)** %0, align 8
  %1 = getelementptr i8, i8* %pUniformArgs, i64 24
  %2 = getelementptr i8, i8* %pUniformArgs, i64 32
  %3 = bitcast i8* %2 to double addrspace(1)**
  %explicit_4 = load double addrspace(1)*, double addrspace(1)** %3, align 8
  %4 = getelementptr i8, i8* %pUniformArgs, i64 56
  %5 = getelementptr i8, i8* %pUniformArgs, i64 64
  %pWorkDim = bitcast i8* %5 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*
  %6 = bitcast %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_33 to i8*
  %7 = bitcast %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_76 to i8*
  %8 = bitcast i8* %4 to i64*
  %9 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_76, i64 0, i32 0, i32 0, i64 0
  %10 = load i64, i64* %8, align 1
  store i64 %10, i64* %9, align 8
  %11 = bitcast i8* %1 to i64*
  %12 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %explicit_33, i64 0, i32 0, i32 0, i64 0
  %13 = load i64, i64* %11, align 1
  store i64 %13, i64* %12, align 8
  %14 = getelementptr i8, i8* %pUniformArgs, i64 168
  %15 = bitcast i8* %14 to i64*
  %NumGroups_0.i.i = load i64, i64* %15, align 8
  %GroupID_0.i.i = load i64, i64* %pWGId, align 8
  %16 = add nsw i64 %GroupID_0.i.i, 1
  %17 = icmp eq i64 %NumGroups_0.i.i, %16
  %18 = zext i1 %17 to i64
  %19 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64 0, i32 3, i64 %18, i64 0
  %LocalSize_0.i.i = load i64, i64* %19, align 8
  %20 = mul i64 %GroupID_0.i.i, 800000
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:
  %dim_0_ind_var.i = phi i64 [ 0, %wrapper_entry ], [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ]
  %dim_0_tid.i = phi i64 [ 0, %wrapper_entry ], [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ]
  %.sroa.03.0.copyload.i = load i64, i64* %12, align 8
  %21 = getelementptr inbounds double, double addrspace(1)* %explicit_0, i64 %.sroa.03.0.copyload.i
  %.sroa.0.0.copyload.i = load i64, i64* %9, align 8
  %22 = getelementptr inbounds double, double addrspace(1)* %explicit_4, i64 %.sroa.0.0.copyload.i
  %23 = add i64 %20, %dim_0_tid.i
  %24 = getelementptr inbounds double, double addrspace(1)* %22, i64 %23
  %25 = bitcast double addrspace(1)* %24 to i64 addrspace(1)*
  %26 = load i64, i64 addrspace(1)* %25, align 8
  %27 = getelementptr inbounds double, double addrspace(1)* %21, i64 %23
  %28 = bitcast double addrspace(1)* %27 to i64 addrspace(1)*
  store i64 %26, i64 addrspace(1)* %28, align 8
  %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %LocalSize_0.i.i
  %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %___ZTS4CopyIdE_separated_args.exit, label %scalar_kernel_entry.i

___ZTS4CopyIdE_separated_args.exit:
  ret void
}
