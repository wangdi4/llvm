; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S < %s | FileCheck %s

; Verify that we patch the the single operand phi %.lcssa322 which uses extraneous liveout value %liveout not defined inside the region.

; CHECK: %.lcssa322 = phi i32 [ %liveout, %for.body78.split ], [ %liveout, %afterloop


@g_hjwx = local_unnamed_addr global i32 49, align 4
@g_jyqdijl = local_unnamed_addr global i32 78, align 4

define void @foo(i32 %liveout, i32 %.ph) {
entry:
  %v_l = alloca i32, align 4
  br label %for.body78

for.body78:                                       ; preds = %entry, %for.body78
  %0 = phi i32 [ 0, %entry ], [ %inc83, %for.body78 ]
  %1 = phi i32 [ %.ph, %entry ], [ %add80, %for.body78 ]
  %add80 = add i32 %1, 64
  store i32 %add80, i32* @g_hjwx, align 4
  store i32 %liveout, i32* %v_l, align 4
  %2 = load i32, i32* @g_jyqdijl, align 4
  %inc81 = add i32 %2, 1
  store i32 %inc81, i32* @g_jyqdijl, align 4
  %inc83 = add nuw nsw i32 %0, 1
  %cmp77 = icmp ult i32 %inc83, 63
  br i1 %cmp77, label %for.body78, label %for.end84

for.end84:                                        ; preds = %for.body78
  %.lcssa322 = phi i32 [ %liveout, %for.body78 ]
  %.lcssa321 = phi i32 [ %add80, %for.body78 ]
  %.lcssa = phi i32 [ %inc83, %for.body78 ]
  ret void
}
