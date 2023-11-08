; REQUIRES: asserts
; RUN: opt -passes='require<dopevectortype>' -debug-only=dopevectortype -disable-output < %s 2>&1 | FileCheck %s

; Test for no ifx.types.dv metadata.

; CHECK: DVTYPE: No metadata found.

