; This test is to check i2101 type is supported by CoerceWin64Types pass.

; RUN: opt -passes='debugify,sycl-kernel-coerce-win64-types,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-coerce-win64-types' -S %s | FileCheck %s

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1


; CHECK: define spir_func void @_ZN10ac_private17ap_equal_zeros_toILi1064ELi2101EEEbU7_ExtIntIXT0_EEi(ptr %{{.*}})


; Function Attrs: nounwind
define spir_func void @_ZN10ac_private17ap_equal_zeros_toILi1064ELi2101EEEbU7_ExtIntIXT0_EEi(ptr byval(i2101) align 8 %0) #0 {
  %2 = alloca i1, align 1
  %3 = addrspacecast ptr %2 to ptr addrspace(4)
  %4 = alloca i2101, align 8
  %5 = addrspacecast ptr %4 to ptr addrspace(4)
  %6 = alloca i2101, align 8
  %7 = addrspacecast ptr %6 to ptr addrspace(4)
  %8 = alloca i2101, align 8
  %9 = alloca i32, align 4
  %10 = load i2101, ptr %0, align 8
  store i2101 %10, ptr addrspace(4) %5, align 8
  call void @llvm.lifetime.start.p0(i64 264, ptr %6)
  %11 = load i2101, ptr addrspace(4) %5, align 8
  store i2101 %11, ptr addrspace(4) %7, align 8
  %12 = load i2101, ptr addrspace(4) %7, align 8
  store i2101 %12, ptr %8, align 8
  call void @llvm.lifetime.end.p0(i64 264, ptr %6)
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
