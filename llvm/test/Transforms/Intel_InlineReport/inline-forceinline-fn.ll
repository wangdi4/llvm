; RUN: opt -passes='inlineforceinline,cgscc(inline)' -inline-report=0xe807 -inline-forceinline < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CL,CHECK
; RUN: opt -passes='inlinereportsetup,inlineforceinline,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-forceinline < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Test should force inlining of routines marked with the inline keyword and
; verify with inline report. Output should be identical to if forceinline
; was used in source.

; CHECK-CL: define dso_local noundef i32 @main()
; CHECK-CL: store i32 2048
; CHECK-CL: add nsw i32 %{{[0-9]+}}, 2048

; CHECK: COMPILE FUNC: main
; CHECK-NEXT:    -> INLINE: foo{{.*}}Callee is always inline
; CHECK-NEXT:    -> INLINE: foo{{.*}}Callee is always inline

; CHECK-MD: define dso_local noundef i32 @main()
; CHECK-MD: store i32 2048
; CHECK-MD: add nsw i32 %{{[0-9]+}}, 2048


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$foo = comdat any

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %res = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %res) #3
  %call = call noundef i32 @foo()
  store i32 %call, ptr %res, align 4, !tbaa !3
  %call1 = call noundef i32 @foo()
  %0 = load i32, ptr %res, align 4, !tbaa !3
  %add = add nsw i32 %0, %call1
  store i32 %add, ptr %res, align 4, !tbaa !3
  %1 = load i32, ptr %res, align 4, !tbaa !3
  call void @llvm.lifetime.end.p0(i64 4, ptr %res) #3
  ret i32 %1
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: inlinehint mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i32 @foo() #2 comdat {
entry:
  ret i32 2048
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { inlinehint mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
