define void @g() {
  ret void
}

@u = linkonce global i8 41

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65534, ptr @g, ptr @u }]
