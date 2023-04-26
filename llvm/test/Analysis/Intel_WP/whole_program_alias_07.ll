; REQUIRES: asserts
; This test checks that the alias @aliasfunc and @aliasfunc2 weren't resolved
; since there is a recursion.

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   ALIASES UNRESOLVED: 2
; CHECK:     aliasfunc
; CHECK:     aliasfunc2
; CHECK:   LIBFUNCS NOT FOUND: 0
; CHECK:   VISIBLE OUTSIDE LTO: 0

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$aliasfunc = comdat largest

@anon.6f7604b53389da5bee8b13e9daa87246.6 = private unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr @add, ptr @sub, ptr @aliasfunc2] }, comdat($aliasfunc)

@aliasfunc = unnamed_addr alias ptr, getelementptr inbounds ({ [3 x ptr] }, ptr @anon.6f7604b53389da5bee8b13e9daa87246.6, i32 0, i32 0, i32 1)
@aliasfunc2 = unnamed_addr alias ptr, ptr @aliasfunc

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
  %aliascast = bitcast ptr %aliasload to ptr
  %call1 = call i32 %aliascast(i32 %argc)
  ret i32 %call1
}
