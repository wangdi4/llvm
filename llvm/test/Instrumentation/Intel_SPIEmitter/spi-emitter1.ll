; REQUIRES: asserts
; RUN: opt -passes spi-emitter -spi-generate=true -debug-only=spi-emitter -disable-output %s 2>&1 | FileCheck %s

; Test the spi-emitter pass exists, and when run on IR
; which is not enabled with coverage mapping symbols, just
; reports that fact.

; CHECK: spi-emitter: No code coverage symbols found

define i32 @main() {
  ret i32 0
}
