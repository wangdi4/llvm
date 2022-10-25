; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-OLD

; Check that @g is not inlined because it would introduce exception handling
; code into a loop in @main which does not already have exception handling.
; Check that @h is inlined because it does not have exception handling.

; CHECK-OLD: INLINE: h{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-OLD: -> g{{.*}}{{\[\[}}Callee has exception handling{{\]\]}}
; CHECK: call i32 @g
; CHECK-NOT: call i32 @h
; CHECK-NEW: INLINE: h{{.*}}<<Callee has single callsite and local linkage>>
; CHECK-NEW: -> g{{.*}}{{\[\[}}Callee has exception handling{{\]\]}}

target triple = "x86_64-unknown-linux-gnu"

declare dso_local ptr @__cxa_begin_catch(ptr)

define linkonce_odr hidden void @__clang_call_terminate(ptr) {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #4
  unreachable
}

define internal i32 @h(i32 %mine) nounwind {
  ret i32 %mine
}

define internal i32 @g(i32 %mine) personality ptr @__gxx_personality_v0 {
  %inv = invoke i32 @h(i32 7) to label %ilabel1 unwind label %lpad
  ret i32 2

ilabel1:
  ret i32 1

lpad:
  %t29 = landingpad { ptr, i32 }
          catch ptr null
  %t30 = extractvalue { ptr, i32 } %t29, 0
  call void @__clang_call_terminate(ptr %t30)
  unreachable
}

declare dso_local i32 @__gxx_personality_v0(...)

declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture) #2

declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture) #2

define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %s = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %i) #2
  store i32 0, ptr %i, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, ptr %s) #2
  store i32 0, ptr %s, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %t2 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %t2, 1500
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %t3 = load i32, ptr %i, align 4
  %call = call i32 @g(i32 %t3)
  %t4 = load i32, ptr %s, align 4
  %add = add nsw i32 %t4, %call
  store i32 %add, ptr %s, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %t5 = load i32, ptr %i, align 4
  %inc = add nsw i32 %t5, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %t6 = load i32, ptr %s, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %s) #2
  call void @llvm.lifetime.end.p0i8(i64 4, ptr %i) #2
  ret i32 %t6
}


; end INTEL_FEATURE_SW_ADVANCED
