; RUN: opt < %s -O3 -prepare-for-lto -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='lto-pre-link<O3>' -S 2>&1 | FileCheck %s

; Check that the sequence of stores is transformed into a llvm.memset,
; because memcopyopt is not disabled in prepare-for-lto phase when DTRANS is
; not enabled.

; CHECK: define fastcc void @index_tree_init
; CHECK: call{{.*}}@llvm.memset

%struct.index_tree = type { %struct.index_tree_node_s*, %struct.index_tree_node_s*, %struct.index_tree_node_s*, i32 }
%struct.index_tree_node_s = type { i64, i64, %struct.index_tree_node_s*, %struct.index_tree_node_s*, %struct.index_tree_node_s* }

define fastcc void @index_tree_init(%struct.index_tree* nocapture %tree) unnamed_addr #4 {
entry:
  %root = getelementptr inbounds %struct.index_tree, %struct.index_tree* %tree, i64 0, i32 0
  store %struct.index_tree_node_s* null, %struct.index_tree_node_s** %root, align 8
  %leftmost = getelementptr inbounds %struct.index_tree, %struct.index_tree* %tree, i64 0, i32 1
  store %struct.index_tree_node_s* null, %struct.index_tree_node_s** %leftmost, align 8
  %rightmost = getelementptr inbounds %struct.index_tree, %struct.index_tree* %tree, i64 0, i32 2
  store %struct.index_tree_node_s* null, %struct.index_tree_node_s** %rightmost, align 8
  %count = getelementptr inbounds %struct.index_tree, %struct.index_tree* %tree, i64 0, i32 3
  store i32 0, i32* %count, align 8
  ret void
}

