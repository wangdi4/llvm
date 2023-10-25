; RUN: opt -passes spi-emitter -S %s | FileCheck %s

; Currently, this test just checks that the pass name is recognized,
; as a valid pass.
; TODO: Extend this test as code for pass is developed.

; CHECK: define i32 @main()
; CHECK:   ret i32 0

define i32 @main() {
  ret i32 0
}
