; RUN: opt < %s -dtransanalysis -dtrans-print-allocations -disable-output 2>&1 | FileCheck %s

; struct S1 {
;   int  a;
;   int  b;
; };
%struct.S1 = type { i32, i32 }

; struct S2 {
;   int        a[32];
;   struct S1  s1;
; };
%struct.S2 = type { [32 x i32], %struct.S1 }


define void @test1() {
  ; s1 = (struct S1*)malloc(sizeof(struct S1));
  %p = call noalias i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.S1 = type { i32, i32 }

define void @test2() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s2 = bitcast i8* %p to %struct.S2*
  ; g(s2);
  call void @g(%struct.S2* %s2)
  ; free(s2);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.S2 = type { [32 x i32], %struct.S1 }

; This test checks the case where GEP is used to access fields of an
; allocated structure.
define void @test3() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s2 = bitcast i8* %p to %struct.S2*
  ; g(s2);
  call void @g(%struct.S2* %s2)
  ; f(&(s2->s1));
  %p2 = getelementptr inbounds i8, i8* %p, i64 128
  %s1 = bitcast i8* %p2 to %struct.S1*
  call void @f(%struct.S1* %s1)
  ; h(s2->a[4]);
  %p3 = getelementptr inbounds i8, i8* %p, i64 16
  %pa4 = bitcast i8* %p3 to i32*
  %a4 = load i32, i32* %pa4
  call void @h(i32 %a4)
  ; free(s2);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.S2 = type { [32 x i32], %struct.S1 }

define void @test4() {
  ; arr = (int*)malloc(32*32*sizeof(int));
  %p = tail call noalias i8* @malloc(i64 4096)
  %arr = bitcast i8* %p to [32 x [32 x i32]]*
  ; i(arr);
  call void @i([32 x [32 x i32]]* %arr)
  ; free(arr);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [32 x [32 x i32]]

define void @test5() {
  ; s2 = (struct S2*)malloc(sizeof(struct S2));
  %p = tail call noalias i8* @malloc(i64 136)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  %s2 = bitcast i8* %p to %struct.S2*
  ; g(s2);
  call void @g(%struct.S2* %s2)
  ; free(s2);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to multiple types
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.S2 = type { [32 x i32], %struct.S1 }
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [17 x %struct.S1]

; This test checks the case where an array of structures is allocated.
define void @test6() {
  ; s1 = (struct S1*)malloc(32*sizeof(struct S1));
  %p = tail call noalias i8* @malloc(i64 256)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [32 x %struct.S1]

declare void @f(%struct.S1*)
declare void @g(%struct.S2*)
declare void @h(i32)
declare void @i([32 x [32 x i32]]*)

declare noalias i8* @malloc(i64)
declare void @free(i8* nocapture)
