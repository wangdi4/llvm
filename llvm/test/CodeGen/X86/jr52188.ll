; RUN: llc < %s -intel-libirc-allowed -mtriple=x86_64-pc-linux-gnu

; Make sure we are not crashing on this test.

define i32 @test_float(ptr %foo, ptr %t1) #0 {
entry:
  %call = call i32 %foo(ptr %t1) #0
  ret i32 %call
}

attributes #0 = { "approx-func-fp-math"="true" }