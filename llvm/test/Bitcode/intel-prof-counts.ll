; RUN: opt -print-prof-counts -S < %s 2>&1 | FileCheck %s

; Verify that the profiling counts for branch weight values are printed
; beside instructions that contain profiling data.

; CHECK: br i1 undef, label %then, label %else, !prof !0 ; Weights = 349, 151
; CHECK: br i1 undef, label %then.a, label %merge{{$}}
; CHECK: switch i32 undef, label %end [
; CHECK: ], !prof !1 ; Weights = 487, 11, 0, 0, 2
; CHECK: %res = select i1 undef, i32 0, i32 1, !prof !2 ; Weights = 43, 457

define i32 @foo() {
    br i1 undef, label %then, label %else, !prof !0
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
  ], !prof !1

case0:
    br label %end
case7:
    br label %end
case1:
    br label %end
case4:
    br label %end

end:
  %res = select i1 undef, i32 0, i32 1, !prof !2
  ret i32 %res
}

!0 = !{!"branch_weights", i32 349, i32 151}
!1 = !{!"branch_weights", i32 487, i32 11, i32 0, i32 0, i32 2}
!2 = !{!"branch_weights", i32 43, i32 457}
