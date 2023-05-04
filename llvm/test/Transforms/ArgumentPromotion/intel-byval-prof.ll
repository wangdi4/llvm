; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; This test is to verify that functions and calls created by argument
; promotion to replace the original function get the profile count
; information set.

%struct.ss = type { i32, i64 }

define internal void @f(ptr byval(%struct.ss) align 8 %b) !prof !0 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

; CHECK-LABEL: define internal void @f(i32 %b.0.val) !prof !0
; CHECK: add i32 %b.0.val

define internal void @g(ptr byval(%struct.ss) align 32 %b) !prof !1 {
entry:
  %tmp = getelementptr %struct.ss, ptr %b, i32 0, i32 0
  %tmp1 = load i32, ptr %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, ptr %tmp, align 4
  ret void
}

; CHECK-LABEL: define internal void @g(i32 %b.0.val) !prof !1
; CHECK: add i32 %b.0.val

define i32 @main() nounwind  {
entry:
  %S = alloca %struct.ss
  %tmp1 = getelementptr %struct.ss, ptr %S, i32 0, i32 0
  store i32 1, ptr %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, ptr %S, i32 0, i32 1
  store i64 2, ptr %tmp4, align 4
  br i1 undef, label %path1, label %path2
path1:
  call void @f(ptr byval(%struct.ss) %S), !intel-profx !2
  br label %exit
path2:
  call void @g(ptr byval(%struct.ss) %S), !intel-profx !3
  br label %exit
exit:
  ret i32 0
}

; CHECK-LABEL: define i32 @main
; CHECK: call void @f(i32 {{.*}}), !intel-profx !2
; CHECK: call void @g(i32 %{{.*}}), !intel-profx !3

!0 = !{!"function_entry_count", i64 0}
!1 = !{!"function_entry_count", i64 10000}
!2 = !{!"intel_profx", i64 0}
!3 = !{!"intel_profx", i64 10000}

; CHECK: !0 = !{!"function_entry_count", i64 0}
; CHECK: !1 = !{!"function_entry_count", i64 10000}
; CHECK: !2 = !{!"intel_profx", i64 0}
; CHECK: !3 = !{!"intel_profx", i64 10000}
