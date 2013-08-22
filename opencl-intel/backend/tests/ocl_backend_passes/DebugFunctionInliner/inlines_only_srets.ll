; RUN: opt < %s -debug-inliner -S | FileCheck %s

%struct = type { i32 }

define i32 @g() {
  ret i32 1
}

define void @g_sret(%struct* sret %s) {
  %address = getelementptr %struct* %s, i32 0, i32 0
  store i32 2, i32* %address
  ret void
}

; CHECK: define i32 @f
define i32 @f() {
  ; CHECK: call i32 @g
  %value1 = call i32 @g()
  %alloca = alloca %struct
  ; CHECK-NOT: call void @g_sret
  ; CHECK: store i32 2
  call void @g_sret(%struct* sret %alloca)
  %address = getelementptr %struct* %alloca, i32 0, i32 0
  %value2 = load i32* %address
  %value = add i32 %value1, %value2
  ret i32 %value
}
