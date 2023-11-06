; RUN: opt < %s -passes=indvars -S | FileCheck %s

; Verify that loop liveout phi %incdec.ptr35.us.lcssa is eliminated by
; computing the exit value of %incdec.ptr35.us.

; The SCEV form of %incdec.ptr35.us is like this-
; (16 + (16 * (zext i32 (-2 + %dual.012) to i64))<nuw><nsw> + %twp.addr.016)

; The cost model started giving up on exit value computation when the TTI cost
; of SCEVMulExpr increased from 1 to 2. Since the multiplication in this case is
; with 16 which is a power of 2, the cost model was changed to treat it as the 
; cheaper Shl operation.

; CHECK-NOT: %incdec.ptr35.us.lcssa

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double* @foo(double* %twp.addr.016, double* %data, i32 %div, i32 %dual.012, i32 %mul79) {
entry:
  %cmp337 = icmp ugt i32 %dual.012, 1
  br i1 %cmp337, label %for.body34.us, label %exit

for.body34.us:                                    ; preds = %entry, %for.body34.us
  %twp.addr.110.us = phi double* [ %incdec.ptr35.us, %for.body34.us ], [ %twp.addr.016, %entry ]
  %a.08.us = phi i32 [ %inc.us, %for.body34.us ], [ 1, %entry ]
  %incdec.ptr.us = getelementptr inbounds double, double* %twp.addr.110.us, i64 1
  %t7 = load double, double* %twp.addr.110.us, align 8
  %incdec.ptr35.us = getelementptr inbounds double, double* %twp.addr.110.us, i64 2
  %t8 = load double, double* %incdec.ptr.us, align 8
  %mul52.us = fmul fast double %t8, %t7
  %idxprom59.us = sext i32 %a.08.us to i64
  %arrayidx60.us = getelementptr inbounds double, double* %data, i64 %idxprom59.us
  store double %mul52.us, double* %arrayidx60.us, align 8
  %inc.us = add nuw nsw i32 %a.08.us, 1
  %cmp33.us = icmp ult i32 %inc.us, %dual.012
  br i1 %cmp33.us, label %for.body34.us, label %for.end83.loopexit

for.end83.loopexit:                               ; preds = %for.body34.us
  %incdec.ptr35.us.lcssa = phi double* [ %incdec.ptr35.us, %for.body34.us ]
  ret double* %incdec.ptr35.us.lcssa

exit:
  ret double* null
}
