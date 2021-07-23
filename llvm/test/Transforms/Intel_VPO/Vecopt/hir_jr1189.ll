; Test from CMPLRLLVM-1189. Check that we can generate vector code.
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-predicate -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -print-after=hir-vplan-vec -S  %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -S %s 2>&1 | FileCheck %s

; HIR opt-predicate generates two vectorizable loops and both should be vectorized.
; CHECK: DO i1 = 0, {{.*}}, 4
; CHECK: DO i1 = 0, {{.*}}, 4
;Module Before HIR
; ModuleID = 'jr1189.c'
source_filename = "jr1189.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g = dso_local local_unnamed_addr global i32 0, align 4
@a = common dso_local local_unnamed_addr global [64 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %i.014 = phi i32 [ 17, %entry ], [ %inc, %for.end ]
  %0 = load i32, i32* @g, align 4, !tbaa !2
  %tobool = icmp eq i32 %0, 0
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %lor.end
  %indvars.iv = phi i64 [ 41, %for.cond1.preheader ], [ %indvars.iv.next, %lor.end ]
  br i1 %tobool, label %lor.rhs, label %lor.end

lor.rhs:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds [64 x i64], [64 x i64]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %1 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %tobool4 = icmp ne i64 %1, 0
  br label %lor.end

lor.end:                                          ; preds = %for.body3, %lor.rhs
  %2 = phi i1 [ true, %for.body3 ], [ %tobool4, %lor.rhs ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %3 = trunc i64 %indvars.iv.next to i32
  %cmp2 = icmp ult i32 %3, 42
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %lor.end
  %.lcssa = phi i1 [ %2, %lor.end ]
  %lor.ext.le = zext i1 %.lcssa to i32
  %call = tail call i32 (i32, ...) bitcast (i32 (...)* @print_int to i32 (i32, ...)*)(i32 %lor.ext.le) #2
  %inc = add nuw nsw i32 %i.014, 1
  %exitcond = icmp eq i32 %inc, 47
  br i1 %exitcond, label %for.end6, label %for.cond1.preheader

for.end6:                                         ; preds = %for.end
  ret i32 0
}

declare dso_local i32 @print_int(...) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA64_y", !8, i64 0}
!8 = !{!"long long", !4, i64 0}

