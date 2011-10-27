; RUN: llc < %s

define noalias i32* @factorial() nounwind readnone {
entry:
  %a = alloca i32, align 64 
  ret i32* %a
}

