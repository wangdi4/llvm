; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two structs with different size floating-point fields.
; Check that AddressTaken is set on MYSTRUCT, as MYSTRUCTX is a compatible
; type, so @target1 and @target2 are targets for @fp.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { float, double }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { float, double }
%struct.MYSTRUCTX = type { double, float }

@myarg = internal dso_local global %struct.MYSTRUCT zeroinitializer, align 4
@fp = internal dso_local global float (%struct.MYSTRUCT*)* null, align 8

define dso_local float @target1(%struct.MYSTRUCT* %arg) {
  %my1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %arg, i32 0, i32 0
  %t1 = load float, float* %my1, align 8
  ret float %t1
}

define dso_local double @target2(%struct.MYSTRUCTX* %arg) {
entry:
  %my1 = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 0
  %t1 = load double, double* %my1, align 8
  ret double %t1
}

define dso_local i32 @main() #0 {
  %t0 = load float (%struct.MYSTRUCT*)*, float (%struct.MYSTRUCT*)** @fp, align 8
  %call = call float %t0(%struct.MYSTRUCT* @myarg)
  %conv = fptosi float %call to i32
  ret i32 %conv
}
