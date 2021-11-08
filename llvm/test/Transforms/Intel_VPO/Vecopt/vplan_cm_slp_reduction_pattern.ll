; INTEL_FEATURE_SW_ADVANCED
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -enable-intel-advanced-opts -mcpu=core-avx2 -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; REQUIRES: intel_feature_sw_advanced
; The input HIR coming into the vectorizer looks like the following:
;   DO i1 = 0, %n + -1, 1   <DO_LOOP>
;     %0 = (%k)[-1 * i1 + %n];
;     %1 = (%s)[i1 + %n].0;
;     %conv1 = uitofp.i16.double(%1);
;     %mul = %0  *  %conv1;
;     %d1.057 = %mul  +  %d1.057; <Safe Reduction>
;     %2 = (%s)[i1 + %n].1;
;     %conv7 = uitofp.i16.double(%2);
;     %mul8 = %0  *  %conv7;
;     %d2.058 = %mul8  +  %d2.058; <Safe Reduction>
;     %3 = (%s)[i1 + %n].2;
;     %conv14 = uitofp.i16.double(%3);
;     %mul15 = %0  *  %conv14;
;     %d3.059 = %mul15  +  %d3.059; <Safe Reduction>
;     %4 = (%s)[i1 + %n].3;
;     %conv21 = uitofp.i16.double(%4);
;     %mul22 = %0  *  %conv21;
;     %d4.060 = %mul22  +  %d4.060; <Safe Reduction>
;   END LOOP
;
; This loop is better optimized by the SLP vectorizer. However, CM chooses VF=4
; to vectorize this loop which causes a big regression. This LIT test is added
; to show this issue. Subsequent changes implement an interim fix where we
; check for such a loop pattern to suppress vectorization and test for the
; loop not being vectorized.
;
; CHECK-NOT: DO i1 = {{.*}} <auto-vectorized>
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i16, i16, i16, i16 }
define dso_local void @foo(double* nocapture readonly %k, %struct.S* nocapture readonly %s, i64 %n) local_unnamed_addr #0 {
entry:
  %cmp54 = icmp sgt i64 %n, 0
  br i1 %cmp54, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %d4.060 = phi double [ %add23, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %d3.059 = phi double [ %add16, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %d2.058 = phi double [ %add9, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %d1.057 = phi double [ %add2, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %k.addr.056 = phi double* [ %incdec.ptr, %for.body ], [ %k, %for.body.preheader ]
  %l1.055 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %add.ptr = getelementptr inbounds double, double* %k.addr.056, i64 %n
  %0 = load double, double* %add.ptr, align 8
  %add = add nsw i64 %l1.055, %n
  %red = getelementptr inbounds %struct.S, %struct.S* %s, i64 %add, i32 0
  %1 = load i16, i16* %red, align 2
  %conv1 = uitofp i16 %1 to double
  %mul = fmul fast double %0, %conv1
  %add2 = fadd fast double %mul, %d1.057
  %blue = getelementptr inbounds %struct.S, %struct.S* %s, i64 %add, i32 1
  %2 = load i16, i16* %blue, align 2
  %conv7 = uitofp i16 %2 to double
  %mul8 = fmul fast double %0, %conv7
  %add9 = fadd fast double %mul8, %d2.058
  %green = getelementptr inbounds %struct.S, %struct.S* %s, i64 %add, i32 2
  %3 = load i16, i16* %green, align 2
  %conv14 = uitofp i16 %3 to double
  %mul15 = fmul fast double %0, %conv14
  %add16 = fadd fast double %mul15, %d3.059
  %opacity = getelementptr inbounds %struct.S, %struct.S* %s, i64 %add, i32 3
  %4 = load i16, i16* %opacity, align 2
  %conv21 = uitofp i16 %4 to double
  %mul22 = fmul fast double %0, %conv21
  %add23 = fadd fast double %mul22, %d4.060
  %inc = add nuw nsw i64 %l1.055, 1
  %incdec.ptr = getelementptr inbounds double, double* %k.addr.056, i64 -1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %add2.lcssa = phi double [ %add2, %for.body ]
  %add9.lcssa = phi double [ %add9, %for.body ]
  %add16.lcssa = phi double [ %add16, %for.body ]
  %add23.lcssa = phi double [ %add23, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %d1.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add2.lcssa, %for.end.loopexit ]
  %d2.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add9.lcssa, %for.end.loopexit ]
  %d3.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add16.lcssa, %for.end.loopexit ]
  %d4.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add23.lcssa, %for.end.loopexit ]
  tail call void @baz(double %d1.0.lcssa, double %d2.0.lcssa, double %d3.0.lcssa, double %d4.0.lcssa) #2
  ret void
}

declare dso_local void @baz(double, double, double, double) local_unnamed_addr #1
; end INTEL_FEATURE_SW_ADVANCED
