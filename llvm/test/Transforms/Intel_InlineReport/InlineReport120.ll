; RUN: opt < %s -passes='require<profile-summary>,cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,require<profile-summary>,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

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

define i32 @callee1(i32 %x) "function-inline-cost"="80" {
  ret i32 %x
}

define i32 @caller2(i32 %y1) {
  %y3 = call i32 @callee1(i32 %y1), !prof !21
  ret i32 %y3
}

declare i32 @__gxx_personality_v0(...)

!llvm.module.flags = !{!1}
!21 = !{!"branch_weights", i64 300}

!1 = !{i32 1, !"ProfileSummary", !2}
!2 = !{!3, !4, !5, !6, !7, !8, !9, !10}
!3 = !{!"ProfileFormat", !"SampleProfile"}
!4 = !{!"TotalCount", i64 10000}
!5 = !{!"MaxCount", i64 1000}
!6 = !{!"MaxInternalCount", i64 1}
!7 = !{!"MaxFunctionCount", i64 1000}
!8 = !{!"NumCounts", i64 3}
!9 = !{!"NumFunctions", i64 3}
!10 = !{!"DetailedSummary", !11}
!11 = !{!12, !13, !14}
!12 = !{i32 10000, i64 100, i32 1}
!13 = !{i32 999000, i64 100, i32 1}
!14 = !{i32 999999, i64 1, i32 2}
