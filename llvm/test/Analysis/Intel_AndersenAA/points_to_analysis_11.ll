; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s
; CHECK: foo:%2    --> same as (8) foo:%1
; Tests bitcast to vectortype is handled by Andersens analysis

@flags1 = external global [30001 x double], align 16
@ptr1 = global ptr getelementptr inbounds ([30001 x double], ptr @flags1, i64 0, i64 0), align 8

; Function Attrs: nounwind uwtable
define void @foo() #0 {
  %1 = load ptr, ptr @ptr1, align 8
  %2 = getelementptr inbounds double, ptr %1, i64 0
  %wide.load21 = load <2 x double>, ptr %2, align 8
  ret void
}
