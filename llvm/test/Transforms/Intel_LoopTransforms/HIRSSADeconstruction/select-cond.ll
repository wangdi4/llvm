; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Test checks that CMP instruction did not get propogated into SELECT condition
; since one of the CMP arguments got redefined between CMP and SELECT.

; CHECK:  + DO i1 = 0, (zext.i2.i32(trunc.i32.i2((ptrtoint.ptr.i32(%src) /u 4))) + %n + -9)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 536870910>  <LEGAL_MAX_TC = 536870910>
; CHECK: |   %max4.0114.out = %max4.0114;
;        |   %indexCurr4.0116.out = %indexCurr4.0116;
; CHECK: |   %16 = %curr4.0115 >u %max4.0114.out;
;        |   %17 = bitcast.<4 x i1>.i4(%16);
; CHECK: |   if (%17 != 0)
;        |   {
; CHECK: |      %max4.0114 = %max4.0114  +  %curr4.0115;
; CHECK: |      %19 = (%curr4.0115 >u %max4.0114.out) ? %indexCurr4.0116.out : zeroinitializer;
;        |      %indexMax4.0118 = %indexMax4.0118  -  %19;
;        |   }
;        |   %max4.0114.out2 = %max4.0114;
;        |   %indexMax4.0118.out = %indexMax4.0118;
;        |   %indexCurr4.0116 = %indexCurr4.0116  +  <float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00>;
;        |   %add.ptr35.val = (<4 x float>*)(%src)[4 * i1 + -1 * zext.i2.i32(trunc.i32.i2((ptrtoint.ptr.i32(%src) /u 4))) + 8];
;        |   %curr4.0115 = %add.ptr35.val;
;        + END LOOP

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@_ZL8const_4s = internal unnamed_addr constant <4 x float> <float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00>, align 16
@llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer

