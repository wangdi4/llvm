; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CL,CHECK
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Test checks that the nested call to foo in the recursively inlined call to bar
; within main() is not inlined with the reason "Callsite is noinline"

; CHECK-CL: define dso_local noundef i32 @bar()
; CHECK-CL: call noundef i32 @foo()
; CHECK-CL: define dso_local noundef i32 @main()
; CHECK-CL: call noundef i32 @foo()
; CHECK-CL: mul nsw i32 %{{[0-9]+}}, 2

; CHECK: COMPILE FUNC: bar
; CHECK-NEXT:    -> foo{{.*}}Callsite is noinline

; CHECK: COMPILE FUNC: main
; CHECK-NEXT:    -> INLINE: bar{{.*}}Callsite is always inline (recursive)
; CHECK-NEXT:       -> foo{{.*}}Callsite is noinline

; CHECK-NOT: Not tested for inlining

; CHECK-MD: define dso_local noundef i32 @bar()
; CHECK-MD: call noundef i32 @foo()
; CHECK-MD: define dso_local noundef i32 @main()
; CHECK-MD: call noundef i32 @foo()
; CHECK-MD: mul nsw i32 %{{[0-9]+}}, 2

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @foo() #0 {
entry:
  ret i32 2048
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @bar() #0 {
entry:
  %res = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %res) #3
  %call = call noundef i32 @foo() #4
  store i32 %call, ptr %res, align 4, !tbaa !3
  %0 = load i32, ptr %res, align 4, !tbaa !3
  %mul = mul nsw i32 %0, 2
  call void @llvm.lifetime.end.p0(i64 4, ptr %res) #3
  ret i32 %mul
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local noundef i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %call = call noundef i32 @bar() #5
  ret i32 %call
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { noinline }
attributes #5 = { alwaysinline_recursive }

!llvm.module.flags = !{!0, !1} 
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
