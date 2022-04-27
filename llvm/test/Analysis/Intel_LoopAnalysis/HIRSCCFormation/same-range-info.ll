; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck %s

; RUN: opt < %s -enable-new-pm=0 -analyze -scalar-evolution | FileCheck %s --check-prefix=SCEV
; RUN: opt %s -passes="print<scalar-evolution>" -disable-output 2>&1 | FileCheck %s --check-prefix=SCEV

; Verify that we create the SCC (%shr87321 -> %shr87) even though %shr87321 has singled range info because its range info is the same as the other SCC node (%shr87).

; CHECK: SCC1: %shr87 -> %shr87321

; SCEV: -->  %shr87321 U: [-262144,262144) S: [-262144,262144)
; SCEV: -->  %shr87 U: [-262144,262144) S: [-262144,262144)

define void @foo(i32 %t56, i32 %t57, i32 %init, i32 %t49) {
entry:
  %shr = ashr i32 %init, 13
  br label %for.body80

for.body80:                                       ; preds = %for.body80, %entry
  %shr87321 = phi i32 [ %shr, %entry ], [ %shr87, %for.body80 ]
  %i1.2317 = phi i32 [ 2, %entry ], [ %inc89, %for.body80 ]
  %mul84 = mul nsw i32 %t57, %t56
  %add85 = add i32 %mul84, 4096
  %add86 = add i32 %add85, %shr87321
  %shr87 = ashr i32 %add86, 13
  %inc89 = add nuw nsw i32 %i1.2317, 1
  %exitcond = icmp eq i32 %inc89, %t49
  br i1 %exitcond, label %for.cond78.for.end90_crit_edge, label %for.body80

for.cond78.for.end90_crit_edge:                   ; preds = %for.body80
  %shr87.lcssa = phi i32 [ %shr87, %for.body80 ]
  ret void
}
