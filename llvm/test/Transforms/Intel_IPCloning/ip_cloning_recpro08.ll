; RUN: opt < %s -ip-cloning -inline -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning),cgscc(inline)' -S 2>&1 | FileCheck %s

; Test that after @foo is cloned as a recursive progression clone 8 times and
; all of those clones are inlined into @MAIN__, that @MAIN__ has the
; "contains-rec-pro-clone" attribute. Also, check that @bar and @foo, which
; were not recursive progression clones nor had a recursive progression
; inlined into them, are marked with the "noinline" and no attributes,
; respectively. (They are NOT marked with the "contains-rec-pro-clone"
; attribute.)

; CHECK: define dso_local void @MAIN__() [[ATTR1:#[0-9]+]]
; CHECK: define internal void @bar() [[ATTR2:#[0-9]+]]
; CHECK: define internal void @foo(i32* noalias nocapture readonly %0)
; CHECK: attributes [[ATTR1]] = { "contains-rec-pro-clone" }
; CHECK: attributes [[ATTR2]] = { noinline }

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

define dso_local void @MAIN__() {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @foo(i32* nonnull %1)
  call void @bar()
  ret void
}

define internal void @bar() #0 {
  %1 = load i32, i32* @count, align 8
  %2 = add nsw i32 %1, 1
  store i32 %2, i32* @count, align 8
  ret void
}

; Function Attrs: nounwind
define internal void @foo(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 8
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = add nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %14

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %14

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  call void @foo(i32* nonnull %2)
  %13 = add nsw i32 %12, 1
  br label %14

; <label>:14:                                     ; preds = %11, %9, %7
  ret void
}

attributes #0 = { noinline }
