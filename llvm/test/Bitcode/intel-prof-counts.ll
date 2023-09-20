; RUN: opt -print-prof-counts -S < %s 2>&1 | FileCheck %s

; Verify that the profiling counts for branch weight values are printed
; beside instructions that contain profiling data.

; CHECK: define i32 @foo()
; CHECK: ; entry_count = 500
; CHECK: br i1 undef, label %then, label %else, !prof !1 ; Weights = 349 [69.8%], 151 [30.2%]
; CHECK: br i1 undef, label %then.a, label %merge{{$}}
; CHECK: switch i32 undef, label %end [
; CHECK: ], !prof !2 ; Weights = 487 [97.4%], 11 [2.2%], 0 [0.0%], 0 [0.0%], 2 [0.4%]
; CHECK: %res = select i1 undef, i32 0, i32 1, !prof !3 ; Weights = 43 [8.6%], 457 [91.4%]

define i32 @foo() !prof !0 {

    br i1 undef, label %then, label %else, !prof !1
then:
    ; A branch without profiling data should not get anything extra printed
    br i1 undef, label %then.a, label %merge
then.a:
    br label %merge
else:
    br label %merge

merge:
  switch i32 undef, label %end [
    i32 0, label %case0
    i32 7, label %case7
    i32 1, label %case1
    i32 4, label %case4
  ], !prof !2

case0:
    br label %end
case7:
    br label %end
case1:
    br label %end
case4:
    br label %end

end:
  %res = select i1 undef, i32 0, i32 1, !prof !3
  ret i32 %res
}

!0 = !{!"function_entry_count", i64 500}
!1 = !{!"branch_weights", i32 349, i32 151}
!2 = !{!"branch_weights", i32 487, i32 11, i32 0, i32 0, i32 2}
!3 = !{!"branch_weights", i32 43, i32 457}
