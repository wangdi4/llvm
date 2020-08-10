; This function is part of the whole_program_alias_02.ll test cases. The
; function @sub2 will be called from @main (whole_program_alias_02.ll) . The
; whole program analysis should produce whole program not detected since @sub2
; will be treated as missing libfunc.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @sub(i32 %a)

define i32 @sub2(i32 %a) {
entry:
  %sub = call i32 @sub(i32 %a)
  ret i32 %sub
}