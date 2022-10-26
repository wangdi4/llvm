; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; INTEL CUSTOMIZATION:

; RUN: opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-for-fusion-heuristics=true -inline-threshold=20 -inline-for-fusion-min-arg-refs=3 -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK-EARLY,CHECK-NEW %s
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-for-fusion-heuristics=true -inline-threshold=20 -inline-for-fusion-min-arg-refs=3 -inline-report=0xe886 | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefixes=CHECK-OLD,CHECK-LATE %s

; Test checks that inlining happens for all foo() call sites. The inlining is
; supposed to be followed by loop fusion and vectorization.

; Check that the IR has calls only in @baz when we are done. All calls to
; @foo in @bar will be inlined out. (In the EARLY case, the IR is dumped
; BEFORE the inlining report.)

; CHECK-EARLY: define{{.*}}@foo
; CHECK-EARLY-NOT: call
; CHECK-EARLY: define{{.*}}@bar
; CHECK-EARLY-NOT: call
; CHECK-EARLY: define{{.*}}@baz
; CHECK-EARLY: call i32 @foo
; CHECK-EARLY: call i32 @foo

; Check for old pass manager with old inline report and new pass manager with
; old and metadata inline report

; CHECK-OLD: COMPILE FUNC: bar
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; CHECK-OLD: COMPILE FUNC: baz
; CHECK-OLD-NOT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-OLD-NOT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; Check for new pass manager with old inline report

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-NEW: COMPILE FUNC: bar
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NEXT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; CHECK-NEW: COMPILE FUNC: baz
; CHECK-NEW-NOT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused
; CHECK-NEW-NOT: INLINE{{.*}}foo{{.*}}Callee has multiple callsites with loops that could be fused

; Check that the IR has calls only in @baz when we are done. All calls to
; @foo in @bar will be inlined out. (In the LATE case, the IR is dumped
; AFTER the inlining report.)

; CHECK-LATE: define{{.*}}@foo
; CHECK-LATE-NOT: call
; CHECK-LATE: define{{.*}}@bar
; CHECK-LATE-NOT: call
; CHECK-LATE: define{{.*}}@baz
; CHECK-LATE: call i32 @foo
; CHECK-LATE: call i32 @foo

target triple = "x86_64-unknown-linux-gnu"

@arr1 = common global [100 x i32] zeroinitializer, align 16
@arr2 = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %ptr1, i32* %ptr2) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %ptr1.addr = phi i32* [ %ptr1, %entry ], [ %add.ptr, %for.body ]
  %ptr2.addr = phi i32* [ %ptr2, %entry ], [ %add.ptr7, %for.body ]
  %sum = phi i32 [ 0, %entry ], [ %add6, %for.body ]
  %i = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load i32, i32* %ptr1.addr, align 4, !tbaa !2
  %1 = load i32, i32* %ptr2.addr, align 4, !tbaa !2
  %sub = sub nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %ptr1.addr, i64 1
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, i32* %ptr2.addr, i64 1
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %sub4 = sub nsw i32 %2, %3
  %shl = shl i32 %sub, 1
  %shl5 = shl i32 %sub4, 2
  %add = add i32 %shl, %sum
  %add6 = add i32 %add, %shl5
  %inc = add nuw nsw i32 %i, 1
  %add.ptr = getelementptr inbounds i32, i32* %ptr1.addr, i64 2
  %add.ptr7 = getelementptr inbounds i32, i32* %ptr2.addr, i64 2
  %exitcond = icmp eq i32 %inc, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add6
}

; Function Attrs: nounwind uwtable
define i32 @bar() local_unnamed_addr #0 {
entry:
  %gep_arr1_1 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 0
  %gep_arr2_1 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 0
  %call1 = call i32 @foo(i32* %gep_arr1_1, i32* %gep_arr2_1)
  %gep_arr1_2 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 8
  %gep_arr2_2 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 8
  %call2 = call i32 @foo(i32* %gep_arr1_2, i32* %gep_arr2_2)
  %add1 = add i32 %call1, %call2
  %mul1 = mul i32 %add1, 4
  %sub1 = sub i32 %mul1, 10
  %gep_arr1_3 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 16
  %gep_arr2_3 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 16
  %call3 = call i32 @foo(i32* %gep_arr1_3, i32* %gep_arr2_3)
  %add2 = add i32 %mul1, %call3
  %mul2 = mul i32 %add2, 4
  %sub2 = sub i32 %mul2, 10
  %gep_arr1_4 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 24
  %gep_arr2_4 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 24
  %call4 = call i32 @foo(i32* %gep_arr1_4, i32* %gep_arr2_4)
  %add3 = add i32 %sub2, %call4
  %mul3 = mul i32 %add3, 4
  %sub3 = sub i32 %mul3, 10
  %gep_arr1_5 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 0
  %gep_arr2_5 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 0
  %call5 = call i32 @foo(i32* %gep_arr1_5, i32* %gep_arr2_5)
  %add4 = add i32 %sub3, %call5
  %mul4 = mul i32 %add4, 4
  %sub4 = sub i32 %mul4, 10
  %gep_arr1_6 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 8
  %gep_arr2_6 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 8
  %call6 = call i32 @foo(i32* %gep_arr1_6, i32* %gep_arr2_6)
  %add5 = add i32 %sub4, %call6
  %mul5 = mul i32 %add5, 4
  %sub5 = sub i32 %mul5, 10
  %gep_arr1_7 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 16
  %gep_arr2_7 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 16
  %call7 = call i32 @foo(i32* %gep_arr1_7, i32* %gep_arr2_7)
  %add6 = add i32 %sub5, %call7
  %mul6 = mul i32 %add6, 4
  %sub6 = sub i32 %mul6, 10
  %gep_arr1_8 = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 24
  %gep_arr2_8 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 24
  %call8 = call i32 @foo(i32* %gep_arr1_8, i32* %gep_arr2_8)
  %add7 = add i32 %sub6, %call8
  %mul7 = mul i32 %add7, 4
  %sub7 = sub i32 %mul7, 10
  ret i32 %mul7
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
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
; end INTEL_FEATURE_SW_ADVANCED
