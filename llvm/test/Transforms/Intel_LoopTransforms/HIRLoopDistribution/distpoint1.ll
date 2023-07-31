;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa"    < %s 2>&1 | FileCheck %s
;
;  for (i =0 ; i<n ; i++) {
;          a1[i] = b1[i] + c1[i];
; #pragma distribute_point
;          a2[i] = b2[i] * c2[i]; }
;
; CHECK: BEGIN REGION
; CHECK:  DO i1 = 0, sext.i32.i64(%n) + -1
; CHECK:     [[B1_LD:%.*]] = (@b1)[0][i1]
; CHECK:     [[C1_LD:%.*]] = (@c1)[0][i1]
; CHECK:     [[ADD:%.*]] = [[B1_LD:%.*]] + [[C1_LD:%.*]]
; CHECK:     (@a1)[0][i1] = [[ADD:%.*]]
; CHECK:  END LOOP
; CHECK:  DO i1 = 0, sext.i32.i64(%n) + -1
; CHECK:      [[B2_LD:%.*]] = (@b2)[0][i1]
; CHECK:      [[C2_LD:%.*]] = (@c2)[0][i1];
; CHECK:      [[MUL:%.*]] = [[B2_LD:%.*]] * [[C2_LD:%.*]]
; CHECK:      (@a2)[0][i1] = [[MUL:%.*]]
; CHECK:  END LOOP
;
;Module Before HIR; ModuleID = 'distpoint1.c'
source_filename = "distpoint1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@c1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@b2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@c2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@DD = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], ptr @b1, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @c1, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !2
  %add = fadd float %0, %1
  %arrayidx4 = getelementptr inbounds [1000 x float], ptr @a1, i64 0, i64 %indvars.iv
  store float %add, ptr %arrayidx4, align 4, !tbaa !2
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %arrayidx6 = getelementptr inbounds [1000 x float], ptr @b2, i64 0, i64 %indvars.iv
  %3 = load float, ptr %arrayidx6, align 4, !tbaa !2
  %arrayidx8 = getelementptr inbounds [1000 x float], ptr @c2, i64 0, i64 %indvars.iv
  %4 = load float, ptr %arrayidx8, align 4, !tbaa !2
  %mul = fmul float %3, %4
  %arrayidx10 = getelementptr inbounds [1000 x float], ptr @a2, i64 0, i64 %indvars.iv
  store float %mul, ptr %arrayidx10, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
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
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1bfc9df47817f0ad6a4bbfc288ea20461fe3701d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 7003241b9fa1b19e13fdc37bf1949b2a68a9fabc)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
