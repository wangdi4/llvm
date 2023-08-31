; RUN: opt -passes="vplan-vec" -vplan-force-vf=4 -disable-output -print-after=vplan-vec < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test function with a kernel-call-once call to @foo. Attribute is on the function.
; Call to kernel-call-once function shouldn't be widened, but the loop should be vectorized
define dso_local void @kco(i64 %N, ptr %I) local_unnamed_addr {
; CHECK-LABEL: vector.body:
; CHECK-NEXT: [[UNIPHI:%.*]] = phi i64 [ 0, %VPlannedBB2 ], [ [[TMP6:%.*]], %vector.body ]
; CHECK-NEXT: [[VECPHI:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, %VPlannedBB2 ], [ [[TMP5:%.*]], %vector.body ]
; CHECK-NEXT: [[UNIPHI4:%.*]] = phi i64 [ 0, %VPlannedBB2 ], [ [[TMP4:%.*]], %vector.body ]
; CHECK-NEXT: [[VECPHI5:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, %VPlannedBB2 ], [ [[TMP3:%.*]], %vector.body ]
; CHECK-NEXT: call void @foo(i64 [[UNIPHI4]])
entry:
  %outer.idx = load i64, ptr %I
  br label %for.body.preheader
for.body.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body
for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %acc = phi i64 [0, %for.body.preheader], [%acc.next, %for.body]
  call void @foo(i64 %acc)
  %acc.next = add i64 %acc, 1
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.body, label %for.end
for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit
exit:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @foo(i64 %a) #0

attributes #0 = { "kernel-call-once" }
