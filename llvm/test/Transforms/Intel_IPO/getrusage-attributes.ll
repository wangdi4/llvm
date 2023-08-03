; RUN: opt < %s -passes='inferattrs' -S 2>&1 | FileCheck %s

; Test checks if attributes for @getrusage are inferred correctly.
; See "CHECK" lines inlined blelow.

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.rusage = type { %struct.timeval, %struct.timeval, %union.anon, %union.anon.0, %union.anon.1, %union.anon.2, %union.anon.3, %union.anon.4, %union.anon.5, %union.anon.6, %union.anon.7, %union.anon.8, %union.anon.9, %union.anon.10, %union.anon.11, %union.anon.12 }
%struct.timeval = type { i64, i64 }
%union.anon = type { i64 }
%union.anon.0 = type { i64 }
%union.anon.1 = type { i64 }
%union.anon.2 = type { i64 }
%union.anon.3 = type { i64 }
%union.anon.4 = type { i64 }
%union.anon.5 = type { i64 }
%union.anon.6 = type { i64 }
%union.anon.7 = type { i64 }
%union.anon.8 = type { i64 }
%union.anon.9 = type { i64 }
%union.anon.10 = type { i64 }
%union.anon.11 = type { i64 }
%union.anon.12 = type { i64 }

@.str = private unnamed_addr constant [16 x i8] c"ru_maxrss: %li\0A\00", align 1
@.str.1 = private unnamed_addr constant [16 x i8] c"ru_minflt: %li\0A\00", align 1
@.str.2 = private unnamed_addr constant [16 x i8] c"ru_majflt: %li\0A\00", align 1
@.str.3 = private unnamed_addr constant [17 x i8] c"ru_inblock: %li\0A\00", align 1
@.str.4 = private unnamed_addr constant [17 x i8] c"ru_oublock: %li\0A\00", align 1
@.str.5 = private unnamed_addr constant [15 x i8] c"ru_nvcsw: %li\0A\00", align 1
@.str.6 = private unnamed_addr constant [16 x i8] c"ru_nivcsw: %li\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %rusage = alloca %struct.rusage, align 8
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 144, ptr %rusage) #4
  %0 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 2, !intel-tbaa !3
  store i64 -1, ptr %0, align 8, !tbaa !9
  %1 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 6, !intel-tbaa !10
  store i64 -1, ptr %1, align 8, !tbaa !9
  %2 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 7, !intel-tbaa !11
  store i64 -1, ptr %2, align 8, !tbaa !9
  %3 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 9, !intel-tbaa !12
  store i64 -1, ptr %3, align 8, !tbaa !9
  %4 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 10, !intel-tbaa !13
  store i64 -1, ptr %4, align 8, !tbaa !9
  %5 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 14, !intel-tbaa !14
  store i64 -1, ptr %5, align 8, !tbaa !9
  %6 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 15, !intel-tbaa !15
  store i64 -1, ptr %6, align 8, !tbaa !9
  %call = call i32 @getrusage(i32 noundef 0, ptr noundef %rusage) #4
  %7 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 2, !intel-tbaa !3
  %8 = load i64, ptr %7, align 8, !tbaa !9
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %8)
  %9 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 6, !intel-tbaa !10
  %10 = load i64, ptr %9, align 8, !tbaa !9
  %call2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i64 noundef %10)
  %11 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 7, !intel-tbaa !11
  %12 = load i64, ptr %11, align 8, !tbaa !9
  %call3 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i64 noundef %12)
  %13 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 9, !intel-tbaa !12
  %14 = load i64, ptr %13, align 8, !tbaa !9
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i64 noundef %14)
  %15 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 10, !intel-tbaa !13
  %16 = load i64, ptr %15, align 8, !tbaa !9
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str.4, i64 noundef %16)
  %17 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 14, !intel-tbaa !14
  %18 = load i64, ptr %17, align 8, !tbaa !9
  %call6 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i64 noundef %18)
  %19 = getelementptr inbounds %struct.rusage, ptr %rusage, i32 0, i32 15, !intel-tbaa !15
  %20 = load i64, ptr %19, align 8, !tbaa !9
  %call7 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i64 noundef %20)
  call void @llvm.lifetime.end.p0(i64 144, ptr %rusage) #4
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1
; CHECK: declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: nounwind
declare dso_local i32 @getrusage(i32 noundef, ptr noundef) #2
; CHECK: ; Function Attrs: nofree nounwind
; CHECK-NEXT: declare dso_local i32 @getrusage(i32 noundef, ptr noundef writeonly)

declare dso_local i32 @printf(ptr noundef, ...) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !7, i64 32}
!4 = !{!"struct@rusage", !5, i64 0, !5, i64 16, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !7, i64 88, !7, i64 96, !7, i64 104, !7, i64 112, !7, i64 120, !7, i64 128, !7, i64 136}
!5 = !{!"struct@timeval", !6, i64 0, !6, i64 8}
!6 = !{!"long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!7, !7, i64 0}
!10 = !{!4, !7, i64 64}
!11 = !{!4, !7, i64 72}
!12 = !{!4, !7, i64 88}
!13 = !{!4, !7, i64 96}
!14 = !{!4, !7, i64 128}
!15 = !{!4, !7, i64 136}
