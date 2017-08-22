; RUN: llc -march=nios2 -O0 < %s | FileCheck %s

@.str = private unnamed_addr constant [7 x i8] c"%08x \0A\00", align 1
declare i32 @printf(i8*, ...)

define i32 @load_byte(i8* %a) nounwind {
entry:
; CHECK: load_byte:
; CHECK:   ldb {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %b = load i8, i8* %a
  %c = sext i8 %b to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @load_ubyte(i8* %a) nounwind {
entry:
; CHECK: load_ubyte:
; CHECK:   ldbu {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %b = load i8, i8* %a
  %c = zext i8 %b to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @load_half(i16* %a) nounwind {
entry:
; CHECK: load_half:
; CHECK:   ldh {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %b = load i16, i16* %a
  %c = sext i16 %b to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @load_uhalf(i16* %a) nounwind {
entry:
; CHECK: load_uhalf:
; CHECK:   ldhu {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %b = load i16, i16* %a
  %c = zext i16 %b to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}

define i32 @load_word(i32* %a) nounwind {
entry:
; CHECK: load_word:
; CHECK:   ldw {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
  %c = load i32, i32* %a
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %c)
  ret i32 0
}
