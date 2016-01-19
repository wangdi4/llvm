; RUN:  opt  -sna -analyze < %s | FileCheck %s
;
; The compiler expects the following output.
; CHECK: SN[5]  pred:  succ:
; CHECK: SN_LIST SN[5]
; CHECK:    SN_IF_THEN_ELSE SN[4]
; CHECK:        SN_BLOCK SN[0] B[%entry]
; CHECK:        SN_BLOCK SN[1] B[%if.then]
; CHECK:        SN_BLOCK SN[2] B[%if.end]
; CHECK:    END  SN_IF_THEN_ELSE SN[4]
; CHECK:    SN_BLOCK SN[3] B[%return]
; CHECK:END  SN_LIST SN[5]
;
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128" 
target triple = "x86_64-unknown-linux-gnu" 

@a = common global i32 0, align 4
; Function Attrs: nounwind uwtable
define i32 @foo() {
entry:
  %retval = alloca i32, align 4
  %0 = load i32, i32* @a, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 1, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %1 = load i32, i32* %retval, align 4
  ret i32 %1
}
