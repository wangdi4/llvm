; RUN: opt < %s -inline -enable-new-pm=0 -inline-report=0xf847 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt < %s -passes='cgscc(inline)' -inline-report=0xf847 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -inlinereportsetup -inline-report=0xf8c6 < %s -S | opt -inline -S -enable-new-pm=0 -inline-report=0xf8c6 -S | opt -inlinereportemitter -inline-report=0xf8c6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xf8c6 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xf8c6 -S | opt -passes='inlinereportemitter' -inline-report=0xef8c6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that bar is inlined into foo and then baz is inlined in bar when
; #pragma inline recursive is used

; CHECK-CL-LABEL: declare i32 @jazz

; CHECK-CL-LABEL: define i32 @baz
; CHECK-CL: call i32 @jazz

; CHECK-CL-LABEL: define i32 @bar
; CHECK-CL: call i32 @baz

; CHECK-CL-LABEL: define i32 @foo
; CHECK-CL: call i32 @jazz
; CHECK-CL: call i32 @jazz
; CHECK-CL-NOT: call i32 bar
; CHECK-CL-NOT: call i32 baz

; CHECK-CL-LABEL: COMPILE FUNC: baz
; CHECK: EXTERN: jazz

; CHECK-LABEL: COMPILE FUNC: bar
; CHECK: baz {{.*}}Inlining is not profitable

; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: INLINE: bar {{.*}}Callee is always inline (recursive)
; CHECK: INLINE: baz {{.*}}Callee is always inline (recursive)
; CHECK: EXTERN: jazz
; CHECK: INLINE: bar {{.*}}Callee is always inline (recursive)
; CHECK: INLINE: baz {{.*}}Callee is always inline (recursive)
; CHECK: EXTERN: jazz

; CHECK-MD-LABEL: declare {{.*}} i32 @jazz

; CHECK-MD-LABEL: define i32 @baz
; CHECK-MD: call i32 @jazz

; CHECK-MD-LABEL: define i32 @bar
; CHECK-MD: call i32 @baz

; CHECK-MD-LABEL: define i32 @foo
; CHECK-MD: call i32 @jazz
; CHECK-MD: call i32 @jazz
; CHECK-MD-NOT: call i32 bar
; CHECK-MD-NOT: call i32 baz

declare i32 @jazz(i32 %y1)

define i32 @baz(i32 %y1)  "function-inline-cost"="3000" {
entry:
  %cmp = icmp ugt i32 %y1, 0
  br i1 %cmp, label %call, label %ret
call:
  %y3 = call i32 @jazz(i32 %y1)
  br label %ret
ret:
  %y4 = phi i32 [ %y1, %entry ], [ %y3, %call ]
  ret i32 %y4
}

define i32 @bar(i32 %y1)  "function-inline-cost"="3000" alwaysinline_recursive {
entry:
  %cmp = icmp ugt i32 %y1, 0
  br i1 %cmp, label %call, label %ret
call:
  %y3 = call i32 @baz(i32 %y1)
  br label %ret
ret:
  %y4 = phi i32 [ %y1, %entry ], [ %y3, %call ]
  ret i32 %y4
}

define i32 @foo(i32 %y1) {
  %y3 = call i32 @bar(i32 %y1)
  %y4 = call i32 @bar(i32 %y1)
  %y5 = add i32 %y3, %y4
  ret i32 %y5
}
