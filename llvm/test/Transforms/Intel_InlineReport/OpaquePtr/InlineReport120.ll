; RUN: opt -opaque-pointers < %s -passes='require<profile-summary>,cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<profile-summary>,cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that callee1 is inlined and recognized as a hot callsite.

; CHECK-CL-LABEL: define i32 @callee1(i32 %x)
; CHECK-CL-LABEL: define i32 @caller2(i32 %y1)
; CHECK-CL-NOT: call i32 @callee1(i32 %y1)

; CHECK-LABEL: COMPILE FUNC: callee1
; CHECK-LABEL: COMPILE FUNC: caller2
; CHECK: INLINE: callee1{{.*}}Callsite is hot

; CHECK-MD-LABEL: define i32 @callee1(i32 %x)
; CHECK-MD-LABEL: define i32 @caller2(i32 %y1)
; CHECK-MD-NOT: call i32 @callee1(i32 %y1)

define i32 @callee1(i32 %x) #0 {
bb:
  ret i32 %x
}

define i32 @caller2(i32 %y1) {
bb:
  %y3 = call i32 @callee1(i32 %y1), !prof !14
  ret i32 %y3
}

declare i32 @__gxx_personality_v0(...)

attributes #0 = { "function-inline-cost"="80" }

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
