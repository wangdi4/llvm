; REQUIRES: asserts
; This test checks that whole program was detected since the alias @aliasfunc
; was resolved. In this case we have an alias with multiple aliasees.

; RUN: opt %s -o %t_wp4alias.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-assume-executable %t_wp4alias.bc -o %t_wp4alias \
; RUN:     2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:   MAIN DEFINITION:  DETECTED
; CHECK:   LINKING AN EXECUTABLE:  DETECTED
; CHECK:   WHOLE PROGRAM READ:  DETECTED
; CHECK:   WHOLE PROGRAM SEEN:  DETECTED
; CHECK:   WHOLE PROGRAM SAFE:  DETECTED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$aliasfunc = comdat largest

@anon.6f7604b53389da5bee8b13e9daa87246.6 = private unnamed_addr constant { [2 x ptr] } { [2 x ptr] [ptr @add, ptr @sub] }, comdat($aliasfunc)
@aliasfunc = unnamed_addr alias ptr, getelementptr inbounds ({ [2 x ptr] }, ptr @anon.6f7604b53389da5bee8b13e9daa87246.6, i32 0, i32 0, i32 1)

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

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %aliasload = load ptr, ptr @aliasfunc, align 8
  %call1 = call i32 %aliasload(i32 %argc)
  ret i32 %call1
}
