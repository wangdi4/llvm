; RUN: llc -march=nios2 -O3 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Comparison with registers

define i32 @compare_equal_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_equal_reg:
; CHECK:   cmpeq {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp eq i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_not_equal_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_not_equal_reg:
; CHECK:   cmpne {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp ne i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sge_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_sge_reg:
; CHECK:   cmpge {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp sge i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sgt_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_sgt_reg:
; CHECK:   cmplt {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp sgt i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ugt_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_ugt_reg:
; CHECK:   cmpltu {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp ugt i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sle_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_sle_reg:
; CHECK:   cmpge {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp sle i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ule_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_ule_reg:
; CHECK:   cmpgeu {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp ule i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_slt_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_slt_reg:
; CHECK:   cmplt {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp slt i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ult_reg(i32 %a, i32 %b) nounwind {
entry:
; CHECK: compare_ult_reg:
; CHECK:   cmpltu {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
  %res = icmp ult i32 %a, %b
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

;; Comparison with immediate

define i32 @compare_equal_imm(i32 %a) nounwind {
entry:
; CHECK: compare_equal_imm:
; CHECK:   cmpeqi {{r[0-9]+}}, {{r[0-9]+}}, 17
  %res = icmp eq i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_not_equal_imm(i32 %a) nounwind {
entry:
; CHECK: compare_not_equal_imm:
; CHECK:   cmpnei {{r[0-9]+}}, {{r[0-9]+}}, 17
  %res = icmp ne i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sge_imm(i32 %a) nounwind {
entry:
; CHECK: compare_sge_imm:
; CHECK:   cmpgei {{r[0-9]+}}, {{r[0-9]+}}, 17
  %res = icmp sge i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sgt_imm(i32 %a) nounwind {
entry:
; CHECK: compare_sgt_imm:
; CHECK:   cmpgei {{r[0-9]+}}, {{r[0-9]+}}, 18
  %res = icmp sgt i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ugt_imm(i32 %a) nounwind {
entry:
; CHECK: compare_ugt_imm:
; CHECK:   cmpgeui {{r[0-9]+}}, {{r[0-9]+}}, 18
  %res = icmp ugt i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_sle_imm(i32 %a) nounwind {
entry:
; CHECK: compare_sle_imm:
; CHECK:   cmplti {{r[0-9]+}}, {{r[0-9]+}}, 18
  %res = icmp sle i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ule_imm(i32 %a) nounwind {
entry:
; CHECK: compare_ule_imm:
; CHECK:   cmpltui {{r[0-9]+}}, {{r[0-9]+}}, 18
  %res = icmp ule i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_slt_imm(i32 %a) nounwind {
entry:
; CHECK: compare_slt_imm:
; CHECK:   cmplti {{r[0-9]+}}, {{r[0-9]+}}, 17
  %res = icmp slt i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @compare_ult_imm(i32 %a) nounwind {
entry:
; CHECK: compare_ult_imm:
; CHECK:   cmpltui {{r[0-9]+}}, {{r[0-9]+}}, 17
  %res = icmp ult i32 %a, 17
  %c = zext i1 %res to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}