; Function Attrs: mustprogress nofree nosync nounwind readonly willreturn uwtable
define dso_local noundef float @f(ptr noundef %src, i32 noundef %n, ptr nocapture noundef readnone %position) local_unnamed_addr {
entry:
  %0 = ptrtoint ptr %src to i32
  %shr = lshr i32 %0, 2
  %and = and i32 %shr, 3
  %sub = sub nuw nsw i32 4, %and
  %sub1 = sub nsw i32 %n, %sub
  %arrayidx8 = getelementptr inbounds float, ptr %src, i32 %sub
  %src.val = load <4 x float>, ptr %src, align 1
  %cmp113 = icmp sgt i32 %sub1, 4
  br i1 %cmp113, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %conv = sitofp i32 %sub to float
  %vecinit.i109 = insertelement <4 x float> undef, float %conv, i64 0
  %add13 = fadd fast float %conv, 1.000000e+00
  %vecinit1.i110 = insertelement <4 x float> %vecinit.i109, float %add13, i64 1
  %add11 = fadd fast float %conv, 2.000000e+00
  %vecinit2.i111 = insertelement <4 x float> %vecinit1.i110, float %add11, i64 2
  %add9 = fadd fast float %conv, 3.000000e+00
  %vecinit3.i112 = insertelement <4 x float> %vecinit2.i111, float %add9, i64 3
  %1 = load float, ptr %arrayidx8, align 4
  %vecinit.i = insertelement <4 x float> undef, float %1, i64 0
  %add6 = sub nuw nsw i32 5, %and
  %arrayidx7 = getelementptr inbounds float, ptr %src, i32 %add6
  %2 = load float, ptr %arrayidx7, align 4
  %vecinit1.i = insertelement <4 x float> %vecinit.i, float %2, i64 1
  %add4 = sub nuw nsw i32 6, %and
  %arrayidx5 = getelementptr inbounds float, ptr %src, i32 %add4
  %3 = load float, ptr %arrayidx5, align 4
  %vecinit2.i = insertelement <4 x float> %vecinit1.i, float %3, i64 2
  %add = xor i32 %and, 7
  %arrayidx3 = getelementptr inbounds float, ptr %src, i32 %add
  %4 = load float, ptr %arrayidx3, align 4
  %vecinit3.i = insertelement <4 x float> %vecinit2.i, float %4, i64 3
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  %max4.1.lcssa = phi <4 x float> [ %max4.1, %if.end ]
  %indexMax4.1.lcssa = phi <4 x float> [ %indexMax4.1, %if.end ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %max4.0.lcssa = phi <4 x float> [ %src.val, %entry ], [ %max4.1.lcssa, %for.cond.cleanup.loopexit ]
  %indexMax4.0.lcssa = phi <4 x float> [ <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>, %entry ], [ %indexMax4.1.lcssa, %for.cond.cleanup.loopexit ]
  %conv16 = sitofp i32 %n to float
  %sub23 = fadd fast float %conv16, -4.000000e+00
  %vecinit.i105 = insertelement <4 x float> undef, float %sub23, i64 0
  %sub21 = fadd fast float %conv16, -3.000000e+00
  %vecinit1.i106 = insertelement <4 x float> %vecinit.i105, float %sub21, i64 1
  %sub19 = fadd fast float %conv16, -2.000000e+00
  %vecinit2.i107 = insertelement <4 x float> %vecinit1.i106, float %sub19, i64 2
  %sub17 = fadd fast float %conv16, -1.000000e+00
  %vecinit3.i108 = insertelement <4 x float> %vecinit2.i107, float %sub17, i64 3
  %add.ptr = getelementptr inbounds float, ptr %src, i32 %n
  %add.ptr39 = getelementptr inbounds float, ptr %add.ptr, i32 -4
  %add.ptr39.val = load <4 x float>, ptr %add.ptr39, align 1
  %5 = fcmp ugt <4 x float> %add.ptr39.val, %max4.0.lcssa
  %6 = bitcast <4 x float> %vecinit3.i108 to <4 x i32>
  %and.i104 = select <4 x i1> %5, <4 x i32> %6, <4 x i32> zeroinitializer
  %7 = bitcast <4 x i32> %and.i104 to <4 x float>
  %8 = tail call fast <4 x float> @llvm.x86.sse.max.ps(<4 x float> %indexMax4.0.lcssa, <4 x float> %7)
  %9 = icmp slt <4 x i32> %and.i104, zeroinitializer
  %10 = bitcast <4 x i1> %9 to i4
  %11 = zext i4 %10 to i32
  %12 = bitcast <4 x float> %8 to <4 x i32>
  %13 = icmp slt <4 x i32> %12, zeroinitializer
  %14 = bitcast <4 x i1> %13 to i4
  %15 = zext i4 %14 to i32
  %add47 = add nuw nsw i32 %11, %15
  %conv48 = sitofp i32 %add47 to float
  ret float %conv48

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %nextFour.0119 = phi i32 [ 4, %for.body.lr.ph ], [ %add37, %if.end ]
  %indexMax4.0118 = phi <4 x float> [ <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00>, %for.body.lr.ph ], [ %indexMax4.1, %if.end ]
  %indexCurr4.0116 = phi <4 x float> [ %vecinit3.i112, %for.body.lr.ph ], [ %add.i, %if.end ]
  %curr4.0115 = phi <4 x float> [ %vecinit3.i, %for.body.lr.ph ], [ %add.ptr35.val, %if.end ]
  %max4.0114 = phi <4 x float> [ %src.val, %for.body.lr.ph ], [ %max4.1, %if.end ]
  %16 = fcmp ugt <4 x float> %curr4.0115, %max4.0114
  %17 = bitcast <4 x i1> %16 to i4
  %cmp29.not = icmp eq i4 %17, 0
  br i1 %cmp29.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %18 = fadd <4 x float> %max4.0114, %curr4.0115
  %19 = select <4 x i1> %16, <4 x float> %indexCurr4.0116, <4 x float> zeroinitializer
  %20 = fsub <4 x float> %indexMax4.0118, %19
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %max4.1 = phi <4 x float> [ %18, %if.then ], [ %max4.0114, %for.body ]
  %indexMax4.1 = phi <4 x float> [ %20, %if.then ], [ %indexMax4.0118, %for.body ]
  %add.i = fadd <4 x float> %indexCurr4.0116, <float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00>
  %add.ptr35 = getelementptr inbounds float, ptr %arrayidx8, i32 %nextFour.0119
  %add.ptr35.val = load <4 x float>, ptr %add.ptr35, align 16
  %add37 = add nuw nsw i32 %nextFour.0119, 4
  %cmp = icmp slt i32 %add37, %sub1
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

; Function Attrs: nofree nosync nounwind readnone
declare <4 x float> @llvm.x86.sse.max.ps(<4 x float>, <4 x float>)
