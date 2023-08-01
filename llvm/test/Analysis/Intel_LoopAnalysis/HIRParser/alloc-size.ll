; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that we use (padded) size of 16 bytes for memrefs based on x86_fp80* type.
; We shouldn't see any denominator in parsing of (%start)[i1].

; CHECK: + DO i1 = 0, (-1 * ptrtoint.ptr.i64(%start) + ptrtoint.ptr.i64(%end) + -16)/u16, 1 <DO_LOOP>
; CHECK: |   %conv.i = fpext.double.x86_fp80(%f.019.i);
; CHECK: |   %t2 = (%start)[i1];
; CHECK: |   %mul.i = %t2  *  %conv.i;
; CHECK: |   (%start)[i1] = %mul.i;
; CHECK: |   %mul6.i = %conv.i  *  %factor;
; CHECK: |   %f.019.i = fptrunc.x86_fp80.double(%mul6.i);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_ZN11Polynomials10PolynomialIeE5scaleEe(x86_fp80 %factor, ptr %start, ptr %end) {
entry:
  %cmp.i17.i = icmp eq ptr %start, %end
  br i1 %cmp.i17.i, label %.exit, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %entry
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %f.019.i = phi double [ %conv7.i, %for.body.i ], [ 1.000000e+00, %for.body.i.preheader ]
  %c.sroa.0.018.i = phi ptr [ %incdec.ptr.i.i, %for.body.i ], [ %start, %for.body.i.preheader ]
  %conv.i = fpext double %f.019.i to x86_fp80
  %t2 = load x86_fp80, ptr %c.sroa.0.018.i, align 16
  %mul.i = fmul x86_fp80 %t2, %conv.i
  store x86_fp80 %mul.i, ptr %c.sroa.0.018.i, align 16
  %mul6.i = fmul x86_fp80 %conv.i, %factor
  %conv7.i = fptrunc x86_fp80 %mul6.i to double
  %incdec.ptr.i.i = getelementptr inbounds x86_fp80, ptr %c.sroa.0.018.i, i64 1
  %cmp.i.i = icmp eq ptr %incdec.ptr.i.i, %end
  br i1 %cmp.i.i, label %.exit.loopexit, label %for.body.i

.exit.loopexit: ; preds = %for.body.i
  br label %.exit

.exit: ; preds = %.exit.loopexit, %entry
  ret void
}

