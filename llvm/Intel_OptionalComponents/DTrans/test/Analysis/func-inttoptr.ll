; This test checks that integer-to-ptr won't cause a segmentation
; fault during DTrans with whole program analysis and LTO.

target triple = "x86_64-unknown-linux-gnu"

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -enable-dtrans=true -exported-symbol=main -whole-program-assume -intel-libirc-allowed -o %t2 %t1

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
  %bc = bitcast i32 (i32)* @add to i32 (i32,i32)*
  %call1 = call i32 %bc(i32 %argc, i32 %argc)
  %fun = inttoptr i32 1 to i32 (i32)*
  %call2 = call i32 %fun(i32 %call1)
  ret i32 %call2
}
