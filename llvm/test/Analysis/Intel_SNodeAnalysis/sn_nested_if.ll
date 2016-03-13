; RUN:  opt  -sna -analyze < %s | FileCheck %s
;
; The compiler expects the following output.
;
; CHECK: SN[8]  pred:  succ:
; CHECK: SN_LIST SN[8]
; CHECK:    SN_IF_THEN_ELSE SN[7]
; CHECK:        SN_BLOCK SN[0] B[%entry]
; CHECK:        SN_BLOCK SN[1] B[%if.then]
; CHECK:        SN_IF_THEN_ELSE SN[6]
; CHECK:            SN_BLOCK SN[2] B[%if.else.3]
; CHECK:            SN_BLOCK SN[3] B[%if.then.5]
; CHECK:            SN_BLOCK SN[4] B[%if.end.7]
; CHECK:        END  SN_IF_THEN_ELSE SN[6]
; CHECK:    END  SN_IF_THEN_ELSE SN[7]
; CHECK:    SN_BLOCK SN[5] B[%return]
; CHECK: END  SN_LIST SN[8]
 
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"d\0A\00", align 1
@str = private unnamed_addr constant [2 x i8] c"d\00"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %i) {
entry:
  %cmp = icmp sgt i32 %i, 100
  br i1 %cmp, label %if.then, label %if.else.3

if.then:                                          ; preds = %entry
  %cmp1 = icmp slt i32 %i, 1000
  %.v = select i1 %cmp1, i32 1, i32 -2
  %0 = add i32 %.v, %i
  br label %return

if.else.3:                                        ; preds = %entry
  %cmp4 = icmp slt i32 %i, 10
  br i1 %cmp4, label %if.then.5, label %if.end.7

if.then.5:                                        ; preds = %if.else.3
  %add6 = add nsw i32 %i, 3
  br label %return

if.end.7:                                         ; preds = %if.else.3
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @str, i64 0, i64 0))
  br label %return

return:                                           ; preds = %if.then, %if.end.7, %if.then.5
  %retval.0 = phi i32 [ %add6, %if.then.5 ], [ 0, %if.end.7 ], [ %0, %if.then ]
  ret i32 %retval.0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly)

