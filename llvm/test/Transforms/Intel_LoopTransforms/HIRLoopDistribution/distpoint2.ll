;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa"    < %s 2>&1 | FileCheck %s

; There is a bug in the test: the loop should be stripmined, but the checks does not expect that.
; REQUIRES: 0
; XFAIL: *

;
;  Note that b2 is a global scalar, not turning into register temp.
;  Distributed into 3 ways
;
;  for (i =0 ; i<n ; i++) {
;      if (DD[i] > 1) {
;	b2 = b1[i] - c1[i];
;#pragma distribute_point
;	a1[i] = b2* 5.0;
;      }
;      else if (DD[i] < 1) {
;	b2 = b1[i] * 2;
;      }
;      else {
;#pragma distribute_point
;	b2 = 123;
;      }
;      a4[i] = b2 + i;
;
;  HIR before distribution
;
;   DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;      %0 = (@DD)[0][i1];
;      if (%0 > 1.000000e+00)
;      {
;         %1 = (@b1)[0][i1];
;         %2 = (@c1)[0][i1];
;         %sub = %1  -  %2;
;         (@b2)[0] = %sub;
;         %4 = (@b2)[0]; <distribute_point>
;         %conv6 = %4  *  5.000000e+00;
;         (@a1)[0][i1] = %conv6;
;       }
;       else
;       {
;          if (%0 < 1.000000e+00)
;          {
;             %5 = (@b1)[0][i1];
;             %mul16 = %5  *  2.000000e+00;
;             (@b2)[0] = %mul16;
;          }
;          else
;          {
;             (@b2)[0] = 1.230000e+02; <distribute_point>
;          }
;       }
;       %7 = (@b2)[0];
;       %conv19 = sitofp.i32.float(i1);
;       %add = %7  +  %conv19;
;       (@a4)[0][i1] = %add;
;     END LOOP
;
;   HIR expected after distribution
; CHECK:           BEGIN REGION
; CHECK:              DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                %0 = (@DD)[0][i1]
; CHECK:                (%.TempArray)[0][i1] = %0
; CHECK:                if (%0 > 1.000000e+00)
; CHECK:                {
; CHECK:                  %1 = (@b1)[0][i1]
; CHECK:                  %2 = (@c1)[0][i1]
; CHECK:                  %sub = %1  -  %2
; CHECK:                 (@b2)[0] = %sub
; CHECK:                 (%.TempArray2)[0][i1] = (@b2)[0]
; CHECK:                }
; CHECK:              END LOOP
; CHECK:              DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                %0 = (%.TempArray)[0][i1]
; CHECK:               (@b2)[0] = (%.TempArray2)[0][i1]
; CHECK:                if (%0 > 1.000000e+00)
; CHECK:                {
; CHECK:                  %4 = (@b2)[0];
; CHECK:                  %conv6 = %4  *  5.000000e+00
; CHECK:                  (@a1)[0][i1] = %conv6
; CHECK:                }
; CHECK:                else
; CHECK:                {
; CHECK:                  if (%0 < 1.000000e+00)
; CHECK:                  {
; CHECK:                     %5 = (@b1)[0][i1]
; CHECK:                     %mul16 = %5  *  2.000000e+00
; CHECK:                     (@b2)[0] = %mul16
; CHECK:                     (%.TempArray2)[0][i1] = (@b2)[0]
; CHECK:                  }
; CHECK:                }
; CHECK:              END LOOP
; CHECK:              DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                 %0 = (%.TempArray)[0][i1]
; CHECK:                 (@b2)[0] = (%.TempArray2)[0][i1]
; CHECK:                if (%0 > 1.000000e+00)
; CHECK:                {
; CHECK:                }
; CHECK:                else
; CHECK:                {
; CHECK:                  if (%0 < 1.000000e+00)
; CHECK:                  {
; CHECK:                  }
; CHECK:                  else
; CHECK:                  {
; CHECK:                     (@b2)[0] = 1.230000e+02
; CHECK:                  }
; CHECK:               }
; CHECK:               %7 = (@b2)[0]
; CHECK:               %conv19 = sitofp.i32.float(i1)
; CHECK:               %add = %7  +  %conv19
; CHECK:               @a4)[0][i1] = %add
; CHECK:             END LOOP
; CHECK:           END REGION
;
;
;Module Before HIR; ModuleID = 'distpoint2.c'
source_filename = "distpoint2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@DD = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@b1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@c1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@b2 = common dso_local local_unnamed_addr global float 0.000000e+00, align 4
@a1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a4 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a3 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@c2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp31 = icmp sgt i32 %n, 0
  br i1 %cmp31, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %if.end18, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end18 ]
  %arrayidx = getelementptr inbounds [1000 x float], ptr @DD, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %0, 1.000000e+00
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [1000 x float], ptr @b1, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds [1000 x float], ptr @c1, i64 0, i64 %indvars.iv
  %2 = load float, ptr %arrayidx5, align 4, !tbaa !2
  %sub = fsub float %1, %2
  store float %sub, ptr @b2, align 4, !tbaa !7
  %3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %4 = load float, ptr @b2, align 4, !tbaa !7
  %conv6 = fmul float %4, 5.000000e+00
  %arrayidx8 = getelementptr inbounds [1000 x float], ptr @a1, i64 0, i64 %indvars.iv
  store float %conv6, ptr %arrayidx8, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %3) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  br label %if.end18

if.else:                                          ; preds = %for.body
  %cmp11 = fcmp olt float %0, 1.000000e+00
  br i1 %cmp11, label %if.then13, label %if.else17

if.then13:                                        ; preds = %if.else
  %arrayidx15 = getelementptr inbounds [1000 x float], ptr @b1, i64 0, i64 %indvars.iv
  %5 = load float, ptr %arrayidx15, align 4, !tbaa !2
  %mul16 = fmul float %5, 2.000000e+00
  store float %mul16, ptr @b2, align 4, !tbaa !7
  br label %if.end18

if.else17:                                        ; preds = %if.else
  %6 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  store float 1.230000e+02, ptr @b2, align 4, !tbaa !7
  call void @llvm.directive.region.exit(token %6) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  br label %if.end18

if.end18:                                         ; preds = %if.then13, %if.else17, %if.then
  %7 = load float, ptr @b2, align 4, !tbaa !7
  %8 = trunc i64 %indvars.iv to i32
  %conv19 = sitofp i32 %8 to float
  %add = fadd float %7, %conv19
  %arrayidx21 = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %indvars.iv
  store float %add, ptr %arrayidx21, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end18
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 65e8f9d46b54671e271ba934ab45010c98c98cce) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d5a4f1cbcbff7885d7b00fe044003cf37ba39d4d)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!4, !4, i64 0}
