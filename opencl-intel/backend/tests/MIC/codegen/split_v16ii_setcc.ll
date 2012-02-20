; XFAIL: win32
; This tests causes the legalizer to split a v16ii setcc node.
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf 
;

define void @A() {
entry:
  br label %.critedge.i

.critedge.i:
  %vectorPHI183.i = phi <16 x i1> [ %local_edge20227.i, %.critedge.i ], [ undef, %entry ]
  %extract202.i = extractelement <16 x i1> %vectorPHI183.i, i32 15
  %exitcond14.i = icmp eq <16 x i64> undef, zeroinitializer
  %local_edge20227.i = xor <16 x i1> %exitcond14.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %.critedge.i
}
