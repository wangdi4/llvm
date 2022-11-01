; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s
; CHECK: foo:%3    --> same as (8) foo:%1
; Tests bitcast to vectortype is handled by Andersens analysis

@flags1 = external global [30001 x double], align 16
@ptr1 = global double* getelementptr inbounds ([30001 x double], [30001 x double]* @flags1, i64 0, i64 0), align 8

; Function Attrs: nounwind uwtable
define void @foo() #0 {
  %1 = load double*, double** @ptr1, align 8
  %2 = getelementptr inbounds double, double* %1, i64 0
  %3 = bitcast double* %2 to <2 x double>*
  %wide.load21 = load <2 x double>, <2 x double>* %3, align 8
  ret void
}

