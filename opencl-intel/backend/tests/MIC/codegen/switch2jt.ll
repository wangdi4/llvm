; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

; source code for first function
;int func(int a, int x)
;{
;  int b, y;
;  switch (a) {
;    case 70: b = 576; break;
;    case 72: b = 4389584; break;
;    case 73: b = 78473; break;
;    case 74: b =  53653; break;
;    case 75: b = 6543; break;
;    default: b = 11; break;
;  }
;
;  switch (x) {
;    case 171: y = 5716; break;
;    case 172: y = 43584; break;
;    case 173: y = 784673; break;
;    case 174: y =  553653; break;
;    case 176: y = 63543; break;
;    default: y = 3311; break;
;  }
;
;  return (b+y);
;}
;

define i32 @func(i32 %a, i32 %x) nounwind readnone {
; KNF:  lea       .LJTI0_0(%rip), [[R1:%[a-z0-9]+]]
; KNF:  movl      ([[R1]],{{%[a-z0-9]+}},4), {{%[a-z0-9]+}}
; KNF:  jmp       *{{%[a-z0-9]+}}
; KNF:  lea       .LJTI0_1(%rip), [[R2:%[a-z0-9]+]]
; KNF:  movl      ([[R2]],{{%[a-z0-9]+}},4), {{%[a-z0-9]+}}
; KNF:  jmp       *{{%[a-z0-9]+}}
; KNF:  .globl .LJTI0_0
; KNF:  .LJTI0_0:
; KNF:  .globl .LJTI0_1
; KNF:  .LJTI0_1:

  switch i32 %a, label %5 [
    i32 70, label %6
    i32 72, label %1
    i32 73, label %2
    i32 74, label %3
    i32 75, label %4
  ]

; <label>:1                                       ; preds = %0
  br label %6

; <label>:2                                       ; preds = %0
  br label %6

; <label>:3                                       ; preds = %0
  br label %6

; <label>:4                                       ; preds = %0
  br label %6

; <label>:5                                       ; preds = %0
  br label %6

; <label>:6                                       ; preds = %5, %4, %3, %2, %1, %0
  %b.0 = phi i32 [ 11, %5 ], [ 6543, %4 ], [ 53653, %3 ], [ 78473, %2 ], [ 4389584, %1 ], [ 576, %0 ]
  switch i32 %x, label %11 [
    i32 171, label %12
    i32 172, label %7
    i32 173, label %8
    i32 174, label %9
    i32 176, label %10
  ]

; <label>:7                                       ; preds = %6
  br label %12

; <label>:8                                       ; preds = %6
  br label %12

; <label>:9                                       ; preds = %6
  br label %12

; <label>:10                                      ; preds = %6
  br label %12

; <label>:11                                      ; preds = %6
  br label %12

; <label>:12                                      ; preds = %11, %10, %9, %8, %7, %6
  %y.0 = phi i32 [ 3311, %11 ], [ 63543, %10 ], [ 553653, %9 ], [ 784673, %8 ], [ 43584, %7 ], [ 5716, %6 ]
  %13 = add nsw i32 %y.0, %b.0
  ret i32 %13
}


define i32 @func_b(i32 %x) nounwind readnone {
  %y = add i32 %x, 7
  ret i32 %y
}


define i32 @func1(i32 %x) nounwind readnone {
; <label>:0
; KNF:  lea       .LJTI2_0(%rip), [[R1:%[a-z0-9]+]]
; KNF:  movl      ([[R1]],{{%[a-z0-9]+}},4), {{%[a-z0-9]+}}
; KNF:  jmp       *{{%[a-z0-9]+}}
; KNF:  .globl .LJTI2_0
; KNF:  .LJTI2_0:

  switch i32 %x, label %9 [
    i32 102, label %10
    i32 103, label %1
    i32 104, label %2
    i32 105, label %3
    i32 106, label %4
    i32 107, label %5
    i32 108, label %6
    i32 109, label %7
    i32 110, label %8
  ]

; <label>:1                                       ; preds = %0
  br label %10

; <label>:2                                       ; preds = %0
  br label %10

; <label>:3                                       ; preds = %0
  br label %10

; <label>:4                                       ; preds = %0
  br label %10

; <label>:5                                       ; preds = %0
  br label %10

; <label>:6                                       ; preds = %0
  br label %10

; <label>:7                                       ; preds = %0
  br label %10

; <label>:8                                       ; preds = %0
  br label %10

; <label>:9                                       ; preds = %0
  br label %10

; <label>:10                                      ; preds = %9, %8, %7, %6, %5, %4, %3, %2, %1, %0
  %y.0 = phi i32 [ 784379, %9 ], [ 673443, %8 ], [ 6732, %7 ], [ 67322, %6 ], [ 673111, %5 ], [ 67311, %4 ], [ 6731, %3 ], [ 673, %2 ], [ 8908423, %1 ], [ 7564, %0 ]
  ret i32 %y.0
}

