; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-complete-unroll-force-constprop -hir-details 2>&1 < %s | FileCheck %s

; The global @struct_blob contains global variables and functions inside it
; This test case is checking that DDRef simplification utility used by constant
; propagation can simplify the refs if the pointer type value is constant global 
; var, a function or nullptr. We give up on simplifying (@stuct_glob)[0][3].0
; which stores non-constant global var @char_glob.


; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %char_ld = (@stuct_glob)[0][i1].0;
; CHECK: |   %func_ld = (@stuct_glob)[0][i1].1;
; CHECK: + END LOOP


; Detailed mode checks are checking that-
; 1. AddressOf refs of @bar/@const_char_glob are assigned symbase of 2 which is
; GenericRvalSymbase as they cannot cause data dependencies.
; 2. We use consistent symbase for the new blobs @bar/@const_char_glob which
; were not in HIR before.

; CHECK: Dump After

; This inst was eliminated by copy propagation
; CHECK-NOT: %char_ld = null;
; CHECK: %func_ld = &((@bar)[0]);
; CHECK: <RVAL-REG> &((LINEAR ptr @bar)[i64 0]) inbounds  {sb:2}
; CHECK:    <BLOB> LINEAR ptr @bar {sb:[[BARSB:[0-9]+]]}

; CHECK: %char_ld = &((@const_char_glob)[0]);
; CHECK: <RVAL-REG> &((LINEAR ptr @const_char_glob)[i64 0]) inbounds  {sb:2}
; CHECK:    <BLOB> LINEAR ptr @const_char_glob {sb:[[GLOBSB:[0-9]+]]}

; CHECK: %func_ld = &((@bar)[0]);
; CHECK:    <BLOB> LINEAR ptr @bar {sb:[[BARSB]]}

; CHECK: %char_ld = &((@const_char_glob)[0]);
; CHECK:    <BLOB> LINEAR ptr @const_char_glob {sb:[[GLOBSB]]}

; CHECK: %func_ld = &((@bar)[0]);
; CHECK: %char_ld = (@stuct_glob)[0][3].0;
; CHECK: %func_ld = &((@bar)[0]);


%stuct.func.ptr = type { ptr, ptr }

@char_glob = internal unnamed_addr global i8 0
@const_char_glob = internal unnamed_addr constant i8 0
@stuct_glob = internal unnamed_addr constant [4 x %stuct.func.ptr] [%stuct.func.ptr { ptr null, ptr @bar }, %stuct.func.ptr { ptr @const_char_glob, ptr @bar }, %stuct.func.ptr { ptr @const_char_glob, ptr @bar }, %stuct.func.ptr { ptr @char_glob, ptr @bar }]


declare void @bar()

define void @foo() {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %gep1 = getelementptr [4 x %stuct.func.ptr], ptr @stuct_glob, i64 0, i64 %iv, i32 0
  %char_ld = load ptr, ptr %gep1
  %gep2 = getelementptr [4 x %stuct.func.ptr], ptr @stuct_glob, i64 0, i64 %iv, i32 1
  %func_ld = load ptr, ptr %gep2
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 4
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi ptr [ %char_ld, %loop ]
  ret void
}  

