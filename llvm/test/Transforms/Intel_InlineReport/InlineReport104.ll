; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes='cgscc(inline),module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline),module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that the inlining report includes the results of cloning
; after inlining. Also check that it does NOT include 'Newly created callsite',
; 'Not tested for inlining', or 'DELETE' messages.

; This test is similar to InlineReport86.ll, but checks that the functions
; cloned also get callsite information in the inlining report.

; CHECK-CL: DEAD STATIC FUNC: mynothing
; CHECK-CL: COMPILE FUNC: myadd
; CHECK-CL: INLINE: mynothing{{.*}}Callee is single basic block
; CHECK-CL: COMPILE FUNC: mysub
; CHECK-CL: INLINE: mynothing{{.*}}Callee has single callsite and local linkage
; CHECK: COMPILE FUNC: main
; CHECK: myadd.1{{.*}}Callee has noinline attribute
; CHECK: myadd.2{{.*}}Callee has noinline attribute
; CHECK: mysub.3{{.*}}Callee has noinline attribute
; CHECK: mysub.3{{.*}}Callee has noinline attribute
; CHECK: mysub.4{{.*}}Callee has noinline attribute
; CHECK-NOT: Newly created callsite
; CHECK-NOT: Not tested for inlining
; CHECK-NOT: DELETE
; CHECK-MD: DEAD STATIC FUNC: mynothing
; CHECK-MD: COMPILE FUNC: myadd
; CHECK-MD: INLINE: mynothing{{.*}}Callee is single basic block
; CHECK-MD: COMPILE FUNC: mysub
; CHECK-MD: INLINE: mynothing{{.*}}Callee has single callsite and local linkage
; CHECK: COMPILE FUNC: myadd.1
; CHECK: INLINE: mynothing{{.*}}Callee is single basic block
; CHECK: COMPILE FUNC: myadd.2
; CHECK: INLINE: mynothing{{.*}}Callee is single basic block
; CHECK: COMPILE FUNC: mysub.3
; CHECK: INLINE: mynothing{{.*}}Callee has single callsite and local linkage
; CHECK: COMPILE FUNC: mysub.4
; CHECK: INLINE: mynothing{{.*}}Callee has single callsite and local linkage

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = tail call i32 @myadd(i32 1, i32 2)
  %call1 = tail call i32 @myadd(i32 1, i32 3)
  %add1 = add nsw i32 %call1, %call
  %call2 = tail call i32 @mysub(i32 4, i32 5)
  %call3 = tail call i32 @mysub(i32 4, i32 5)
  %add2 = add nsw i32 %call1, %call3
  %add3 = add i32 %add1, %add2
  %call4 = tail call i32 @mysub(i32 6, i32 7)
  %add4 = add i32 %add3, %call4
  ret i32 %add4
}

define internal i32 @mynothing(i32 %a) {
entry:
  ret i32 %a
}

define internal i32 @myadd(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %call = tail call i32 @mynothing(i32 %a)
  %add = add nsw i32 %b, %call
  ret i32 %add
}

define internal i32 @mysub(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %call = tail call i32 @mynothing(i32 %b)
  %sub = sub nsw i32 %a, %call
  ret i32 %sub
}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
