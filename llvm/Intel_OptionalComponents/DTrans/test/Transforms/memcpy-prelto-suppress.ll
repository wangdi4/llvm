; RUN: opt < %s -enable-npm-dtrans -passes='lto-pre-link<O3>' -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that the sequence of stores is not transformed into a llvm.memset,
; because memcopyopt is disabled in prepare-for-lto phase when DTRANS is
; enabled.

; CHECK: define fastcc void @index_tree_init
; CHECK-NOT: call{{.*}}@llvm.memset

%struct.index_tree = type { ptr, ptr, ptr, i32 }
%struct.index_tree_node_s = type { i64, i64, ptr, ptr, ptr }

define fastcc void @index_tree_init(ptr nocapture %tree) unnamed_addr #4 {
entry:
  %root = getelementptr inbounds %struct.index_tree, ptr %tree, i64 0, i32 0
  store ptr null, ptr %root, align 8
  %leftmost = getelementptr inbounds %struct.index_tree, ptr %tree, i64 0, i32 1
  store ptr null, ptr %leftmost, align 8
  %rightmost = getelementptr inbounds %struct.index_tree, ptr %tree, i64 0, i32 2
  store ptr null, ptr %rightmost, align 8
  %count = getelementptr inbounds %struct.index_tree, ptr %tree, i64 0, i32 3
  store i32 0, ptr %count, align 8
  ret void
}
