; RUN: opt -passes='function(sroa),module(print<inline-report>)' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-DEF %s
; RUN: opt -passes='inlinereportsetup,function(sroa),inlinereportemitter' -disable-output -inline-report=0xe886 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-DEF %s
; RUN: opt -passes='function(sroa),module(print<inline-report>)' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-EXT,CHECK-EXT-CL %s
; RUN: opt -passes='inlinereportsetup,function(sroa),inlinereportemitter' -disable-output -inline-report=0xea86 < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-EXT,CHECK-EXT-MD %s

; Test the inline report with skippable intrinsics enabled and disabled.

; CHECK: COMPILE FUNC: main
; CHECK-EXT-MD: llvm.lifetime.start.p0 {{.*}}Callee is intrinsic
; CHECK-EXT: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK-EXT: DELETE: llvm.lifetime.end.p0 {{.*}}Dead code
; CHECK-EXT-CL: llvm.lifetime.start.p0 {{.*}}Callee is intrinsic
; CHECK: EXTERN: fprintf
; CHECK: EXTERN: _Z8ConvolveiiPfS_Pii
; CHECK: EXTERN: fprintf
; CHECK: EXTERN: fprintf
; CHECK-EXT: llvm.lifetime.end.p0 {{.*}}Callee is intrinsic
; CHECK-DEF-NOT: DELETE: llvm.lifetime.start.p0
; CHECK-DEF-NOT: DELETE: llvm.lifetime.end.p0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Kernel = dso_local global [100 x i32] zeroinitializer, align 16
@stderr = external dso_local global ptr, align 8
@.str = private unnamed_addr constant [8 x i8] c"\0A %d  \0A\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"\0A Pass \0A\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"\0A Fail \0A\00", align 1

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %P = alloca [20000 x float], align 16
  %sum = alloca [100 x float], align 16
  %height = alloca i32, align 4
  %width = alloca i32, align 4
  %channel = alloca i32, align 4
  %i = alloca i32, align 4
  %i3 = alloca i32, align 4
  %result = alloca float, align 4
  %i13 = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  %j = alloca i32, align 4
  %iresult = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 80000, ptr %P) #4
  call void @llvm.lifetime.start.p0(i64 400, ptr %sum) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %height) #4
  store i32 100, ptr %height, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %width) #4
  store i32 100, ptr %width, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %channel) #4
  store i32 4, ptr %channel, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  store i32 0, ptr %i, align 4, !tbaa !3
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %0 = load i32, ptr %i, align 4, !tbaa !3
  %1 = load i32, ptr %width, align 4, !tbaa !3
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i3) #4
  store i32 0, ptr %i3, align 4, !tbaa !3
  br label %for.cond4

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr %i, align 4, !tbaa !3
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [100 x float], ptr %sum, i64 0, i64 %idxprom, !intel-tbaa !7
  store float 0.000000e+00, ptr %arrayidx, align 4, !tbaa !7
  %3 = load i32, ptr %i, align 4, !tbaa !3
  %4 = load i32, ptr %i, align 4, !tbaa !3
  %idxprom1 = sext i32 %4 to i64
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @Kernel, i64 0, i64 %idxprom1, !intel-tbaa !10
  store i32 %3, ptr %arrayidx2, align 4, !tbaa !10
  %5 = load i32, ptr %i, align 4, !tbaa !3
  %inc = add nsw i32 %5, 1
  store i32 %inc, ptr %i, align 4, !tbaa !3
  br label %for.cond, !llvm.loop !12

for.cond4:                                        ; preds = %for.body7, %for.cond.cleanup
  %6 = load i32, ptr %i3, align 4, !tbaa !3
  %cmp5 = icmp slt i32 %6, 20000
  br i1 %cmp5, label %for.body7, label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i3) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %result) #4
  store float 0.000000e+00, ptr %result, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 4, ptr %i13) #4
  store i32 0, ptr %i13, align 4, !tbaa !3
  br label %for.cond14

for.body7:                                        ; preds = %for.cond4
  %7 = load i32, ptr %i3, align 4, !tbaa !3
  %rem = srem i32 %7, 10
  %conv = sitofp i32 %rem to float
  %8 = load i32, ptr %i3, align 4, !tbaa !3
  %idxprom8 = sext i32 %8 to i64
  %arrayidx9 = getelementptr inbounds [20000 x float], ptr %P, i64 0, i64 %idxprom8, !intel-tbaa !15
  store float %conv, ptr %arrayidx9, align 4, !tbaa !15
  %9 = load i32, ptr %i3, align 4, !tbaa !3
  %inc11 = add nsw i32 %9, 1
  store i32 %inc11, ptr %i3, align 4, !tbaa !3
  br label %for.cond4, !llvm.loop !17

for.cond14:                                       ; preds = %for.cond.cleanup24, %for.cond.cleanup6
  %10 = load i32, ptr %i13, align 4, !tbaa !3
  %cmp15 = icmp slt i32 %10, 2000000
  br i1 %cmp15, label %for.body17, label %for.cond.cleanup16

