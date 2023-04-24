; RUN: opt -S -passes=simplifycfg < %s | FileCheck %s

; Verify that 'or' condition is not converted to switch when "pre_loopopt"
; attribute is present on the function but it is done otherwise.

declare void @foo1()

declare void @foo2()

define void @test_pre_loopopt(i32 %V) "pre_loopopt" {
; CHECK-LABEL: test_pre_loopopt
; CHECK-NOT: switch
entry:
  %C1 = icmp eq i32 %V, 4
  %C2 = icmp eq i32 %V, 17
  %CN = or i1 %C1, %C2
  br i1 %CN, label %T, label %F
T:              
  call void @foo1( )
  ret void
F:              
  call void @foo2( )
  ret void
}

define void @test_no_loopopt(i32 %V) {
; CHECK-LABEL: test_no_loopopt
; CHECK: switch
entry:
  %C1 = icmp eq i32 %V, 4
  %C2 = icmp eq i32 %V, 17
  %CN = or i1 %C1, %C2
  br i1 %CN, label %T, label %F
T:              
  call void @foo1( )
  ret void
F:              
  call void @foo2( )
  ret void
}

