; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans

; This test is a regression test for CMPLRLLVM-9213 to verify that the compiler
; does not crash when attempting to partial inline devirtualized functions
; when profiling information is being used.

; RUN: opt < %s -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -passes=partial-inliner -S  | FileCheck %s

%"class.Base" = type { i32 (...)** }
%"class.Derived" = type { %class.Base }
%"class.Derived2" = type { %class.Base }

; Function that is a devirtualization target, and has profiling information.
define i32 @Func(%"class.Derived"* %this, i1 %cond, i32* %align.val) !prof !1 !_Intel.Devirt.Target !6 {
entry:
  br i1 %cond, label %if.then, label %return, !prof !4
if.then:
  store i32 10, i32* %align.val, align 4
  br label %return
return:
  ret i32 0
}

; Another devirtualized target
define i32 @Func2(%"class.Derived2"* %this, i1 %cond, i32* %align.val) !prof !2 !_Intel.Devirt.Target !6 {
  ret i32 1;
}

; In this function, verify that partial inlining occurs for the call to @Func.
define internal i32 @Caller(%"class.Base"* %b, i1 %cond, i32* %align.val) !prof !3 {
entry:
  %0 = bitcast %"class.Base"* %b to i32 (%"class.Base"*, i1, i32*)***
  %vtable = load i32 (%class.Base*, i1, i32*)**, i32 (%class.Base*, i1, i32*)*** %0
  %1 = load i32 (%class.Base*, i1, i32*)*, i32 (%class.Base*, i1, i32*)** %vtable
  %2 = bitcast i32 (%class.Base*, i1, i32*)* %1 to i8*
  %cmp = icmp eq i8* %2, bitcast (i32 (%"class.Derived"*, i1, i32*)* @Func to i8*)
  br i1 %cmp, label %isa_derived, label %isa_derived2, !prof !5
isa_derived:
  %obj_d = bitcast %"class.Base"* %b to %"class.Derived"*
  %val1 = call i32 @Func(%"class.Derived"* %obj_d, i1 %cond, i32* %align.val)
  br label %return

isa_derived2:
  %obj_d2 = bitcast %"class.Base"* %b to %"class.Derived2"*
  %val2 = call i32 @Func2(%"class.Derived2"* %obj_d2, i1 %cond, i32* %align.val)
  br label %return

return:
  %val = phi i32 [%val1, %isa_derived], [%val2, %isa_derived2]
  ret i32 %val
}
; CHECK-LABEL: @Caller
; CHECK: isa_derived:
; CHECK: br
; CHECK: call void @Func.1.

!1 = !{!"function_entry_count", i64 90}
!2 = !{!"function_entry_count", i64 10}
!3 = !{!"function_entry_count", i64 100}
!4 = !{!"branch_weights", i32 30, i32 60}
!5 = !{!"branch_weights", i32 90, i32 10}
!6 = !{!"_Intel.Devirt.Target"}

; end INTEL_FEATURE_SW_DTRANS
