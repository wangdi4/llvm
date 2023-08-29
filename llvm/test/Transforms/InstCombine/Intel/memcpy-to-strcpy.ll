; RUN: opt -passes="instcombine" < %s -S | grep "getelementptr"
; RUN: opt -passes="instcombine" < %s -S | grep "getelementptr"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.T = type { double, double }

; Function Attrs: nounwind uwtable
define void @test_memcpy2strcpy(ptr %t1, ptr %t2) #0 {
entry:
  call void @llvm.memcpy.p0.p0.i64(ptr %t2, ptr %t1, i64 16, i1 false), !tbaa.struct !1
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0.p0.i64(ptr nocapture, ptr nocapture readonly, i64, i1) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1183)"}
!1 = !{i64 0, i64 8, !2, i64 8, i64 8, !2}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
