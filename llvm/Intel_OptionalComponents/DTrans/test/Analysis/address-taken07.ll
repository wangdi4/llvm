; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two structs with function pointers of different types.
; Right now, these will be compatible, but we could refine the analysis
; so that they are not.
; Check that AddressTaken is set on MYSTRUCT, as MYSTRUCTX is a compatible
; type, so @target1 and @target2 are targets for @fp.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32 (i32)*, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32 (i32)*, i32 }
%struct.MYSTRUCTX = type { i64 (i32)*, i32 }

@myarg = internal dso_local global %struct.MYSTRUCT { i32 (i32)* @target0, i32 5 }, align 8
@myargx = internal dso_local global %struct.MYSTRUCTX { i64 (i32)* @target00, i32 10 }, align 8
@fp = internal dso_local global i32 (%struct.MYSTRUCT*)* null, align 8

define dso_local i32 @target0(i32 %myx) {
  %mul = mul nsw i32 2, %myx
  ret i32 %mul
}

define dso_local i64 @target00(i32 %myx) {
  %mul = mul nsw i32 2, %myx
  %conv = sext i32 %mul to i64
  ret i64 %conv
}

define dso_local i32 @target1(%struct.MYSTRUCT* %arg) {
  %myfp = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %arg, i32 0, i32 0
  %t0 = load i32 (i32)*, i32 (i32)** %myfp
  %myint = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %arg, i32 0, i32 1
  %t1 = load i32, i32* %myint, align 8
  %call = call i32 %t0(i32 %t1)
  ret i32 %call
}

define dso_local i64 @target2(%struct.MYSTRUCTX* %arg) #0 {
  %myfp = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 0
  %t0 = load i64 (i32)*, i64 (i32)** %myfp, align 8
  %myint = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 1
  %t1 = load i32, i32* %myint, align 8
  %call = call i64 %t0(i32 %t1)
  ret i64 %call
}

define dso_local i32 @main() {
  %t0 = load i32 (%struct.MYSTRUCT*)*, i32 (%struct.MYSTRUCT*)** @fp, align 8
  %call = call i32 %t0(%struct.MYSTRUCT* @myarg)
  ret i32 %call
}
