; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check a structure assigned function pointers

%struct.MYSTRUCT = type { i32 ()*, i32 ()* }

@coxglobalstruct = internal dso_local global %struct.MYSTRUCT { i32 ()* @myfoo, i32 ()* @mybar }, align 8

define internal i32 @myfoo() {
entry:
  ret i32 3
}

define internal i32 @mybar() {
entry:
  ret i32 4
}

define i32 @main() {
entry:
  store i32 ()* @myfoo, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 0), align 8
  store i32 ()* @mybar, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 1), align 8
  %0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 0), align 8
  %call = call i32 %0()
  %1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 1), align 8
  %call1 = call i32 %1()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32 ()*, i32 ()* }
; CHECK: Field LLVM Type: i32 ()*
; CHECK: Single Value: i32 ()* @myfoo
; CHECK: Field LLVM Type: i32 ()*
; CHECK: Single Value: i32 ()* @mybar

