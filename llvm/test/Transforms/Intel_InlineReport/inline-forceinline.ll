; RUN: opt -passes='inlineforceinline,cgscc(inline)' -inline-report=0xe807 -inline-forceinline < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BOTH
; RUN: opt -passes='inlinereportsetup,inlineforceinline,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-forceinline < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-BOTH,CHECK-META

; Test should force inlining of routines marked with the inline keyword and
; verify with inline report. Output should be identical to if forceinline
; was used in source.

; CHECK: define dso_local noundef i32 @main()
; CHECK: call{{.*}}f1
; CHECK: add nsw i32 %{{[0-9]+}}, 1
; CHECK: call{{.*}}f1
; CHECK: call{{.*}}f2
; CHECK: %call.i = call noundef i32 @f1()
; CHECK: add nsw i32 %{{[0-9]+}}, 3
; CHECK: call{{.*}}f4
; CHECK: add nsw i32 %{{[0-9]+}}, 7
; CHECK: call{{.*}}f4
; CHECK: call{{.*}}f4
; CHECK: add nsw i32 %{{[0-9]+}}, 14

; CHECK-BOTH: COMPILE FUNC: f2
; CHECK-BOTH-NEXT:    -> f1{{.*}}Callee has noinline attribute

; CHECK-BOTH: COMPILE FUNC: f4
; CHECK-BOTH-NEXT:    -> INLINE: f3{{.*}}Callee is single basic block

; CHECK-BOTH: COMPILE FUNC: f5
; CHECK-BOTH-NEXT:    -> f4{{.*}}Callee has noinline attribute

; CHECK-BOTH: COMPILE FUNC: f6
; CHECK-BOTH-NEXT:    -> INLINE: f4{{.*}}Callsite is always inline
; CHECK-BOTH-NEXT:       -> INLINE: f3{{.*}}Callee is single basic block

; CHECK-BOTH: COMPILE FUNC: main
; CHECK-BOTH-NEXT:    -> f1{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> INLINE: f1{{.*}}Callsite is always inline
; CHECK-BOTH-NEXT:    -> f1{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> f2{{.*}}Callsite is noinline
; CHECK-BOTH-NEXT:    -> INLINE: f2{{.*}}Callsite is always inline
; CHECK-BOTH-NEXT:       -> f1{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> INLINE: f3{{.*}}Callee is single basic block
; CHECK-BOTH-NEXT:    -> f4{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> INLINE: f4{{.*}}Callsite is always inline
; CHECK-BOTH-NEXT:       -> INLINE: f3{{.*}}Callee is single basic block
; CHECK-BOTH-NEXT:    -> f4{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> INLINE: f5{{.*}}Callsite is always inline (recursive)
; CHECK-BOTH-NEXT:       -> f4{{.*}}Callee has noinline attribute
; CHECK-BOTH-NEXT:    -> INLINE: f6{{.*}}Callee is single basic block
; CHECK-BOTH-NEXT:       -> INLINE: f4{{.*}}Callsite is always inline
; CHECK-BOTH-NEXT:          -> INLINE: f3{{.*}}Callee is single basic block

; CHECK-META: define dso_local noundef i32 @main()
; CHECK-META: call{{.*}}f1
; CHECK-META: add nsw i32 %{{[0-9]+}}, 1
; CHECK-META: call{{.*}}f1
; CHECK-META: call{{.*}}f2
; CHECK-META: %call.i = call noundef i32 @f1()
; CHECK-META: add nsw i32 %{{[0-9]+}}, 3
; CHECK-META: call{{.*}}f4
; CHECK-META: add nsw i32 %{{[0-9]+}}, 7
; CHECK-META: call{{.*}}f4
; CHECK-META: call{{.*}}f4
; CHECK-META: add nsw i32 %{{[0-9]+}}, 14


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @f1() #0 {
entry:
  ret i32 1
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @f2() #1 {
entry:
  %call = call noundef i32 @f1()
  %add = add nsw i32 2, %call
  ret i32 %add
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @f3() #1 {
entry:
  ret i32 3
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @f4() #0 {
entry:
  %call = call noundef i32 @f3()
  %add = add nsw i32 4, %call
  ret i32 %add
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @f5() #1 {
entry:
  %call = call noundef i32 @f4()
  %add = add nsw i32 5, %call
  ret i32 %add
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @f6() #1 {
entry:
  %call = call noundef i32 @f4() #4
  %mul = mul nsw i32 2, %call
  ret i32 %mul
}

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %res = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %res) #5
  store i32 0, ptr %res, align 4, !tbaa !3
  %call = call noundef i32 @f1()
  %0 = load i32, ptr %res, align 4, !tbaa !3
  %add = add nsw i32 %0, %call
  store i32 %add, ptr %res, align 4, !tbaa !3
  %call1 = call noundef i32 @f1() #4
  %1 = load i32, ptr %res, align 4, !tbaa !3
  %add2 = add nsw i32 %1, %call1
  store i32 %add2, ptr %res, align 4, !tbaa !3
  %call3 = call noundef i32 @f1() #6
  %2 = load i32, ptr %res, align 4, !tbaa !3
  %add4 = add nsw i32 %2, %call3
  store i32 %add4, ptr %res, align 4, !tbaa !3
  %call5 = call noundef i32 @f2() #7
  %3 = load i32, ptr %res, align 4, !tbaa !3
  %add6 = add nsw i32 %3, %call5
  store i32 %add6, ptr %res, align 4, !tbaa !3
  %call7 = call noundef i32 @f2() #4
  %4 = load i32, ptr %res, align 4, !tbaa !3
  %add8 = add nsw i32 %4, %call7
  store i32 %add8, ptr %res, align 4, !tbaa !3
  %call9 = call noundef i32 @f3()
  %5 = load i32, ptr %res, align 4, !tbaa !3
  %add10 = add nsw i32 %5, %call9
  store i32 %add10, ptr %res, align 4, !tbaa !3
  %call11 = call noundef i32 @f4() #7
  %6 = load i32, ptr %res, align 4, !tbaa !3
  %add12 = add nsw i32 %6, %call11
  store i32 %add12, ptr %res, align 4, !tbaa !3
  %call13 = call noundef i32 @f4() #4
  %7 = load i32, ptr %res, align 4, !tbaa !3
  %add14 = add nsw i32 %7, %call13
  store i32 %add14, ptr %res, align 4, !tbaa !3
  %call15 = call noundef i32 @f4() #6
  %8 = load i32, ptr %res, align 4, !tbaa !3
  %add16 = add nsw i32 %8, %call15
  store i32 %add16, ptr %res, align 4, !tbaa !3
  %call17 = call noundef i32 @f5() #6
  %9 = load i32, ptr %res, align 4, !tbaa !3
  %add18 = add nsw i32 %9, %call17
  store i32 %add18, ptr %res, align 4, !tbaa !3
  %call19 = call noundef i32 @f6()
  %10 = load i32, ptr %res, align 4, !tbaa !3
  %add20 = add nsw i32 %10, %call19
  store i32 %add20, ptr %res, align 4, !tbaa !3
  %11 = load i32, ptr %res, align 4, !tbaa !3
  call void @llvm.lifetime.end.p0(i64 4, ptr %res) #5
  ret i32 %11
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { mustprogress noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { inlinehint }
attributes #5 = { nounwind }
attributes #6 = { inlinehint_recursive }
attributes #7 = { noinline }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
