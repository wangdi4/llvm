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

%struct.MYEMPTY = type { }
%struct.MYSTRUCT = type { i32 (i32)*, i32 }
%struct.MYSTRUCTX = type { %struct.MYEMPTY*, i32 }

@myempty = internal dso_local global %struct.MYEMPTY zeroinitializer, align 8

@myarg = internal dso_local global %struct.MYSTRUCT { i32 (i32)* @target0, i32 5 }, align 8
@myargx = internal dso_local global %struct.MYSTRUCTX { %struct.MYEMPTY* @myempty, i32 10 }, align 8
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

define dso_local i32 @target2(%struct.MYSTRUCTX* %arg) {
  %myfp = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 0
  %t0 = load %struct.MYEMPTY*, %struct.MYEMPTY** %myfp, align 8
  %t2 = bitcast %struct.MYEMPTY* %t0 to i32 (i32)*
  %myint = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 1
  %t1 = load i32, i32* %myint, align 8
  %call = call i32 %t2(i32 %t1)
  ret i32 %call
}

define dso_local i32 @main() {
  %t0 = load i32 (%struct.MYSTRUCT*)*, i32 (%struct.MYSTRUCT*)** @fp, align 8
  %call = call i32 %t0(%struct.MYSTRUCT* @myarg)
  ret i32 %call
}
