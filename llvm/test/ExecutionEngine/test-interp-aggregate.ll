; RUN: %lli -O0 -force-interpreter=true %s

@U = global [2 x i32] [i32 42, i32 4]
@V = global {i32, i32} zeroinitializer
@W = global [2 x i32] undef

define i32 @main() {
  %1 = load [2 x i32], [2 x i32]* @U
  %2 = extractvalue [2 x i32] %1, 0
  %3 = extractvalue [2 x i32] %1, 1
  %4 = add i32 %2, %3
  %5 = sub i32 %4, 46
  %6 = load {i32, i32}, {i32, i32}* @V
  %7 = extractvalue {i32, i32} %6, 0
  %8 = extractvalue {i32, i32} %6, 1
  %9 = sub i32 %5, %7
  %10 = sub i32 %9, %8
  %11 = insertvalue {i32, i32} %6, i32 10, 0
  store [2 x i32] %1, [2 x i32]* @W
  store {i32, i32} %11, {i32, i32}* @V
  ret i32 %10
}
