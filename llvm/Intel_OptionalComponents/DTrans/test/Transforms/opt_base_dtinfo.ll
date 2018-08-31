; This test is run without forcing whole-program to be on to verify
; the DTrans optimization base class does not perform type transformation
; unless the dtrans analysis has analyzed the module. This is necessary
; because the base class relies on information about the types collected
; during analysis.

; RUN: opt < %s -S -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.test01 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.test01 | FileCheck %s

; CHECK-NOT: %__DTT_struct.test01 = type { i32, i32, i32 }

%struct.test01 = type { i32, i32, i32 }
%struct.test01dep = type { i32, %struct.test01* }

define void @test01() {
  %local1 = alloca %struct.test01
  %localdep = alloca %struct.test01dep

  ; The following will trigger a verification error if this test fails because
  ; the dependent type will not be changed.
  %faddr = getelementptr %struct.test01dep, %struct.test01dep* %localdep, i64 0, i32 1
  store %struct.test01* %local1, %struct.test01** %faddr

  ret void;
}
