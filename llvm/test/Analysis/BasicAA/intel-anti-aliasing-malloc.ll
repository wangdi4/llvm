; RUN: opt -basic-aa -instcombine -gvn -tbaa %s -S | FileCheck %s

; CHECK-NOT: %5 = load i64, i64* %size, align 8, !tbaa !2

; ModuleID = 'second-test.cpp'
source_filename = "second-test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.data_t = type { [10 x double], i64 }

; Function Attrs: uwtable
define dso_local i64* @_Z3foov() local_unnamed_addr #0 {
entry:
  %call = tail call %struct.data_t* @_Z9somewherev()
  %call1 = tail call noalias i8* @malloc(i64 8) #3
  %0 = bitcast i8* %call1 to i64*
  %size = getelementptr inbounds %struct.data_t, %struct.data_t* %call, i64 0, i32 1, !intel-tbaa !2
  %1 = load i64, i64* %size, align 8, !tbaa !2
  %cmp13 = icmp ugt i64 %1, 0
  br i1 %cmp13, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %entry
  tail call void @_Z6escapePm(i64* %0)
  ret i64* %0

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %2 = phi i64 [ 0, %for.body.lr.ph ], [ %4, %for.body ]
  %i.014 = phi i32 [ 0, %for.body.lr.ph ], [ %inc3, %for.body ]
  store i64 %2, i64* %0, align 8, !tbaa !9
  %arrayidx = getelementptr inbounds %struct.data_t, %struct.data_t* %call, i64 0, i32 0, i64 %2, !intel-tbaa !10
  %3 = load double, double* %arrayidx, align 8, !tbaa !10
  %inc = fadd double %3, 1.000000e+00
  store double %inc, double* %arrayidx, align 8, !tbaa !10
  %inc3 = add nuw nsw i32 %i.014, 1
  %4 = zext i32 %inc3 to i64
  %5 = load i64, i64* %size, align 8, !tbaa !2
  %cmp = icmp ugt i64 %5, %4
  br i1 %cmp, label %for.body, label %for.cond.for.cond.cleanup_crit_edge
}

declare dso_local %struct.data_t* @_Z9somewherev() local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #2

declare dso_local void @_Z6escapePm(i64*) local_unnamed_addr #1

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !8, i64 80}
!3 = !{!"struct@_ZTS6data_t", !4, i64 0, !8, i64 80}
!4 = !{!"array@_ZTSA10_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"long", !6, i64 0}
!9 = !{!8, !8, i64 0}
!10 = !{!3, !5, i64 0}
