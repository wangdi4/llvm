; Check that extra attributes (e.g. nofpclass) are not incorrectly attached to mask argument when vectorizing math library calls with SVML
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vector-library=SVML -vplan-force-vf=16 -S < %s 2>&1 | FileCheck --check-prefixes=CHECK %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @f
; CHECK: call fast svml_cc nofpclass(nan inf) <16 x double> @__svml_pow16_mask(<16 x double> undef, <16 x i1> {{.*}}, <16 x double> noundef nofpclass(nan inf) {{.*}}, <16 x double> noundef nofpclass(nan inf) {{.*}})
; CHECK: declare nofpclass(nan inf) <16 x double> @__svml_pow16_mask(<16 x double>, <16 x i1>, <16 x double>, <16 x double>)

define dso_local void @f(ptr nocapture noundef writeonly %table, double noundef %gamma_val) #0 {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %loop.exit ]
  %i.04 = phi i32 [ 0, %entry ], [ %inc, %loop.exit ]
  %cmp.i = icmp ult i64 %indvars.iv, 192
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:
  %0 = trunc i64 %indvars.iv to i32
  %conv.i = uitofp i32 %0 to double
  %call.i = tail call fast nofpclass(nan inf) double @pow(double noundef nofpclass(nan inf) %conv.i, double noundef nofpclass(nan inf) %gamma_val) #2
  %conv2.i = fptoui double %call.i to i8
  br label %loop.exit

if.end.i:
  %conv3.i = trunc i32 %i.04 to i8
  br label %loop.exit

loop.exit:
  %retval.0.i = phi i8 [ %conv2.i, %if.then.i ], [ %conv3.i, %if.end.i ]
  %arrayidx = getelementptr inbounds i8, ptr %table, i64 %indvars.iv
  store i8 %retval.0.i, ptr %arrayidx, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.04, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:
  ret void
}

declare dso_local nofpclass(nan inf) double @pow(double noundef nofpclass(nan inf), double noundef nofpclass(nan inf)) local_unnamed_addr #1

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind }
attributes #2 = { nobuiltin nounwind }
