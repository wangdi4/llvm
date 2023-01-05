; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

; This test case checks that the profiling information was preserved after
; multiversioning when there are freeze instructions.

; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !1
; CHECK: %frz = freeze i1 true
; CHECK: br i1 %frz, label %if.then, label %if.else, !prof !2
; CHECK: %frz.clone = freeze i1 false
; CHECK: br i1 %frz.clone, label %if.then.clone, label %if.else.clone, !prof !3

; CHECK: !1 = !{!"branch_weights", i32 12000, i32 600}
; CHECK: !2 = !{!"branch_weights", i32 12000, i32 0}
; CHECK: !3 = !{!"branch_weights", i32 0, i32 600}

%struct.S = type { i8 }

define i32 @foo1(%struct.S* %arg) !prof !0 {
entry:
  %addr = getelementptr inbounds %struct.S, %struct.S* %arg, i64 0, i32 0
  %value = load i8, i8* %addr, align 1
  %cmp = icmp ne i8 %value, 0
  %frz = freeze i1 %cmp
  br i1 %frz, label %if.then, label %if.else, !prof !1

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %frz, i32 33, i32 22
  ret i32 %result
}

!0 = !{!"function_entry_count", i64 12600}
!1 = !{!"branch_weights", i32 12000, i32 600 }
!2 = !{!"function_entry_count", i64 5700}
!3 = !{!"branch_weights", i32 700, i32 5000 }
