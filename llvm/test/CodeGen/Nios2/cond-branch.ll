; RUN: llc -march=nios2 -O0 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Comparison with registers

define i32 @branch_equal(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_equal:
; CHECK:   beq {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp eq i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_not_equal(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_not_equal:
; CHECK:   bne {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp ne i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_sge(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_sge:
; CHECK:   bge {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp sge i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_uge(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_uge:
; CHECK:   bgeu {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp uge i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_sgt(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_sgt:
; CHECK:   blt {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp sgt i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_ugt(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_ugt:
; CHECK:   bltu {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp ugt i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_sle(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_sle:
; CHECK:   bge {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp sle i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_ule(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_ule:
; CHECK:   bgeu {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp ule i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_slt(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_slt:
; CHECK:   blt {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp slt i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}

define i32 @branch_ult(i32 %a, i32 %b) nounwind {
entry:
; CHECK: branch_ult:
; CHECK:   bltu {{r[0-9]+}}, {{r[0-9]+}}, {{.[0-9a-zA-Z]+}}
  %res = icmp ult i32 %a, %b
  br i1 %res, label %exit, label %print

print:
  %call_a = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  br label %exit

exit:
  ret i32 0
}
