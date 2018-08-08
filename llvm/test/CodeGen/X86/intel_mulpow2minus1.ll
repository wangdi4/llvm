; RUN: llc < %s | FileCheck %s
	
define i32 @test(i32 %a) {
entry:
	%tmp3 = mul i32 %a, 31
; 	CHECK-LABEL: test:
;	CHECK-NOT: imul 
;   CHECK: shll 
;   CHECK-NEXT: sub
	ret i32 %tmp3
}

define i32 @test1(i32 %a) {
entry:
	%tmp3 = mul i32 %a, -31
; 	CHECK-LABEL: test1:
;	CHECK-NOT: imul 
;   CHECK: shll 
;   CHECK-NEXT: sub
;   CHECK-NEXT: movl
;   CHECK-NEXT: ret
	ret i32 %tmp3
}


define i32 @test2(i32 %a) {
entry:
	%tmp3 = mul i32 %a, 33
; 	CHECK-LABEL: test2:
;	CHECK-NOT: imul 
;   CHECK: shll 
;   CHECK-NEXT: leal
	ret i32 %tmp3
}

define i32 @test3(i32 %a) {
entry:
	%tmp3 = mul i32 %a, -33
; 	CHECK-LABEL: test3:
;	CHECK-NOT: imul 
;   CHECK: shll 
;   CHECK-NEXT: leal
;   CHECK-NEXT: negl
	ret i32 %tmp3
}

define i64 @test4(i64 %a) {
entry:
	%tmp3 = mul i64 %a, 31
; 	CHECK-LABEL: test4:
;	CHECK-NOT: imul 
;   CHECK: shlq 
;   CHECK-NEXT: sub
	ret i64 %tmp3
}

define i64 @test5(i64 %a) {
entry:
	%tmp3 = mul i64 %a, -31
; 	CHECK-LABEL: test5:
;	CHECK-NOT: imul 
;   CHECK: shlq 
;   CHECK-NEXT: sub
;   CHECK-NEXT: movq
;   CHECK-NEXT: ret
	ret i64 %tmp3
}


define i64 @test6(i64 %a) {
entry:
	%tmp3 = mul i64 %a, 33
; 	CHECK-LABEL: test6:
;	CHECK-NOT: imul 
;   CHECK: shlq 
;   CHECK-NEXT: leaq
	ret i64 %tmp3
}

define i64 @test7(i64 %a) {
entry:
	%tmp3 = mul i64 %a, -33
; 	CHECK-LABEL: test7:
;	CHECK-NOT: imul 
;   CHECK: shlq 
;   CHECK-NEXT: leaq
;   CHECK-NEXT: negq
	ret i64 %tmp3
}
