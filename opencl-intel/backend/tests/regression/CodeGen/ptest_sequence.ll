; RUN: llc < %s -mtriple=i386-apple-darwin10 -mcpu=penryn | FileCheck %s

declare i32 @llvm.x86.sse41.ptestz(<4 x float> %p1, <4 x float> %p2) nounwind
declare i32 @llvm.x86.sse41.ptestc(<4 x float> %p1, <4 x float> %p2) nounwind

define <4 x float> @test1(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test1:
; CHECK: ptest
; CHECK-NEXT:	jne
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp ne i32 %res, 0 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test2(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test2:
; CHECK: ptest
; CHECK-NEXT:	je
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp eq i32 %res, 0 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test3(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test3:
; CHECK: ptest
; CHECK-NEXT:	jne
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %one = trunc i32 %res to i1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test4(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test4:
; CHECK: ptest
; CHECK-NEXT:	jae
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp ne i32 %res, 0 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test5(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test5:
; CHECK: ptest
; CHECK-NEXT:	jb
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp eq i32 %res, 0 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test6(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test6:
; CHECK: ptest
; CHECK-NEXT:	jae
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %a) nounwind 
  %one = trunc i32 %res to i1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test7(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test7:
; CHECK: ptest
; CHECK-NEXT:	jne
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp eq i32 %res, 1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test8(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test8:
; CHECK: ptest
; CHECK-NEXT:	je
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestz(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp ne i32 %res, 1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test9(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test9:
; CHECK: ptest
; CHECK-NEXT:	jae
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp eq i32 %res, 1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}

define <4 x float> @test10(<4 x float> %a, <4 x float> %b) nounwind {
entry:
; CHECK: test10:
; CHECK: ptest
; CHECK-NEXT:	jb
; CHECK: ret

  %res = call i32 @llvm.x86.sse41.ptestc(<4 x float> %a, <4 x float> %a) nounwind 
  %one = icmp ne i32 %res, 1 
  br i1 %one, label %bb1, label %bb2

bb1:
  %c = fadd <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
  br label %return

bb2:
	%d = fdiv <4 x float> %b, < float 1.000000e+002, float 2.000000e+002, float 3.000000e+002, float 4.000000e+002 >
	br label %return

return:
  %e = phi <4 x float> [%c, %bb1], [%d, %bb2]
  ret <4 x float> %e
}
