; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck  --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD

; Check that inlining is inhibited for @mysavestackgrow, because it does
; special key stack computations. Here, @myalloc explicitly calls @realloc.

target triple = "x86_64-unknown-linux-gnu"

%union.any = type { i8* }

@mysavestack = external dso_local global %union.any*, align 8
@mysavestack_max = external dso_local global i32, align 4

declare noalias i8* @realloc(i8* allocptr, i64) #0

define internal noalias i8* @myalloc(i8*, i64) {
  %t0 = tail call i8* @realloc(i8* nonnull %0, i64 %1)
  ret i8* %t0
}

define dso_local void @mysavestackgrow() {
entry:
  %0 = load i32, i32* @mysavestack_max, align 4
  %mul = mul nsw i32 %0, 3
  %div = sdiv i32 %mul, 2
  %add = add nsw i32 %div, 4
  store i32 %add, i32* @mysavestack_max, align 4
  %1 = load i8*, i8** bitcast (%union.any** @mysavestack to i8**), align 8
  %conv = sext i32 %add to i64
  %mul1 = shl nsw i64 %conv, 3
  %call = call i8* @myalloc(i8* %1, i64 %mul1)
  %2 = bitcast i8* %call to %union.any*
  store %union.any* %2, %union.any** @mysavestack, align 8
  ret void
}

define dso_local i32 @main() {
  tail call void @mysavestackgrow()
  ret i32 0
}

attributes #0 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("realloc") allocsize(1) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

; CHECK-OLD: COMPILE FUNC: mysavestackgrow
; CHECK-OLD: COMPILE FUNC: main
; CHECK-OLD: mysavestackgrow{{.*}}Callsite has key stack computations
; CHECK-OLD: {{.*}}call void @mysavestackgrow

; CHECK-NEW: {{.*}}call void @mysavestackgrow
; CHECK-NEW: COMPILE FUNC: mysavestackgrow
; CHECK-NEW: COMPILE FUNC: main
; CHECK-NEW: mysavestackgrow{{.*}}Callsite has key stack computations
; end INTEL_FEATURE_SW_ADVANCED
