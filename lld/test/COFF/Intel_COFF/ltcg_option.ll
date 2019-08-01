; This test checks that the option /LTCG works.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /ltcg flag is ignored
; RUN: not lld-link /subsystem:console /LTCG %t.obj 2>&1 | FileCheck -check-prefix=CHECKLTCG -allow-empty %s
; CHECKLTCG-NOT: could not open '/LTCG'

; Check that the /LTCG flag is ignored
; RUN: not lld-link /subsystem:console /ltcg %t.obj 2>&1 | FileCheck -check-prefix=CHECKLTCG2 -allow-empty %s
; CHECKLTCG2-NOT: could not open '/ltcg'

; Check that the /ltcg:OPTION flag is ignored
; RUN: not lld-link /subsystem:console /ltcg:incremental %t.obj 2>&1 | FileCheck -check-prefix=CHECKLTCGOPT -allow-empty %s
; CHECKLTCGOPT-NOT: lld-link: error: could not open '/ltcg:incremental'

; Check that the /LTCG:Option flag is ignored
; RUN: not lld-link /subsystem:console /LTCG:INCREMENTAL %t.obj 2>&1 | FileCheck -check-prefix=CHECKLTCGOPT2 -allow-empty %s
; CHECKLTCGOPT2-NOT: lld-link: error: could not open '/LTCG:INCREMENTAL'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}
