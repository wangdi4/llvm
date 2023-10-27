
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CL,CHECK
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD,CHECK

; Test checks precedence of inlining attributes. An 'alwaysinline' callsite
; should inline a function with the 'noinline' attribute (first call to foo
; below). An 'alwaysinline_recursive' callsite should never inline a function
; with the 'noinline' attribute (second call to foo below).


; CHECK-CL: define dso_local noundef i32 @main()
; CHECK-CL: %call.i = call noundef i32 @bar()
; CHECK-CL-NEXT: %mul.i = mul nsw i32 3, %call.i
; CHECK-CL: %call2 = call noundef i32 @foo() #4
; CHECK-CL-NOT: call{{.*}}i32 @foo() 
; CHECK-CL: ret i32

; CHECK: COMPILE FUNC: foo
; CHECK-NEXT:    bar{{.*}}Callee has noinline attribute

; CHECK: COMPILE FUNC: main
; CHECK-NEXT:    INLINE: foo{{.*}}Callsite is always inline
; CHECK-NEXT:       bar{{.*}}Callee has noinline attribute
; CHECK-NEXT:    foo{{.*}}Callee has noinline attribute

; CHECK-MD: define dso_local noundef i32 @main()
; CHECK-MD: %call.i = call noundef i32 @bar()
; CHECK-MD-NEXT: %mul.i = mul nsw i32 3, %call.i
; CHECK-MD: %call2 = call noundef i32 @foo() #4
; CHECK-MD-NOT: call{{.*}}i32 @foo() 
; CHECK-MD: ret i32

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @bar() #0 {
entry:
  ret i32 17
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @foo() #0 {
entry:
  %call = call noundef i32 @bar()
  %mul = mul nsw i32 3, %call
  ret i32 %mul
}

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  %res = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %res) #3
  store i32 0, ptr %res, align 4, !tbaa !3
  %call1 = call noundef i32 @foo() #4
  %0 = load i32, ptr %res, align 4, !tbaa !3
  %add = add nsw i32 %0, %call1
  store i32 %add, ptr %res, align 4, !tbaa !3
  %call2 = call noundef i32 @foo() #5
  %1 = load i32, ptr %res, align 4, !tbaa !3
  %add2 = add nsw i32 %1, %call2
  store i32 %add2, ptr %res, align 4, !tbaa !3
  %2 = load i32, ptr %res, align 4, !tbaa !3
  call void @llvm.lifetime.end.p0(i64 4, ptr %res) #3
  ret i32 %2
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { mustprogress noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind }
attributes #4 = { alwaysinline }
attributes #5 = { alwaysinline_recursive }

!llvm.module.flags = !{!0, !1} 
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}

