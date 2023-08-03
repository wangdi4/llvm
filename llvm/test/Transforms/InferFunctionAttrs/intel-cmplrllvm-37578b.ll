; RUN: opt < %s -S -passes=inferattrs | FileCheck %s

; Regression test for CMPLRLLVM-37578. This test is to verify that the external
; library function 'getopt_long' does not get marked with the 'argmemonly'
; attribute because the function modifies variables that are exposed by the
; library function, such as 'optind' and 'optarg'.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.option = type { ptr, i32, ptr, i32 }

@main.long_options = internal global [2 x %struct.option] [%struct.option { ptr getelementptr inbounds ([4 x i8], ptr @.str, i32 0, i32 0), i32 1, ptr null, i32 97 }, %struct.option zeroinitializer], align 16

@.str = private unnamed_addr constant [4 x i8] c"add\00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"a:\00", align 1

define i32 @main(i32 %argc, ptr %argv) {
  %option_index = alloca i32, align 4
  %argstr = alloca [3 x ptr], align 16
  %arrayidx = getelementptr inbounds [3 x ptr], ptr %argstr, i64 0, i64 0

  %call_long = call i32 @getopt_long(
    i32 noundef 3,
    ptr noundef %arrayidx,
    ptr noundef getelementptr inbounds ([3 x i8], ptr @.str.4, i64 0, i64 0),
    ptr noundef getelementptr inbounds ([2 x %struct.option], ptr @main.long_options, i64 0, i64 0),
    ptr noundef %option_index)

  ret i32 0
}

declare dso_local i32 @getopt_long(i32 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #1

attributes #1 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

; CHECK: declare dso_local i32 @getopt_long(i32 noundef, ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #[[ATTRS:[0-9]+]]
; CHECK: attributes #[[ATTRS]] = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
