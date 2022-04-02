; RUN: opt -passes=openmp-opt -S < %s | FileCheck %s

; CHECK: define spir_kernel void @outer()
; CHECK-NOT: call spir_func void @inner1.internalized()
; CHECK-NOT: call spir_kernel void @inner2.internalized()

target device_triples = "spir64"

define spir_kernel void @outer() {
entry:
  call spir_func void @inner1()
  call spir_kernel void @inner2()
  ret void
}

define linkonce_odr spir_func void @inner1() {
entry:
  ret void
}

define linkonce_odr spir_func void @inner2() {
entry:
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { argmemonly nofree nounwind willreturn }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
