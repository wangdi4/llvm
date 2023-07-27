; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='pgo-icall-prom,ip-cloning,ipsccp,inline' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CL
; RUN: opt -passes='inlinereportsetup,pgo-icall-prom,ip-cloning,ipsccp,inline,inlinereportemitter' -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-MD

; Check that PGO indirect call promotion places direct call specializations underneath
; an indirect call, does not cause the inlining report to die when the indirect call
; is converted to a direct call by IPSCCP.

; Check the form of the classic inlining report

; CHECK-CL-LABEL: COMPILE FUNC: driver
; CHECK-CL: DELETE:
; CHECK-CL: DELETE: (PGO) bar
; CHECK-CL-LABEL: COMPILE FUNC: bar
; CHECK-CL-LABEL: COMPILE FUNC: main
; CHECK-CL: EXTERN: __isoc99_scanf
; CHECK-CL: driver.1 {{.*}}Callee has noinline attribute
; CHECK-CL: driver.2 {{.*}}Callee has noinline attribute
; CHECK-CL-LABEL: COMPILE FUNC: driver.1
; CHECK-CL: INLINE: foo {{.*}}Callsite is hot
; CHECK-CL: DELETE: (PGO) bar
; CHECK-CL: goo {{.*}}Callee has noinline attribute
; CHECK-CL-LABEL: COMPILE FUNC: driver.2
; CHECK-CL: DELETE:
; CHECK-CL: (PGO) bar {{.*}}Callee has noinline attribute
; CHECK-CL-LABEL: COMPILE FUNC: goo
; CHECK-CL-LABEL: COMPILE FUNC: foo
; CHECK-CL: goo {{.*}}Callee has noinline attribute

; Check the form of the metadata inlining report

; CHECK-MD-LABEL: COMPILE FUNC: main
; CHECK-MD: EXTERN: __isoc99_scanf
; CHECK-MD: driver.1 {{.*}}Callee has noinline attribute
; CHECK-MD: driver.2 {{.*}}Callee has noinline attribute
; CHECK-MD-LABEL: COMPILE FUNC: driver
; CHECK-MD: DELETE:
; CHECK-MD: DELETE: (PGO) bar
; CHECK-MD-LABEL: COMPILE FUNC: foo
; CHECK-MD: goo {{.*}}Callee has noinline attribute
; CHECK-MD-LABEL: COMPILE FUNC: goo
; CHECK-MD-LABEL: COMPILE FUNC: bar
; CHECK-MD-LABEL: COMPILE FUNC: driver.1
; CHECK-MD: INLINE: foo {{.*}}Callsite is hot
; CHECK-MD: goo {{.*}}Callee has noinline attribute
; CHECK-MD-LABEL: COMPILE FUNC: driver.2
; CHECK-MD: DELETE:
; CHECK-MD: (PGO) bar {{.*}}Newly created callsite

; ModuleID = 'sm9.c'
source_filename = "sm9.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"%d %d\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 !prof !32 {
entry:
  %N = alloca i32, align 4
  %M = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %N) #5
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %M) #5
  %call = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef nonnull %N, ptr noundef nonnull %M), !intel-profx !33
  %0 = load i32, ptr %N, align 4, !tbaa !34
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %I.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %s.0 = phi i32 [ 0, %entry ], [ %s.1, %for.inc ]
  %cmp = icmp ult i32 %I.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !prof !38

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %M) #5
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %N) #5
  ret i32 %s.0

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %M, align 4, !tbaa !34
  %cmp1 = icmp eq i32 %I.0, %1
  br i1 %cmp1, label %if.then, label %if.else, !prof !39

if.then:                                          ; preds = %for.body
  %call2 = call fastcc i32 @driver(ptr noundef nonnull @foo), !intel-profx !40
  br label %for.inc

