; RUN: opt < %s -argpromotion -S | FileCheck %s
; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; This test is to verify that functions and calls created by argument
; promotion to replace the original function get the profile count
; information set.

%struct.ss = type { i32, i64 }

define internal void @f(%struct.ss* byval %b) !prof !0 {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

; CHECK-LABEL: define internal void @f(i32 %b.0, i64 %b.1) !prof !0
; CHECK: alloca %struct.ss, align 8{{$}}
; CHECK: store i32 %b.0
; CHECK: store i64 %b.1

define internal void @g(%struct.ss* byval align 32 %b) !prof !1 {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

; CHECK-LABEL: define internal void @g(i32 %b.0, i64 %b.1) !prof !1
; CHECK: alloca %struct.ss, align 32
; CHECK: store i32 %b.0
; CHECK: store i64 %b.1

define i32 @main() nounwind  {
entry:
  %S = alloca %struct.ss
  %tmp1 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 0
  store i32 1, i32* %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 1
  store i64 2, i64* %tmp4, align 4
  br i1 undef, label %path1, label %path2
path1:
  call void @f(%struct.ss* byval %S), !intel-profx !2
  br label %exit
path2:
  call void @g(%struct.ss* byval %S), !intel-profx !3
  br label %exit
exit:
  ret i32 0
}

; CHECK-LABEL: define i32 @main
; CHECK: call void @f(i32 %{{.*}}, i64 %{{.*}}), !intel-profx !2
; CHECK: call void @g(i32 %{{.*}}, i64 %{{.*}}), !intel-profx !3

!0 = !{!"function_entry_count", i64 0}
!1 = !{!"function_entry_count", i64 10000}
!2 = !{!"intel_profx", i64 0}
!3 = !{!"intel_profx", i64 10000}

; CHECK: !0 = !{!"function_entry_count", i64 0}
; CHECK: !1 = !{!"function_entry_count", i64 10000}
; CHECK: !2 = !{!"intel_profx", i64 0}
; CHECK: !3 = !{!"intel_profx", i64 10000}
