; RUN: llvm-as %s -o %t.bc
; RUN: opt  -instcombine %t.bc -S -o %t1.ll
; this is the test which reproduces ticket CSSD100005302 - [LLVM Community] Instcombin pass patch for bitcasts needs to be commited back to the LLVM community
; before the fix LLVM crashed, so this test just expects the test not to crash

; CHECK: ret

define i48 @test(<3 x i1> %icmp1, <3 x i1> %icmp2, i16 addrspace(1)* %dst) nounwind {
entry:
  %select1 = select <3 x i1> %icmp1, <3 x i16> zeroinitializer, <3 x i16> <i16 -1, i16 -1, i16 -1> 
  %select2 = select <3 x i1> %icmp2, <3 x i16> zeroinitializer, <3 x i16> %select1
  %tmp2 = bitcast <3 x i16> %select2 to i48
  ret i48 %tmp2
}

