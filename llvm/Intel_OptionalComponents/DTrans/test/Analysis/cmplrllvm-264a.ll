; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -dtrans-usecrulecompat -disable-output -debug-only=dtrans-crc < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-usecrulecompat -disable-output -debug-only=dtrans-crc < %s 2>&1 | FileCheck %s

; Check that %struct.S1 and %struct.S2 are recognized as
; compatible using the c-rule.

; CHECK: dtrans-crc: YES %struct.S1* %struct.S2*

%struct.S1 = type { %struct.T1*, %struct.T2*, %struct.T1* }
%struct.T2 = type { i32, i32 }
%struct.T1 = type { i32, i32 }
%struct.S2 = type { %struct.T1*, %struct.T2*, %struct.T1* }

@myarg = dso_local global %struct.S1 zeroinitializer, align 8
@myfp = dso_local global i32 (%struct.S1*)* null, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @target1(%struct.S1* noundef %arg) {
entry:
  %arg.addr = alloca %struct.S1*, align 8
  store %struct.S1* %arg, %struct.S1** %arg.addr, align 8
  %0 = load %struct.S1*, %struct.S1** %arg.addr, align 8
  %field0 = getelementptr inbounds %struct.S1, %struct.S1* %0, i32 0, i32 0
  %1 = load %struct.T1*, %struct.T1** %field0, align 8
  %cmp = icmp ne %struct.T1* %1, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind uwtable
define dso_local i32 @target2(%struct.S2* noundef %arg) {
entry:
  %arg.addr = alloca %struct.S2*, align 8
  store %struct.S2* %arg, %struct.S2** %arg.addr, align 8
  %0 = load %struct.S2*, %struct.S2** %arg.addr, align 8
  %field1 = getelementptr inbounds %struct.S2, %struct.S2* %0, i32 0, i32 1
  %1 = load %struct.T2*, %struct.T2** %field1, align 8
  %cmp = icmp ne %struct.T2* %1, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i32 (%struct.S1*)*, i32 (%struct.S1*)** @myfp, align 8
  %call = call i32 %0(%struct.S1* noundef @myarg)
  ret i32 %call
}

