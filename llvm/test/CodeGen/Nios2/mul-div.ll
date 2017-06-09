; RUN: llc -march=nios2 -O3 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Multiplications

define i32 @mul_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: mul_reg:
; CHECK:   mul {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = mul i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @mul_imm(i32 %a) nounwind {
entry:
; CHECK: mul_imm:
; CHECK:   muli {{r[0-9]+}}, {{r[0-9]+}}, 217
  %mul = mul i32 %a, 217
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %mul)
  ret i32 0
}

define i32 @mulxss_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: mulxss_reg:
; CHECK:   mulxss {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %za = sext i32 %a to i64
  %zb = sext i32 %b to i64
  %c = mul i64 %za, %zb
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i64 %c)
  ret i32 0
}

define i32 @mulxuu_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: mulxuu_reg:
; CHECK:   mulxuu {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %za = zext i32 %a to i64
  %zb = zext i32 %b to i64
  %c = mul i64 %za, %zb
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i64 %c)
  ret i32 0
}

;; Divisions

define i32 @div_signed(i32 %a, i32 %b) nounwind {
entry:
; CHECK: div_signed:
; CHECK:   div {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = sdiv i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @div_unsigned(i32 %a, i32 %b) nounwind {
entry:
; CHECK: div_unsigned:
; CHECK:   divu {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = udiv i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