if.else:                                          ; preds = %for.body
  %call3 = call fastcc i32 @driver(ptr noundef nonnull @bar), !intel-profx !41
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %call2.pn = phi i32 [ %call2, %if.then ], [ %call3, %if.else ]
  %s.1 = add nsw i32 %s.0, %call2.pn
  %inc = add i32 %I.0, 1
  br label %for.cond, !llvm.loop !42
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @__isoc99_scanf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: inlinehint noinline nounwind uwtable
define internal fastcc i32 @driver(ptr noundef %fp0) unnamed_addr #3 !prof !44 !PGOFuncName !45 {
entry:
  %call = call i32 %fp0() #5, !prof !46, !callees !47, !intel-profx !41
  ret i32 %call
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: cold noinline nounwind uwtable
define internal i32 @foo() #6 !prof !48 {
entry:
  %call = call fastcc i32 @goo(), !intel-profx !40
  ret i32 5
}

; Function Attrs: cold noinline nounwind uwtable
define internal fastcc i32 @goo() unnamed_addr #4 !prof !48 {
entry:
  ret i32 undef
}

; Function Attrs: inlinehint noinline nounwind uwtable
define internal i32 @bar() #3 !prof !44 !PGOFuncName !49 {
entry:
  ret i32 -5
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { inlinehint noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { cold noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { nounwind }
attributes #6 = { cold nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!31}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 1, !"ProfileSummary", !3}
!3 = !{!4, !5, !6, !7, !8, !9, !10, !11, !12, !13}
!4 = !{!"ProfileFormat", !"InstrProf"}
!5 = !{!"TotalCount", i64 30001}
!6 = !{!"MaxCount", i64 10000}
!7 = !{!"MaxInternalCount", i64 10000}
!8 = !{!"MaxFunctionCount", i64 10000}
!9 = !{!"NumCounts", i64 7}
!10 = !{!"NumFunctions", i64 5}
!11 = !{!"IsPartialProfile", i64 0}
!12 = !{!"PartialProfileRatio", double 0.000000e+00}
!13 = !{!"DetailedSummary", !14}
!14 = !{!15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30}
!15 = !{i32 10000, i64 10000, i32 3}
!16 = !{i32 100000, i64 10000, i32 3}
!17 = !{i32 200000, i64 10000, i32 3}
!18 = !{i32 300000, i64 10000, i32 3}
!19 = !{i32 400000, i64 10000, i32 3}
!20 = !{i32 500000, i64 10000, i32 3}
!21 = !{i32 600000, i64 10000, i32 3}
!22 = !{i32 700000, i64 10000, i32 3}
!23 = !{i32 800000, i64 10000, i32 3}
!24 = !{i32 900000, i64 10000, i32 3}
!25 = !{i32 950000, i64 10000, i32 3}
!26 = !{i32 990000, i64 10000, i32 3}
!27 = !{i32 999000, i64 10000, i32 3}
!28 = !{i32 999900, i64 10000, i32 3}
!29 = !{i32 999990, i64 10000, i32 3}
!30 = !{i32 999999, i64 10000, i32 3}
!31 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!32 = !{!"function_entry_count", i64 1}
!33 = !{!"intel_profx", i64 1}
!34 = !{!35, !35, i64 0}
!35 = !{!"int", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C/C++ TBAA"}
!38 = !{!"branch_weights", i32 10000, i32 1}
!39 = !{!"branch_weights", i32 0, i32 10000}
!40 = !{!"intel_profx", i64 0}
!41 = !{!"intel_profx", i64 10000}
!42 = distinct !{!42, !43}
!43 = !{!"llvm.loop.mustprogress"}
!44 = !{!"function_entry_count", i64 10000}
!45 = !{!"sm9.c:driver"}
!46 = !{!"VP", i32 0, i64 10000, i64 -6869725554262207656, i64 10000}
!47 = !{ptr @bar, ptr @foo}
!48 = !{!"function_entry_count", i64 0}
!49 = !{!"sm9.c:bar"}
; end INTEL_FEATURE_SW_ADVANCED
