; This test is to check i2101 type is supported by CoerceWin64Types pass.

; RUN: opt -passes='debugify,dpcpp-kernel-coerce-win64-types,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-coerce-win64-types' -S %s | FileCheck %s

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1


; CHECK: define spir_func void @_ZN10ac_private17ap_equal_zeros_toILi1064ELi2101EEEbU7_ExtIntIXT0_EEi(i2101* %{{.*}})


; Function Attrs: nounwind
define spir_func void @_ZN10ac_private17ap_equal_zeros_toILi1064ELi2101EEEbU7_ExtIntIXT0_EEi(i2101* byval(i2101) align 8 %0) #0 {
  %2 = alloca i1, align 1
  %3 = addrspacecast i1* %2 to i1 addrspace(4)*
  %4 = alloca i2101, align 8
  %5 = addrspacecast i2101* %4 to i2101 addrspace(4)*
  %6 = alloca i2101, align 8
  %7 = addrspacecast i2101* %6 to i2101 addrspace(4)*
  %8 = alloca i2101, align 8
  %9 = alloca i32, align 4
  %10 = load i2101, i2101* %0, align 8
  store i2101 %10, i2101 addrspace(4)* %5, align 8
  %11 = bitcast i2101* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 264, i8* %11)
  %12 = load i2101, i2101 addrspace(4)* %5, align 8
  store i2101 %12, i2101 addrspace(4)* %7, align 8
  %13 = load i2101, i2101 addrspace(4)* %7, align 8
  store i2101 %13, i2101* %8, align 8
  %14 = bitcast i2101* %6 to i8*
  call void @llvm.lifetime.end.p0i8(i64 264, i8* %14)
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!spirv.MemoryModel = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!5}
!spirv.Generator = !{!6}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{i16 6, i16 14}

; DEBUGIFY-NOT: WARNING
