; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Verify that temp cleanup does not fail when processing multiple uses of the
; temp in the same inst which can be reordered w.r.t to the rval def inst.

; In the test case below, we are trying to eliminate the temp copy
; %nt.promoted.out2 by forward substituting its rval into uses.

; It has two uses in 't18 ='. The temp can be eliminated if we move the use
; inst before '%nt.promoted =' and perform forward substitution.
; Temp cleanup reordered the insts when processing the first use but
; compfailed when trying to reorder them again for the second use.

; Dump Before-

; CHECK: + DO i1 = 0, 17, 1   <DO_LOOP>
; CHECK: |   %nt.promoted.out2 = %nt.promoted;
; CHECK: |   %t18.out1 = %t18;
; CHECK: |   if (%tobool.not != 0)
; CHECK: |   {
; CHECK: |      %t19 = (%ee)[0];
; CHECK: |      (%ee)[0] = (%t19 * %t18.out1);
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %nt.promoted = %indvars.iv  +  %nt.promoted;
; CHECK: |      %t18 = %t18.out1 + %t17 + %nt.promoted.out2  +  ((2 + %nt.promoted.out2) * %indvars.iv27);
; CHECK: |   }
; CHECK: |   %nt.promoted.out = %nt.promoted;
; CHECK: |   %t18.out = %t18;
; CHECK: + END LOOP


; Dump After-

; CHECK: + DO i1 = 0, 17, 1   <DO_LOOP>
; CHECK: |   if (%tobool.not != 0)
; CHECK: |   {
; CHECK: |      %t19 = (%ee)[0];
; CHECK: |      (%ee)[0] = (%t19 * %t18);
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %t18 = %nt.promoted + %t18 + %t17  +  ((2 + %nt.promoted) * %indvars.iv27);
; CHECK: |      %nt.promoted = %indvars.iv  +  %nt.promoted;
; CHECK: |   }
; CHECK: + END LOOP


define void @foo(i32 %t12, i32 %nt.promoted29, ptr %ee, i32 %indvars.iv, i32 %indvars.iv27, i32 %t17, i1 %tobool.not) {
entry:
  br label %for.body3

for.body3:                                        ; preds = %for.inc7, %entry
  %t18 = phi i32 [ %t12, %entry ], [ %t26, %for.inc7 ]
  %nt.promoted = phi i32 [ %nt.promoted29, %entry ], [ %nt.promoted31, %for.inc7 ]
  %storemerge1420 = phi i32 [ 19, %entry ], [ %dec8, %for.inc7 ]
  br i1 %tobool.not, label %if.else, label %for.inc7.loopexit

if.else:                                          ; preds = %for.body3
  %t19 = load i32, ptr %ee, align 4
  %mul = mul i32 %t19, %t18
  store i32 %mul, ptr %ee, align 4
  br label %for.inc7

for.inc7.loopexit:                                ; preds = %for.body3
  %t20 = add i32 %t17, %nt.promoted
  %t21 = add i32 %nt.promoted, 2
  %t22 = mul i32 %indvars.iv27, %t21
  %t23 = add i32 %indvars.iv, %nt.promoted
  %t24 = add i32 %t20, %t18
  %t25 = add i32 %t24, %t22
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc7.loopexit, %if.else
  %t26 = phi i32 [ %t25, %for.inc7.loopexit ], [ %t18, %if.else ]
  %nt.promoted31 = phi i32 [ %t23, %for.inc7.loopexit ], [ %nt.promoted, %if.else ]
  %dec8 = add nsw i32 %storemerge1420, -1
  %cmp2 = icmp ugt i32 %dec8, 1
  br i1 %cmp2, label %for.body3, label %for.inc10

for.inc10:                                        ; preds = %for.inc7
  %.lcssa = phi i32 [ %t26, %for.inc7 ]
  %nt.promoted31.lcssa = phi i32 [ %nt.promoted31, %for.inc7 ]
  ret void
}
