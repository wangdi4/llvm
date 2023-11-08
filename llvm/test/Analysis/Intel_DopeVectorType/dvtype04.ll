; REQUIRES: asserts
; RUN: opt -passes='require<dopevectortype>' -debug-only=dopevectortype -disable-output < %s 2>&1 | FileCheck %s

; Test for empty ifx.types.dv metadata.

!ifx.types.dv = !{}

; CHECK: DVTYPE: Empty metadata list.
