; RUN: opt -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-tpv -S %s | FileCheck %s
;
; It is to test whether a composite instruction such as %9 = load i32,
; i32* bitcast (float* getelementptr inbounds (%class.t_class,
; %class.t_class* @a, i64 0, i32 2) to i32*) can be
; broken into a sequence of instructions.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.t_class = type { i8, i32, float, i32* }

@a = external dso_local thread_private global %class.t_class, align 8
@.str.4 = external hidden unnamed_addr constant [20 x i8], align 1
@.str.5 = external hidden unnamed_addr constant [82 x i8], align 1
@"@tid.addr" = external local_unnamed_addr global i32
@.gomp_critical_user_.var = external global [8 x i32]
@.kmpc_loc.0.0.11 = external hidden unnamed_addr constant { i32, i32, i32, i32, i8* }
@.kmpc_loc.0.0.15 = external hidden unnamed_addr constant { i32, i32, i32, i32, i8* }

; Function Attrs: nounwind
declare dso_local i32 @puts(i8* nocapture readonly) local_unnamed_addr #0

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #0

declare dso_local i32 @omp_get_thread_num() local_unnamed_addr #2

; Function Attrs: uwtable
define dso_local void @_ZN7t_class3subEv(%class.t_class* %this) local_unnamed_addr #3 align 2 {
entry:
  %my.tid26 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_barrier({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0.15, i32 %my.tid26)
  %0 = load i8, i8* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 0), align 8, !tbaa !2
  %conv = sext i8 %0 to i32
  %call = call i32 @omp_get_thread_num()
  %add = add nsw i32 %call, 1
  %rem = srem i32 %add, 127
  %cmp = icmp eq i32 %rem, %conv
  br i1 %cmp, label %lor.lhs.false, label %if.then

lor.lhs.false:                                    ; preds = %entry
  %1 = load i32, i32* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 1), align 4, !tbaa !9
  %call4 = call i32 @omp_get_thread_num()
  %add5 = add nsw i32 %call4, 1
  %cmp6 = icmp eq i32 %1, %add5
  br i1 %cmp6, label %lor.lhs.false7, label %if.then

lor.lhs.false7:                                   ; preds = %lor.lhs.false
  %2 = load float, float* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 2), align 8, !tbaa !10
  %call8 = call i32 @omp_get_thread_num()
  %add9 = add nsw i32 %call8, 1
  %conv10 = sitofp i32 %add9 to float
  %sub = fsub float %2, %conv10
  %3 = call float @llvm.fabs.f32(float %sub)
  %4 = fpext float %3 to double
  %cmp12 = fcmp ogt double %4, 0x3E7AD7F29ABCAF48
  br i1 %cmp12, label %if.then, label %lor.lhs.false13

lor.lhs.false13:                                  ; preds = %lor.lhs.false7
  %5 = load i32*, i32** getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 3), align 8, !tbaa !11
  %call14 = call i32 @omp_get_thread_num()
  %add15 = add nsw i32 %call14, 1
  %conv16 = sext i32 %add15 to i64
  %6 = inttoptr i64 %conv16 to i32*
  %cmp17 = icmp eq i32* %5, %6
  br i1 %cmp17, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false13, %lor.lhs.false7, %lor.lhs.false, %entry
  %call18 = call i32 @omp_get_thread_num()
  %7 = load i8, i8* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 0), align 8, !tbaa !2
  %conv19 = sext i8 %7 to i32
  %8 = load i32, i32* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 1), align 4, !tbaa !9
  %9 = load i32, i32* bitcast (float* getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 2) to i32*), align 8, !tbaa !10
  %c1 = bitcast i32 %9 to float
  %conv20 = fpext float %c1 to double
  %10 = load i32*, i32** getelementptr inbounds (%class.t_class, %class.t_class* @a, i64 0, i32 3), align 8, !tbaa !11
  %call21 = call i32 @omp_get_thread_num()
  %add22 = add nsw i32 %call21, 1
  %call23 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([82 x i8], [82 x i8]* @.str.5, i64 0, i64 0), i32 %call18, i32 %conv19, i32 %8, double %conv20, i32* %10, i32 %add22)
  %call24 = call i32 @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.4, i64 0, i64 0))
  %my.tid = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0.11, i32 %my.tid, [8 x i32]* nonnull @.gomp_critical_user_.var)
  call void @exit(i32 1) #5
  unreachable

if.end:                                           ; preds = %lor.lhs.false13
  ret void
}

declare void @__kmpc_critical({ i32, i32, i32, i32, i8* }*, i32, [8 x i32]*) local_unnamed_addr

declare void @__kmpc_barrier({ i32, i32, i32, i32, i8* }*, i32) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare float @llvm.fabs.f32(float) #4

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTS7t_class", !4, i64 0, !6, i64 4, !7, i64 8, !8, i64 16}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!"int", !4, i64 0}
!7 = !{!"float", !4, i64 0}
!8 = !{!"pointer@_ZTSPi", !4, i64 0}
!9 = !{!3, !6, i64 4}
!10 = !{!3, !7, i64 8}
!11 = !{!3, !8, i64 16}
;
; CHECK:  %{{.*}} = bitcast i8* %{{.*}} to %class.t_class*
; CHECK:  %{{.*}} = getelementptr inbounds %class.t_class, %class.t_class* %{{.*}}, i64 0, i32 2
; CHECK:  %{{.*}} = bitcast float* %{{.*}} to i32*
