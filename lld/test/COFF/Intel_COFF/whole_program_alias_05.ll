; REQUIRES: asserts
; This test checks that whole program wasn't achieved due to the following
; issues:
;
;  1) @sub2 is defined outside the module (Inputs/whole_program_alias_sub2.ll)
;     which produces a missing libfunc
;
;  2) @sub won't be internalized because it is used in @sub2
;
;  3) @aliasfunc aliases @add and @sub, but @sub wasn't internalized

; RUN: llvm-as -o %t_wp5alias.bc %s
; RUN: llc %p/Inputs/whole_program_alias_sub2.ll -o %t_foo.obj \
; RUN:          -filetype=obj
; RUN: lld-link /out:%t_wp5alias.exe /entry:main %t_wp5alias.bc %t_foo.obj /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK:   ALIASES UNRESOLVED: 1
; CHECK:       aliasfunc
; CHECK:   LIBFUNCS NOT FOUND: 1
; CHECK:       sub2
; CHECK:   VISIBLE OUTSIDE LTO: 1
; CHECK:       sub
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  NOT DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

$aliasfunc = comdat largest

@aliasfunc = unnamed_addr alias i8*, getelementptr inbounds ({ [2 x i8*] }, { [2 x i8*] }* @anon.6f7604b53389da5bee8b13e9daa87246.6, i32 0, i32 0, i32 1)
@anon.6f7604b53389da5bee8b13e9daa87246.6 = private unnamed_addr constant { [2 x i8*] } { [2 x i8*] [i8* bitcast (i32 (i32)* @add to i8*), i8* bitcast (i32 (i32)* @sub to i8*)] }, comdat($aliasfunc)

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub2(i32 %a)

define external i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %aliasload = load i8*, i8** @aliasfunc
  %aliascast = bitcast i8* %aliasload to i32 (i32)*
  %call1 = call i32 %aliascast(i32 %argc)
  %call2 = call i32 @sub2(i32 %call1)
  ret i32 %call2
}
