; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans

; This test is a regression test for CMPLRLLVM-9213 to verify that the compiler
; does not crash when attempting to partial inline devirtualized functions
; when profiling information is being used.

; RUN: opt -opaque-pointers < %s -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -passes=partial-inliner -S  | FileCheck %s

%"class.Base" = type { ptr }
%"class.Derived" = type { %class.Base }
%"class.Derived2" = type { %class.Base }

define i32 @Func(ptr %this, i1 %cond, ptr %align.val) !prof !0 !_Intel.Devirt.Target !1 {
entry:
  br i1 %cond, label %if.then, label %return, !prof !2

if.then:                                          ; preds = %entry
  store i32 10, ptr %align.val, align 4
  br label %return

return:                                           ; preds = %if.then, %entry
  ret i32 0
}

define i32 @Func2(ptr %this, i1 %cond, ptr %align.val) !prof !3 !_Intel.Devirt.Target !1 {
bb:
  ret i32 1
}

define internal i32 @Caller(ptr %b, i1 %cond, ptr %align.val) !prof !4 {
entry:
  %vtable = load ptr, ptr %b, align 8
  %i1 = load ptr, ptr %vtable, align 8
  %cmp = icmp eq ptr %i1, @Func
  br i1 %cmp, label %isa_derived, label %isa_derived2, !prof !5

isa_derived:                                      ; preds = %entry
  %val1 = call i32 @Func(ptr %b, i1 %cond, ptr %align.val)
  br label %return

isa_derived2:                                     ; preds = %entry
  %val2 = call i32 @Func2(ptr %b, i1 %cond, ptr %align.val)
  br label %return

return:                                           ; preds = %isa_derived2, %isa_derived
  %val = phi i32 [ %val1, %isa_derived ], [ %val2, %isa_derived2 ]
  ret i32 %val
}

; CHECK-LABEL: @Caller
; CHECK: isa_derived:
; CHECK: br
; CHECK: call void @Func.1.

!0 = !{!"function_entry_count", i64 90}
!1 = !{!"_Intel.Devirt.Target"}
!2 = !{!"branch_weights", i32 30, i32 60}
!3 = !{!"function_entry_count", i64 10}
!4 = !{!"function_entry_count", i64 100}
!5 = !{!"branch_weights", i32 90, i32 10}
; end INTEL_FEATURE_SW_DTRANS
