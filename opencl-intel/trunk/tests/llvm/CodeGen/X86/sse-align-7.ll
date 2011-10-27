; XFAIL: x86_64-pc-win32, i686-pc-win32
; RUN: llc < %s -march=x86-64 | grep movaps | count 1

define void @bar(<2 x i64>* %p, <2 x i64> %x) nounwind {
  store <2 x i64> %x, <2 x i64>* %p
  ret void
}
