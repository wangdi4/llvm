; RUN: llc -march=nios2 -O3 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Additions

define i32 @add_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: add_reg:
; CHECK:   add {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = add i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @add_imm(i32 %a) nounwind {
entry:
; CHECK: add_imm:
; CHECK:   addi {{r[0-9]+}}, {{r[0-9]+}}, 512
  %c = add i32 %a, 512
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;; Subtraction

define i32 @sub_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: sub_reg:
; CHECK:   sub {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = sub i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @sub_imm(i32 %a) nounwind {
entry:
; CHECK: sub_imm:
; CHECK:   addi {{r[0-9]+}}, {{r[0-9]+}}, -512
  %c = sub i32 %a, 512
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

