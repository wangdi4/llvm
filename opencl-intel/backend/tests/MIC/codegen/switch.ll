; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define i32 @func(i32 %x) nounwind readnone {
; <label>:0
; KNF:  lea       .LJTI0_0(%rip), [[R1:%[a-z0-9]+]]
; KNF:  movl      ([[R1]],{{%[a-z0-9]+}},4), {{%[a-z0-9]+}}
; KNF:  jmp       *{{%[a-z0-9]+}}
; KNF:  .globl .LJTI0_0
; KNF:  .LJTI0_0:

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

