; RUN: llc -march=nios2 -O0 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

;; Comparison with registers

define i32 @branch(i32 %a, void (i32)* %b) nounwind {
entry:
; CHECK: branch:
; CHECK:   callr
; CHECK:   br
; CHECK:   call
; CHECK:   ret
  call void %b(i32 %a)
  br label %exit

exit:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %a)
  ret i32 0
}


