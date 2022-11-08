; REQUIRES: asserts
; This test checks that whole program was detected since the alias @aliasfunc
; was resolved. In this case we have an alias with multiple aliasees.

; RUN: llvm-as -o %t_wp4alias.bc %s
; RUN: lld-link /out:%t_wp4alias.exe /entry:main %t_wp4alias.bc /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:   MAIN DEFINITION:  DETECTED
; CHECK:   LINKING AN EXECUTABLE:  DETECTED
; CHECK:   WHOLE PROGRAM READ:  DETECTED
; CHECK:   WHOLE PROGRAM SEEN:  DETECTED
; CHECK:   WHOLE PROGRAM SAFE:  DETECTED

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

define internal i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %aliasload = load i8*, i8** @aliasfunc
  %aliascast = bitcast i8* %aliasload to i32 (i32)*
  %call1 = call i32 %aliascast(i32 %argc)
  ret i32 %call1
}
