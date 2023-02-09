; RUN: opt -opaque-pointers=0 < %s -S -passes=inferattrs | FileCheck %s

; Regression test for CMPLRLLVM-37578. This test is to verify that the external
; library function 'getopt_long_only' does not get marked with the 'argmemonly'
; attribute because the function modifies variables that are exposed by the
; library function, such as 'optind' and 'optarg'.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.option = type { i8*, i32, i32*, i32 }

@main.long_options = internal global [2 x %struct.option] [%struct.option { i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 1, i32* null, i32 97 }, %struct.option zeroinitializer], align 16

@.str = private unnamed_addr constant [4 x i8] c"add\00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"a:\00", align 1

define i32 @main(i32 %argc, i8* %argv) {
  %option_index = alloca i32, align 4
  %argstr = alloca [3 x i8*], align 16
  %arrayidx = getelementptr inbounds [3 x i8*], [3 x i8*]* %argstr, i64 0, i64 0

  %call_long = call i32 @getopt_long_only(
    i32 noundef 3,
    i8** noundef %arrayidx,
    i8* noundef getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i64 0, i64 0),
    %struct.option* noundef getelementptr inbounds ([2 x %struct.option], [2 x %struct.option]* @main.long_options, i64 0, i64 0),
    i32* noundef %option_index)

  ret i32 0
}

declare dso_local i32 @getopt_long_only(i32 noundef, i8** noundef, i8* noundef, %struct.option* noundef, i32* noundef) local_unnamed_addr #1

attributes #1 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

; CHECK: declare dso_local i32 @getopt_long_only(i32 noundef, i8** noundef, i8* noundef, %struct.option* noundef, i32* noundef) local_unnamed_addr #[[ATTRS:[0-9]+]]
; CHECK: attributes #[[ATTRS]] = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
