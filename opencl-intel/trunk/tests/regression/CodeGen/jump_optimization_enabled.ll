; RUN: llc %s -o %t.asm
; RUN: FileCheck %s --input-file=%t.asm
; CHECK: j
; CHECK: j
; CHECK: ret
; This test relates to the ticket CSSD100005214
; in GPUs jump's are so expensive so there's no need to do some lazy
; evaluation techniques which adds some jumps
define i32 @test(i32 %a, i32 %b) nounwind readnone {
  %c = icmp eq i32 %a, 1                          ; <i1> [#uses=1]
  %d = icmp eq i32 %b, 2
  %e = or i1 %c, %d
  br i1 %e, label %1, label %2

; <label>:2                                       ; preds = %0
  ret i32 0

; <label>:1                                       ; preds = %0
  ret i32 1
}
