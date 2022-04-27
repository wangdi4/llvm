; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck %s

; RUN: opt < %s -enable-new-pm=0 -analyze -scalar-evolution | FileCheck %s --check-prefix=SCEV
; RUN: opt %s -passes="print<scalar-evolution>" -disable-output 2>&1 | FileCheck %s --check-prefix=SCEV

; Verify that %bf.ashr3579 has signed range info.
; SCEV: -->  %bf.ashr3579 U: [-268435456,268435456) S: [-268435456,268435456)


; Verify that we do not form SCC (%bf.ashr3579 -> %dec -> %bf.ashr35) because %bf.ashr3579 has signed range info.

; CHECK: SCC1: %bf.load3378 -> %bf.set39
; CHECK-NOT: SCC2

define void @foo(i32 %bf.set27, i32 %t) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %bf.ashr3579 = phi i32 [ %bf.ashr35, %for.body ], [ 268435455, %entry ]
  %bf.load3378 = phi i32 [ %bf.set39, %for.body ], [ %bf.set27, %entry ]
  %j.077 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %dec = add nsw i32 %bf.ashr3579, 536870911
  %bf.value37 = and i32 %dec, 536870911
  %bf.clear38 = and i32 %bf.load3378, -536870912
  %bf.set39 = or i32 %bf.value37, %bf.clear38
  %inc = add nuw nsw i32 %j.077, 1
  %cmp30 = icmp slt i32 %inc, %t
  %bf.shl34 = shl i32 %dec, 3
  %bf.ashr35 = ashr exact i32 %bf.shl34, 3
  br i1 %cmp30, label %for.body, label %exit

exit:
  %bf.set39.lcssa = phi i32 [ %bf.set39, %for.body ]
  %bf.ashr35.lcssa82 = phi i32 [ %bf.ashr35, %for.body ]
  ret void
}
