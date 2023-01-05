; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -S -passes='cgscc(inline),instcombine,module(post-inline-ip-cloning)' -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=1 -ip-gen-cloning-min-switch-count=0 -inline-report=0xe807 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline),instcombine,module(post-inline-ip-cloning)' -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=1 -ip-gen-cloning-min-switch-count=0 -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; CMPLRLLVM-29570: Fix core dump with
; tc -r none -t cpu2017ref/527 -l opt_base6_core_avx512 -c " -mllvm -stats -mllvm -inline-report=3" --ignore_rules

; Check that the inlining report does not die when inlining is followed
; by instcombine (which deletes @llvm.experimental.noalias.scope.decl
; intrinsics), followed by cloning.

; CHECK-CL: define {{.*}} @callee(i64* %x)
; CHECK-CL: define internal i32 @caller(i32 %arg)
; CHECK-CL: define dso_local i32 @main()
; CHECK-CL: call i32 @caller.[[CLI0:[0-9]]](i32 5)
; CHECK-CL: call i32 @caller.[[CLI1:[0-9]]](i32 4)
; CHECK-CL: define internal i32 @caller.[[CLI1]](i32 %arg)
; CHECK-CL: define internal i32 @caller.[[CLI0]](i32 %arg)

; CHECK: COMPILE FUNC: callee

; CHECK: COMPILE FUNC: caller
; CHECK: DELETE: llvm.experimental.noalias.scope.decl
; CHECK: DELETE: llvm.experimental.noalias.scope.decl
; CHECK: EXTERN: opaque_callee
; CHECK: INLINE: callee

; CHECK: COMPILE FUNC: main
; CHECK: caller.[[I0:[0-9]]]{{.*}}Callee has noinline attribute
; CHECK: caller.[[I1:[0-9]]]{{.*}}Callee has noinline attribute

; CHECK-CL: COMPILE FUNC: caller.[[I1]]
; CHECK-CL: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-CL: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-CL: EXTERN: opaque_callee
; CHECK-CL: INLINE: callee

; CHECK-CL: COMPILE FUNC: caller.[[I0]]
; CHECK-CL: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-CL: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-CL: EXTERN: opaque_callee
; CHECK-CL: INLINE: callee

; CHECK-MD: COMPILE FUNC: caller.[[I1]]
; CHECK-MD: EXTERN: opaque_callee
; CHECK-MD: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-MD: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-MD: INLINE: callee

; CHECK-MD: COMPILE FUNC: caller.[[I0]]
; CHECK-MD: EXTERN: opaque_callee
; CHECK-MD: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-MD: DELETE: llvm.experimental.noalias.scope.decl
; CHECK-MD: INLINE: callee

; CHECK-MD: define {{.*}} @callee(i64* %x)
; CHECK-MD: define internal i32 @caller(i32 %arg)
; CHECK-MD: define dso_local i32 @main()
; CHECK-MD: call i32 @caller.[[MDI0:[0-9]]](i32 5)
; CHECK-MD: call i32 @caller.[[MDI1:[0-9]]](i32 4)
; CHECK-MD: define internal i32 @caller.[[MDI1]](i32 %arg)
; CHECK-MD: define internal i32 @caller.[[MDI0]](i32 %arg)

declare { i64* } @opaque_callee()

define { i64* } @callee(i64* %x) {
  %res = insertvalue { i64* } undef, i64* %x, 0
  ret { i64* } %res
}

; @opaque_callee() should not receive noalias metadata here.
define internal i32 @caller(i32 %arg) #0 {
  %cmp = icmp sgt i32 %arg, 5
  br i1 %cmp, label %if.then, label %if.end
if.then:
  call void @llvm.experimental.noalias.scope.decl(metadata !0)
  call void @llvm.experimental.noalias.scope.decl(metadata !0)
  %s = call { i64* } @opaque_callee()
  %x = extractvalue { i64* } %s, 0
  call { i64* } @callee(i64* %x), !noalias !0
  ret i32 1
if.end:
  ret i32 0
}

define dso_local i32 @main() {
entry:
  %call = call i32 @caller(i32 5)
  %call1 = call i32 @caller(i32 4)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

declare void @llvm.experimental.noalias.scope.decl(metadata)

!0 = !{!1}
!1 = !{!1, !2, !"scope"}
!2 = !{!2, !"domain"}

attributes #0 = { noinline }
; end INTEL_FEATURE_SW_ADVANCED
