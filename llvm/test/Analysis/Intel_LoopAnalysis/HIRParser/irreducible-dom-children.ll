; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we are able to successfully parse the loop. This was failing during lexical link formation because the loop header for.body9.i dominated bblocks for.cond35.L2_crit_edge.i and L2.preheader.i which are part of an irreducible cycle outside the region. HIR framework cannot handle irreducible cfg. The fix is to not process bblocks outside the region.

; CHECK: + DO i1 = 0, 1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   if (undef #UNDEF# undef)
; CHECK: |   {
; CHECK: |      goto if.then14.i;
; CHECK: |   }
; CHECK: + END LOOP


; Function Attrs: nounwind uwtable
define void @main() local_unnamed_addr #0 {
L3.i.thread:
  br label %if.end.i

L3.i:                                             ; preds = %if.then14.i, %if.then14.i
  switch i8 undef, label %L2.preheader.i.loopexit [
    i8 80, label %if.end.i.backedge
    i8 97, label %if.end.i.backedge
  ]

if.end.i.backedge:                                ; preds = %L3.i, %L3.i
  unreachable

if.end.i:                                         ; preds = %L3.i.thread
  br label %for.body9.i

for.body9.i:                                      ; preds = %for.inc.i, %if.end.i
  %indvars.iv.i = phi i64 [ 63, %if.end.i ], [ undef, %for.inc.i ]
  br i1 undef, label %for.inc.i, label %if.then14.i

if.then14.i:                                      ; preds = %for.body9.i
  switch i8 undef, label %foo.exit.loopexit [
    i8 80, label %L3.i
    i8 112, label %L3.i
  ]

for.inc.i:                                        ; preds = %for.body9.i
  %cmp7.i = icmp ugt i64 %indvars.iv.i, 1
  br i1 %cmp7.i, label %for.body9.i, label %for.end.i

for.end.i:                                        ; preds = %for.inc.i
  br label %for.cond35.L2_crit_edge.i

for.cond35.i:                                     ; preds = %L2.preheader.i
  br label %for.cond35.L2_crit_edge.i

for.cond35.L2_crit_edge.i:                        ; preds = %for.cond35.i, %for.end.i
  br label %L2.preheader.i

L2.preheader.i.loopexit:                          ; preds = %L3.i
  br label %L2.preheader.i

L2.preheader.i:                                   ; preds = %L2.preheader.i.loopexit, %for.cond35.L2_crit_edge.i
  br label %for.cond35.i

foo.exit.loopexit:                                ; preds = %if.then14.i
  unreachable
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
