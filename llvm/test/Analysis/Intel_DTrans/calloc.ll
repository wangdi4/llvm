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
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 1);
  %p = call noalias i8* @calloc(i64 8, i64 1)
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
  ; s1 = (struct S1*)calloc(1, sizeof(struct S1));
  %p = call noalias i8* @calloc(i64 1, i64 8)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: %struct.S1 = type { i32, i32 }

; This test checks the case where an array of structures is allocated.
define void @test3() {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 32);
  %p = tail call noalias i8* @calloc(i64 8, i64 32)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [32 x %struct.S1]

; This test also checks the case where an array of structures is allocated
; but with the arguments reversed.
define void @test4() {
  ; s1 = (struct S1*)calloc(32, sizeof(struct S1));
  %p = tail call noalias i8* @calloc(i64 32, i64 8)
  %s1 = bitcast i8* %p to %struct.S1*
  ; f(s1);
  call void @f(%struct.S1* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [32 x %struct.S1]

; This test checks the case where an array of structures is allocated an cast
; to an array pointer.
define void @test5() {
  ; s1 = (struct S1*)calloc(sizeof(struct S1), 32);
  %p = tail call noalias i8* @calloc(i64 8, i64 32)
  %s1 = bitcast i8* %p to [32 x %struct.S1]*
  ; f(s1);
  call void @g([32 x %struct.S1]* %s1)
  ; free(s1);
  call void @free(i8* %p)
  ret void
}

; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK: Detected type: [32 x %struct.S1]

declare void @f(%struct.S1*)
declare void @g([32 x %struct.S1]*)

declare noalias i8* @calloc(i64, i64)
declare void @free(i8* nocapture)
