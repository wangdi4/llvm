; RUN: llc -march=nios2 -O0 < %s | FileCheck %s

define i32 @store_byte(i8* %a, i32 %b) nounwind {
entry:
; CHECK: store_byte:
; CHECK:   stb {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %c = trunc i32 %b to i8
  store i8 %c, i8* %a
  ret i32 0
}

define i32 @store_half(i16* %a, i32 %b) nounwind {
entry:
; CHECK: store_half:
; CHECK:   sth {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %c = trunc i32 %b to i16
  store i16 %c, i16* %a
  ret i32 0
}

define i32 @store_word(i32* %a, i32 %b) nounwind {
entry:
; CHECK: store_word:
; CHECK:   stw {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  store i32 %b, i32* %a
  ret i32 0
}
