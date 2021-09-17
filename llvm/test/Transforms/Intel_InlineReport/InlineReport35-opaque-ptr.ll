; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -inline -inline-report=7 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -passes='cgscc(inline)' -inline-report=7 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=134 < %s -S | opt -opaque-pointers -inline -inline-report=134 -dtrans-inline-heuristics -S | opt -opaque-pointers -inlinereportemitter -inline-report=134 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=134 < %s -S | opt -opaque-pointers -passes='cgscc(inline)' -inline-report=134 -dtrans-inline-heuristics -S | opt -opaque-pointers -passes='inlinereportemitter' -inline-report=134 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

; Check that @g is not inlined because it would introduce exception handling
; code into a loop in @main which does not already have exception handling.
; Check that @h is inlined because it does not have exception handling.

; CHECK-OLD: INLINE: h{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-OLD: -> g{{.*}}{{\[\[}}Callee has exception handling{{\]\]}}
; CHECK: call i32 @g
; CHECK-NOT: call i32 @h
; CHECK-NEW: INLINE: h{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-NEW: -> g{{.*}}{{\[\[}}Callee has exception handling{{\]\]}}

declare dso_local ptr @__cxa_begin_catch(ptr)

define linkonce_odr hidden void @__clang_call_terminate(ptr %arg) {
bb:
  %i = call ptr @__cxa_begin_catch(ptr %arg)
  unreachable
}

; Function Attrs: nounwind
define internal i32 @h(i32 %mine) #0 {
bb:
  ret i32 %mine
}

define internal i32 @g(i32 %mine) personality ptr @__gxx_personality_v0 {
bb:
  %inv = invoke i32 @h(i32 7)
          to label %ilabel1 unwind label %lpad

bb1:                                              ; No predecessors!
  ret i32 2

ilabel1:                                          ; preds = %bb
  ret i32 1

lpad:                                             ; preds = %bb
  %t29 = landingpad { ptr, i32 }
          catch ptr null
  %t30 = extractvalue { ptr, i32 } %t29, 0
  call void @__clang_call_terminate(ptr %t30)
  unreachable
}

declare dso_local i32 @__gxx_personality_v0(...)

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %s = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i)
  store i32 0, ptr %i, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %s)
  store i32 0, ptr %s, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i3 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %i3, 1500
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i4 = load i32, ptr %i, align 4
  %call = call i32 @g(i32 %i4)
  %i5 = load i32, ptr %s, align 4
  %add = add nsw i32 %i5, %call
  store i32 %add, ptr %s, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i6 = load i32, ptr %i, align 4
  %inc = add nsw i32 %i6, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %i7 = load i32, ptr %s, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %s)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i)
  ret i32 %i7
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

; end INTEL_FEATURE_SW_ADVANCED
