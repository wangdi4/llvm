; RUN: opt < %s -S -passes="globalopt,function(instcombine)" | FileCheck %s
;; check that Global Opt treats glob1 as constant integer value 20 and 
;; store 20 to glob2 and glob3 variables.

; CHECK: store i32 20, ptr @glob2, align 4
; CHECK: store i32 20, ptr @glob3, align 4

@glob2 = local_unnamed_addr global i32 0, align 4
@glob3 = local_unnamed_addr global i32 0, align 4
@glob1 = internal unnamed_addr global i32 15, align 4

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr  {
entry:
  store i32 20, ptr @glob1, align 4
  tail call void @bar() 
  %0 = load i32, ptr @glob1, align 4
  store i32 %0, ptr @glob2, align 4
  store i32 %0, ptr @glob3, align 4
  ret void
}

declare void @bar() local_unnamed_addr 