for.cond.cleanup16:                               ; preds = %for.cond14
  store i32 8, ptr %cleanup.dest.slot, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i13) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %iresult) #4
  %11 = load float, ptr %result, align 4, !tbaa !14
  %conv34 = fptosi float %11 to i32
  store i32 %conv34, ptr %iresult, align 4, !tbaa !3
  %12 = load ptr, ptr @stderr, align 8, !tbaa !18
  %13 = load i32, ptr %iresult, align 4, !tbaa !3
  %call = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %12, ptr noundef @.str, i32 noundef %13)
  %14 = load i32, ptr %iresult, align 4, !tbaa !3
  %cmp35 = icmp eq i32 %14, 4600
  br i1 %cmp35, label %if.then, label %if.else

for.body17:                                       ; preds = %for.cond14
  %15 = load i32, ptr %height, align 4, !tbaa !3
  %16 = load i32, ptr %width, align 4, !tbaa !3
  %arrayidx18 = getelementptr inbounds [100 x float], ptr %sum, i64 0, i64 0, !intel-tbaa !7
  %arrayidx19 = getelementptr inbounds [20000 x float], ptr %P, i64 0, i64 0, !intel-tbaa !15
  %17 = load i32, ptr %channel, align 4, !tbaa !3
  call void @_Z8ConvolveiiPfS_Pii(i32 noundef %15, i32 noundef %16, ptr noundef %arrayidx18, ptr noundef %arrayidx19, ptr noundef @Kernel, i32 noundef %17)
  %arrayidx20 = getelementptr inbounds [100 x float], ptr %sum, i64 0, i64 0, !intel-tbaa !7
  %18 = load float, ptr %arrayidx20, align 16, !tbaa !7
  %arrayidx21 = getelementptr inbounds [100 x float], ptr %sum, i64 0, i64 3, !intel-tbaa !7
  %19 = load float, ptr %arrayidx21, align 4, !tbaa !7
  %add = fadd fast float %18, %19
  store float %add, ptr %result, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #4
  store i32 0, ptr %j, align 4, !tbaa !3
  br label %for.cond22

for.cond22:                                       ; preds = %for.body25, %for.body17
  %20 = load i32, ptr %j, align 4, !tbaa !3
  %21 = load i32, ptr %width, align 4, !tbaa !3
  %cmp23 = icmp slt i32 %20, %21
  br i1 %cmp23, label %for.body25, label %for.cond.cleanup24

for.cond.cleanup24:                               ; preds = %for.cond22
  store i32 11, ptr %cleanup.dest.slot, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #4
  %22 = load i32, ptr %i13, align 4, !tbaa !3
  %inc32 = add nsw i32 %22, 1
  store i32 %inc32, ptr %i13, align 4, !tbaa !3
  br label %for.cond14, !llvm.loop !20

for.body25:                                       ; preds = %for.cond22
  %23 = load i32, ptr %j, align 4, !tbaa !3
  %idxprom26 = sext i32 %23 to i64
  %arrayidx27 = getelementptr inbounds [100 x float], ptr %sum, i64 0, i64 %idxprom26, !intel-tbaa !7
  store float 0.000000e+00, ptr %arrayidx27, align 4, !tbaa !7
  %24 = load i32, ptr %j, align 4, !tbaa !3
  %inc29 = add nsw i32 %24, 1
  store i32 %inc29, ptr %j, align 4, !tbaa !3
  br label %for.cond22, !llvm.loop !21

if.then:                                          ; preds = %for.cond.cleanup16
  %25 = load ptr, ptr @stderr, align 8, !tbaa !18
  %call36 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %25, ptr noundef @.str.1)
  br label %if.end

if.else:                                          ; preds = %for.cond.cleanup16
  %26 = load ptr, ptr @stderr, align 8, !tbaa !18
  %call37 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %26, ptr noundef @.str.2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.lifetime.end.p0(i64 4, ptr %iresult) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %result) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %channel) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %width) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %height) #4
  call void @llvm.lifetime.end.p0(i64 400, ptr %sum) #4
  call void @llvm.lifetime.end.p0(i64 80000, ptr %P) #4
  %27 = load i32, ptr %retval, align 4
  ret i32 %27
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare dso_local void @_Z8ConvolveiiPfS_Pii(i32 noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) #2

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) #3

attributes #0 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA100_f", !9, i64 0}
!9 = !{!"float", !5, i64 0}
!10 = !{!11, !4, i64 0}
!11 = !{!"array@_ZTSA100_i", !4, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!9, !9, i64 0}
!15 = !{!16, !9, i64 0}
!16 = !{!"array@_ZTSA20000_f", !9, i64 0}
!17 = distinct !{!17, !13}
!18 = !{!19, !19, i64 0}
!19 = !{!"pointer@_ZTSP8_IO_FILE", !5, i64 0}
!20 = distinct !{!20, !13}
!21 = distinct !{!21, !13}
