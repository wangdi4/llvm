; RUN: opt -enable-new-pm=0 -loopopt=1 -O2 -debug-pass=Structure -disable-output <%s 2>&1 | FileCheck %s -check-prefix=LPM_WITHPROP
; RUN: opt -enable-new-pm=0 -loopopt=1 -O3 -debug-pass=Structure -disable-output <%s 2>&1 | FileCheck %s -check-prefix=LPM_WITHPROP

; RUN: opt -enable-new-pm=0 -disable-intel-proprietary-opts -loopopt=1 -O2 -debug-pass=Structure -disable-output <%s 2>&1 | FileCheck %s -check-prefix=LPM_NOPROP
; RUN: opt -enable-new-pm=0 -disable-intel-proprietary-opts -loopopt=1 -O3 -debug-pass=Structure -disable-output <%s 2>&1 | FileCheck %s -check-prefix=LPM_NOPROP

; RUN: opt -passes='default<O2>' -loopopt=1 -debug-pass-manager -disable-output <%s 2>&1 | FileCheck %s -check-prefix=NPM_WITHPROP
; RUN: opt -passes='default<O3>' -loopopt=1 -debug-pass-manager -disable-output <%s 2>&1 | FileCheck %s -check-prefix=NPM_WITHPROP

; RUN: opt -passes='default<O2>' -disable-intel-proprietary-opts -loopopt=1 -debug-pass-manager -disable-output <%s 2>&1 | FileCheck %s -check-prefix=NPM_NOPROP
; RUN: opt -passes='default<O3>' -disable-intel-proprietary-opts -loopopt=1 -debug-pass-manager -disable-output <%s 2>&1 | FileCheck %s -check-prefix=NPM_NOPROP

; LPM_WITHPROP:   HIR Framework [LoopOpt]
; LPM_NOPROP-NOT: HIR Framework [LoopOpt]

; NPM_WITHPROP:   Running analysis: HIRFrameworkAnalysis
; NPM_NOPROP-NOT: Running analysis: HIRFrameworkAnalysis

declare void @bar() local_unnamed_addr

define void @foo(i32 %n) local_unnamed_addr {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}
