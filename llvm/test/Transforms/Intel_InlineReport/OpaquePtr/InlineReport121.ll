; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='require<profile-summary>,cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<profile-summary>,cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that @bar is not inlined on the compile step of an -lfto compilation
; because @baz might get converted from a single callsite to multiple callsite
; function.

; CHECK-CL-LABEL: declare i32 @baz
; CHECK-CL-LABEL: define i32 @bar
; CHECK-CL: call i32 @baz
; CHECK-CL-LABEL: define i32 @foo

; CHECK-LABEL: COMPILE FUNC: bar
; CHECK: EXTERN: baz
; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: bar {{.*}}Inlining is not profitable
; CHECK: bar {{.*}}Inlining is not profitable

; CHECK-MD-LABEL: declare {{.*}} i32 @baz
; CHECK-MD-LABEL: define i32 @bar
; CHECK-MD: call i32 @baz
; CHECK-MD-LABEL: define i32 @foo

declare i32 @baz(i32)

define i32 @bar(i32 %y1) #0 {
entry:
  %cmp = icmp ugt i32 %y1, 0
  br i1 %cmp, label %call, label %ret

call:                                             ; preds = %entry
  %y3 = call i32 @baz(i32 %y1)
  br label %ret

ret:                                              ; preds = %call, %entry
  %y4 = phi i32 [ %y1, %entry ], [ %y3, %call ]
  ret i32 %y4
}

define i32 @foo(i32 %y1) {
bb:
  %y3 = call i32 @bar(i32 %y1), !prof !14
  %y4 = call i32 @bar(i32 %y1), !prof !14
  %y5 = add i32 %y3, %y4
  ret i32 %y5
}

declare i32 @__gxx_personality_v0(...)

attributes #0 = { "function-inline-cost"="300" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"ProfileSummary", !1}
!1 = !{!2, !3, !4, !5, !6, !7, !8, !9}
!2 = !{!"ProfileFormat", !"SampleProfile"}
!3 = !{!"TotalCount", i64 10000}
!4 = !{!"MaxCount", i64 1000}
!5 = !{!"MaxInternalCount", i64 1}
!6 = !{!"MaxFunctionCount", i64 1000}
!7 = !{!"NumCounts", i64 3}
!8 = !{!"NumFunctions", i64 3}
!9 = !{!"DetailedSummary", !10}
!10 = !{!11, !12, !13}
!11 = !{i32 10000, i64 100, i32 1}
!12 = !{i32 999000, i64 100, i32 1}
!13 = !{i32 999999, i64 1, i32 2}
!14 = !{!"branch_weights", i64 300}
; end INTEL_FEATURE_SW_ADVANCED
