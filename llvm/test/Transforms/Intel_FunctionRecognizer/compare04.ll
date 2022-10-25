; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; RUN: opt < %s -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Check that @arc_compare is not recognized as a qsort compare, because it
; does not always return the correct return value.

; CHECK-NOT: FUNCTION-RECOGNIZER: FOUND QSORT-COMPARE arc_compare
; CHECK: define{{.*}}@arc_compare
; CHECK-NOT: attributes #0 = { "is-qsort-compare" }

%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }
%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }

; Function Attrs: nounwind uwtable
define internal i32 @arc_compare(%struct.arc** %a1, %struct.arc** %a2) #0 {
entry:
  %0 = load %struct.arc*, %struct.arc** %a1, align 8
  %flow = getelementptr inbounds %struct.arc, %struct.arc* %0, i32 0, i32 7
  %1 = load i64, i64* %flow, align 8
  %2 = load %struct.arc*, %struct.arc** %a2, align 8
  %flow1 = getelementptr inbounds %struct.arc, %struct.arc* %2, i32 0, i32 7
  %3 = load i64, i64* %flow1, align 8
  %cmp = icmp sgt i64 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %4 = load %struct.arc*, %struct.arc** %a1, align 8
  %flow2 = getelementptr inbounds %struct.arc, %struct.arc* %4, i32 0, i32 7
  %5 = load i64, i64* %flow2, align 8
  %6 = load %struct.arc*, %struct.arc** %a2, align 8
  %flow3 = getelementptr inbounds %struct.arc, %struct.arc* %6, i32 0, i32 7
  %7 = load i64, i64* %flow3, align 8
  %cmp4 = icmp slt i64 %5, %7
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.end
  br label %return

if.end6:                                          ; preds = %if.end
  %8 = load %struct.arc*, %struct.arc** %a1, align 8
  %id = getelementptr inbounds %struct.arc, %struct.arc* %8, i32 0, i32 0
  %9 = load i32, i32* %id, align 8
  %10 = load %struct.arc*, %struct.arc** %a2, align 8
  %id7 = getelementptr inbounds %struct.arc, %struct.arc* %10, i32 0, i32 0
  %11 = load i32, i32* %id7, align 8
  %cmp8 = icmp slt i32 %9, %11
  br i1 %cmp8, label %if.then9, label %if.end10

if.then9:                                         ; preds = %if.end6
  br label %return

if.end10:                                         ; preds = %if.end6
  br label %return

return:                                           ; preds = %if.end, %if.then9, %if.then5, %if.then
  %retval.0 = phi i32 [ -1, %if.then ], [ 1, %if.then5 ], [ -1, %if.then9 ], [ 1, %if.end10 ]
  ret i32 %retval.0
}
; end INTEL_FEATURE_SW_ADVANCED
