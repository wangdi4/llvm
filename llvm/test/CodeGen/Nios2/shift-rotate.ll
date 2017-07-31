; RUN: llc -march=nios2 -O3 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Rotations:

;;;; Registers:

define i32 @ror_reg(i32 %a, i32 %b) nounwind {
entry:
; C HECK: ror_reg:
; C HECK:   ror {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %shl = shl i32 %a, %b
  %shr = lshr i32 %a, 22
  %or = or i32 %shl, %shr
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %or)
  ret i32 0
}

define i32 @rol_reg(i32 %a, i32 %b) nounwind {
entry:
; C HECK: rol_reg:
; C HECK:   rol {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %shr = lshr i32 %a, 22
  %shl = shl i32 %a, %b
  %or = or i32 %shr, %shl 
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %or)
  ret i32 0
}

;;;; Immediates:

define i32 @rol_imm(i32 %a) nounwind {
entry:
; CHECK: rol_imm:
; CHECK:   roli {{r[0-9]+}}, {{r[0-9]+}}, 10
  %shr = lshr i32 %a, 22
  %shl = shl i32 %a, 10
  %or = or i32 %shr, %shl 
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %or)
  ret i32 0
}

;; Shifts:

;;;; Registers:

define i32 @sll_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: sll_reg:
; CHECK:   sll {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = shl i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @srl_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: srl_reg:
; CHECK:   srl {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = lshr i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @sra_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: sra_reg:
; CHECK:   sra {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = ashr i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;;;; Immediates:

define i32 @sll_imm(i32 %a) nounwind {
entry:
; CHECK: sll_imm:
; CHECK:   slli {{r[0-9]+}}, {{r[0-9]+}}, 21
  %c = shl i32 %a, 21
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @srl_imm(i32 %a) nounwind {
entry:
; CHECK: srl_imm:
; CHECK:   srli {{r[0-9]+}}, {{r[0-9]+}}, 21
  %c = lshr i32 %a, 21
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @sra_imm(i32 %a) nounwind {
entry:
; CHECK: sra_imm:
; CHECK:   srai {{r[0-9]+}}, {{r[0-9]+}}, 21
  %c = ashr i32 %a, 21
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}
