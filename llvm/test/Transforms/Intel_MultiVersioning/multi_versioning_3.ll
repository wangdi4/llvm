; Test multi versioning setting of profile weights on the conditional
; created that controls the version selection.
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

%struct.S = type { i8 }

; In this case, the profile information should be maintained on the branch
; that controls the version selection.
define i32 @foo1(%struct.S* %arg) !prof !0 {
entry:
  %addr = getelementptr inbounds %struct.S, %struct.S* %arg, i64 0, i32 0
  %value = load i8, i8* %addr, align 1
  %cmp = icmp ne i8 %value, 0
  br i1 %cmp, label %if.then, label %if.else, !prof !1

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp, i32 33, i32 22
  ret i32 %result
}
; CHECK-LABEL: define i32 @foo1(%struct.S* %arg)
; The initial test should get profile weights based on the original counts
; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !1

; The multi-versioned statements should maintain the profile counts.
; CHECK: br i1 true, label %if.then, label %if.else, !prof !2
; CHECK: br i1 false, label %if.then.clone, label %if.else.clone, !prof !3


; In this case, the profile weights need to be reversed on the updated
; instructions because the multi-versioning pass always creates the version
; selection test as a non-equality test.
define i32 @foo2(%struct.S* %arg) !prof !2 {
entry:
  %addr = getelementptr inbounds %struct.S, %struct.S* %arg, i64 0, i32 0
  %value = load i8, i8* %addr, align 1
  %cmp = icmp eq i8 %value, 0
  br i1 %cmp, label %if.then, label %if.else, !prof !3

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp, i32 33, i32 22
  ret i32 %result
}
; CHECK-LABEL: define i32 @foo2(%struct.S* %arg)
; CHECK: br i1 %2, label %entry, label %entry.clone, !prof !5
; CHECK: br i1 false, label %if.then, label %if.else, !prof !6
; CHECK: br i1 true, label %if.then.clone, label %if.else.clone, !prof !7

; No profile information should be set when there wasn't profile information
; available.
define i32 @foo3(%struct.S* %arg) {
entry:
  %addr = getelementptr inbounds %struct.S, %struct.S* %arg, i64 0, i32 0
  %value = load i8, i8* %addr, align 1
  %cmp1 = icmp ne i8 %value, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:

  %result = select i1 %cmp1, i32 33, i32 22
  ret i32 %result
}
; Verify that there is no !prof metadata attached to the branches.
; CHECK-LABEL: define i32 @foo3(%struct.S* %arg)
; CHECK: br i1 %2, label %entry, label %entry.clone{{$}}
; CHECK: br i1 true, label %if.then, label %if.else{{$}}
; CHECK: br i1 false, label %if.then.clone, label %if.else.clone{{$}}


!0 = !{!"function_entry_count", i64 12600}
!1 = !{!"branch_weights", i32 12000, i32 600 }
!2 = !{!"function_entry_count", i64 5700}
!3 = !{!"branch_weights", i32 700, i32 5000 }


; CHECK: !1 = !{!"branch_weights", i32 12000, i32 600}
; CHECK: !2 = !{!"branch_weights", i32 12000, i32 0}
; CHECK: !3 = !{!"branch_weights", i32 0, i32 600}
; CHECK: !5 = !{!"branch_weights", i32 5000, i32 700}
; CHECK: !6 = !{!"branch_weights", i32 0, i32 5000}
; CHECK: !7 = !{!"branch_weights", i32 700, i32 0}
