; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define i8 @mul_i8_test(i8* %ptr, i8 %imm) {
  %element = load i8* %ptr, align 1
  %res = mul i8 %element, %imm
  ret i8 %res
}

define i8 @sdiv_i8_test(i8* %ptr, i8 %imm) {
  %element = load i8* %ptr, align 1
  %res = sdiv i8 %element, %imm
  ret i8 %res
}

define i8 @add_i8_test(i8* %ptr, i8 %imm) {
  %element = load i8* %ptr, align 1
  %res = add i8 %element, %imm
  ret i8 %res
}

define i8 @sub_i8_test(i8* %ptr, i8 %imm) {
  %element = load i8* %ptr, align 1
  %res = sub i8 %element, %imm
  ret i8 %res
}

define i16 @mul_i16_test(i16* %ptr, i16 %imm) {
  %element = load i16* %ptr, align 2
  %res = mul i16 %element, %imm
  ret i16 %res
}

define i16 @sdiv_i16_test(i16* %ptr, i16 %imm) {
  %element = load i16* %ptr, align 2
  %res = sdiv i16 %element, %imm
  ret i16 %res
}

define i16 @add_i16_test(i16* %ptr, i16 %imm) {
  %element = load i16* %ptr, align 2
  %res = add i16 %element, %imm
  ret i16 %res
}

define i16 @sub_i16_test(i16* %ptr, i16 %imm) {
  %element = load i16* %ptr, align 2
  %res = sub i16 %element, %imm
  ret i16 %res
}
