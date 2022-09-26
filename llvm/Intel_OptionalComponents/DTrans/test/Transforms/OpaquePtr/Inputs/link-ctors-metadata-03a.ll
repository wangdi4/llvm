define void @g() {
  ret void
}

@u = linkonce global i8 41

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65534, ptr @g, ptr @u }], !intel_dtrans_type !1

!0 = !{i8 0, i32 1}
!1 = !{!"A", i32 1, !2}
!2 = !{!3, i32 0}
!3 = !{!"L", i32 3, !4, !5, !0}
!4 = !{i32 0, i32 0}
!5 = !{!6, i32 1}
!6 = !{!"F", i1 false, i32 0, !7}
!7 = !{!"void", i32 0}
