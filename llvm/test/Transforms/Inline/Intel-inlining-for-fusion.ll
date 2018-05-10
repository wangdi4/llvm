; INTEL CUSTOMIZATION:

; RUN: opt -inline -inlining-for-fusion-heuristics=true -inline-threshold=20 -inline-for-fusion-min-arg-refs=3 -inline-report=7 < %s -S 2>&1 | FileCheck %s

; Test checks that inlining happens for all foo() call sites. The inlining is supposed to be followed by loop fusion and vectorization.

; CHECK: COMPILE FUNC: bar
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; CHECK: COMPILE FUNC: baz
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; CHECK-NOT: call i32 @foo



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common global [100 x i32] zeroinitializer, align 16
@arr2 = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %ptr1, i32* %ptr2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %ptr1.addr.021 = phi i32* [ %ptr1, %entry ], [ %add.ptr, %for.body ]
  %ptr2.addr.020 = phi i32* [ %ptr2, %entry ], [ %add.ptr7, %for.body ]
  %sum.019 = phi i32 [ 0, %entry ], [ %add6, %for.body ]
  %i.018 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load i32, i32* %ptr1.addr.021, align 4, !tbaa !2
  %1 = load i32, i32* %ptr2.addr.020, align 4, !tbaa !2
  %sub = sub nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %ptr1.addr.021, i64 1
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, i32* %ptr2.addr.020, i64 1
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %sub4 = sub nsw i32 %2, %3
  %shl = shl i32 %sub, 1
  %shl5 = shl i32 %sub4, 2
  %add = add i32 %shl, %sum.019
  %add6 = add i32 %add, %shl5
  %inc = add nuw nsw i32 %i.018, 1
  %add.ptr = getelementptr inbounds i32, i32* %ptr1.addr.021, i64 2
  %add.ptr7 = getelementptr inbounds i32, i32* %ptr2.addr.020, i64 2
  %exitcond = icmp eq i32 %inc, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add6
}

; Function Attrs: nounwind uwtable
define i32 @bar() local_unnamed_addr #0 {
entry:
  %call = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 0))
  %call1 = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 8), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 8))
  %add2 = add i32 %call, %call1
  %call3 = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 16), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 16))
  %add4 = add i32 %add2, %call3
  %call5 = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 24), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 24))
  %add6 = add i32 %add4, %call5
  ret i32 %add6
}

; Function Attrs: nounwind uwtable
  define i32 @baz() local_unnamed_addr #0 {
  entry:
   %call = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 0))
   %call1 = call i32 @foo(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr1, i64 0, i64 8), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @arr2, i64 0, i64 8))
   %add = add i32 %call, %call1
   ret i32 %add
 }


attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0616611669a6ab12f0b530f7e813c5d69d6b2fe7) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d2002df0176c9bbbb45b23af2150c8946a14db46)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
