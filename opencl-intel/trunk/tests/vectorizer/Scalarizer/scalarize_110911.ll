; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: func
;CHECK-NOT: call{{.*}}undef
;CHECK: ret

define void @func(i64* %A, <4 x i32> %C) nounwind {
entry:
  %x = call <4 x i32> @foo1(<4 x i32> %C, i64* %A) 
  br label %basic2
 
basic1:
  call void @boo1(<4 x i32> %add_val, i64* %A)
  br label %end
  
basic2:
  %add_val = add <4 x i32>	%x, <i32 0, i32 2, i32 4, i32 0>
  br label %basic1
  
end:
  ret void
}

declare <4 x i32> @foo1(<4 x i32> %x, i64* %A) nounwind 
declare void @boo1(<4 x i32> %x, i64* %A) nounwind 
