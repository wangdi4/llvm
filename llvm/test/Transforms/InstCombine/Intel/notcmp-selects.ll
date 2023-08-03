; RUN: opt -passes="instcombine" -S -enable-intel-advanced-opts < %s | FileCheck %s
; CHECK-NOT: select
; CHECK-NOT: icmp{{.*}} ne
; CHECK: [[CMP:%[0-9a-z.]+]] = icmp{{.*}} eq i32
; CHECK: br i1 [[CMP]]

; Selects should be removed, and branch condition replaced with !cmp.

 target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: norecurse uwtable mustprogress
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %call = call i32 @_Z4getXv()
  %call1 = call i32 @_Z4getXv()
  %cmp = icmp ne i32 %call, %call1
  %0 = select i1 %cmp, i32 %call, i32 1
  %1 = select i1 %cmp, i32 %call, i32 9
  %cmp7 = icmp eq i32 %0, %1
  br i1 %cmp7, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %z.0 = phi i32 [ 3, %if.then ], [ 4, %if.else ]
  %call8 = call i32 (ptr, ...) @printf(ptr @.str, i32 %z.0)
  ret i32 0
}

declare dso_local i32 @_Z4getXv() local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

attributes #0 = { norecurse uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+mmx,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.1.0.MMDD)"}
