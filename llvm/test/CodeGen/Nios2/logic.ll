; RUN: llc -march=nios2 -O3 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Nor
define i32 @nor_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: nor_reg:
; CHECK:   nor {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %nc = or i32 %a, %b
  %c = xor i32 %nc, -1
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;; And

define i32 @and_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: and_reg:
; CHECK:   and {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = and i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @and_imm(i32 %a) nounwind {
entry:
; CHECK: and_imm:
; CHECK:   andi {{r[0-9]+}}, {{r[0-9]+}}, 512
  %c = and i32 %a, 512
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @andh_imm(i32 %a) nounwind {
entry:
; CHECK: andh_imm:
; CHECK:   andhi {{r[0-9]+}}, {{r[0-9]+}}, 512
  %k = shl i32 512, 16
  %c = and i32 %a, %k
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;; Or

define i32 @or_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: or_reg:
; CHECK:   or {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = or i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @or_imm(i32 %a) nounwind {
entry:
; CHECK: or_imm:
; CHECK:   ori {{r[0-9]+}}, {{r[0-9]+}}, 512
  %c = or i32 %a, 512
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @orh_imm(i32 %a) nounwind {
entry:
; CHECK: orh_imm:
; CHECK:   orhi {{r[0-9]+}}, {{r[0-9]+}}, 512
  %k = shl i32 512, 16
  %c = or i32 %a, %k
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;; Exclusive Or

define i32 @xor_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: xor_reg:
; CHECK:   xor {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %c = xor i32 %a, %b
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @xor_imm(i32 %a) nounwind {
entry:
; CHECK: xor_imm:
; CHECK:   xori {{r[0-9]+}}, {{r[0-9]+}}, 512
  %c = xor i32 %a, 512
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @xorh_imm(i32 %a) nounwind {
entry:
; CHECK: xorh_imm:
; CHECK:   xorhi {{r[0-9]+}}, {{r[0-9]+}}, 512
  %k = shl i32 512, 16
  %c = xor i32 %a, %k
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}
