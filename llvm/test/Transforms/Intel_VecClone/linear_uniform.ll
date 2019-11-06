; Check if a stack variable is created for linear and uniform parameters and if it is loaded inside the WRN region.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4ul_foo
; CHECK: entry:
; a is a uniform parameter. Similarly to k, space is allocated in the stack and a is stored there.
; CHECK-NEXT: %alloca.a = alloca i32*
; CHECK-NEXT: store i32* %a, i32** %alloca.a
; k is a linear parameter. So, space is allocated in the stack and k is stored there.
; CHECK-NEXT: %alloca.k = alloca i32
; CHECK-NEXT: store i32 %k, i32* %alloca.k
; CHECK-LABEL: simd.begin.region
; CHECK: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.UNIFORM
; CHECK-SAME: i32** %alloca.a
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32* %alloca.k
; CHECK-SAME: i32 1
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; CHECK-NEXT: br label %simd.loop.preheader
; CHECK-LABEL: simd.loop.preheader:

; Load the parameters in simd.loop.preheader which is in the WRN region.
; CHECK-NEXT: [[LOAD_LINEAR:%load.k]] = load i32, i32* %alloca.k
; CHECK-NEXT: [[LOAD_UNIFORM:%load.a]] = load i32*, i32** %alloca.a
; CHECK-NEXT: br label %simd.loop
; CHECK-LABEL: simd.loop:

; Check if the uses of the linear and uniform loads are updated correctly.
; CHECK: [[INDEXPROM:%.*]] = sext i32 [[LOAD_LINEAR]] to i64
; CHECK: [[ARRAYIDX:%.*]] = getelementptr inbounds i32, i32* [[LOAD_UNIFORM]], i64 %stride.add

; ModuleID = 'test.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@a = common dso_local local_unnamed_addr global [4096 x i32] zeroinitializer, align 16
; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32* nocapture %a, i32 %k) local_unnamed_addr #0 {
entry:
  %idxprom = sext i32 %k to i64
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, 20
  store i32 %add, i32* %arrayidx, align 4, !tbaa !2
  ret i32 undef
}

attributes #0 = { norecurse nounwind uwtable "vector-variants"="_ZGVbN4ul_foo" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false\
" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-featur\
es"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-\
frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cp\
u"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.disable"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
