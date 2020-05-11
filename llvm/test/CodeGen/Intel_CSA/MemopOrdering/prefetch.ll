; RUN: opt -S -csa-memop-ordering <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_lone_prefetch(i8* %addr) {
; Prefetches don't need to be ordered with the function start/end.
; CHECK-LABEL: test_lone_prefetch

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  ret void
}

define i8 @test_prefetch_load(i8* %addr) {
; Prefetches (both load and store types) should be ordered with aliased loads.
; CHECK-LABEL: test_prefetch_load

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[LPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[LPREF]]

  call void @llvm.prefetch(i8* %addr, i32 1, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 1, i32 3, i32 1)
; CHECK-NEXT: %[[SPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[SPREF]]

  ret i8 %val
}

define void @test_prefetch_store(i8* %addr) {
; Load prefetches should not be used with stores, so they shouldn't be ordered
; with them.
; CHECK-LABEL: test_prefetch_store

  store i8 0, i8* %addr
; CHECK: store i8 0, i8* %addr
; CHECK-NEXT: %[[STORE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[LPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[LPREF]]

  call void @llvm.prefetch(i8* %addr, i32 1, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[STORE]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 1, i32 3, i32 1)
; CHECK-NEXT: %[[SPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[SPREF]]

  ret void
}

define {i8, i1} @test_prefetch_atomic(i8* %addr) {
; Likewise for RMW/cmpxchg atomics.
; CHECK-LABEL: test_prefetch_atomic

  %val = cmpxchg i8* %addr, i8 0, i8 1 seq_cst seq_cst
; CHECK: %val = cmpxchg i8* %addr, i8 0, i8 1 seq_cst seq_cst
; CHECK-NEXT: %[[ATOMIC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[LPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[LPREF]]

  call void @llvm.prefetch(i8* %addr, i32 1, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[ATOMIC]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 1, i32 3, i32 1)
; CHECK-NEXT: %[[SPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[SPREF]]

  ret {i8, i1} %val
}

define i8 @test_prefetch_offs(i8* %addr) {
; Prefetches should be ordered with non-aliased loads which alias under an
; arbitrary-size query.
; CHECK-LABEL: test_prefetch_offs

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %addr_ahead = getelementptr inbounds i8, i8* %addr, i32 64
  call void @llvm.prefetch(i8* %addr_ahead, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr_ahead, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[APREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[APREF]]

  %addr_behind = getelementptr inbounds i8, i8* %addr, i32 -64
  call void @llvm.prefetch(i8* %addr_behind, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr_behind, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[BPREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[BPREF]]

  ret i8 %val
}

define i8 @test_prefetch_noalias(i8* noalias %addr, i8* noalias %addr1) {
; If an address is completely non-aliased, don't order with it.
; CHECK-LABEL: test_prefetch_noalias

  %val = load i8, i8* %addr1
; CHECK: %val = load i8, i8* %addr1

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[APREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[APREF]]

  ret i8 %val
}

define i8 @test_prefetch_simple_loop(i8* %addr) {
; Check that loop-local prefetch ordering works for simple loops.
; CHECK-LABEL: test_prefetch_simple_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  br label %loop

loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop ]

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  br i1 undef, label %loop, label %exit

exit:

  ret i8 %next_sum
}

define i8 @test_prefetch_carried_loop(i8* %addr) {
; Check that loop-local carried prefetch ordering works for simple loops.
; CHECK-LABEL: test_prefetch_carried_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  br label %loop

loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop ]
; CHECK: %[[PHI:[a-z0-9_.]+]] = phi i1 [ false, %entry ], [ %[[LOAD:[a-z0-9_.]+]], %loop ]

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  br i1 undef, label %loop, label %exit

exit:

  ret i8 %next_sum
}

define i8 @test_prefetch_nested_loop(i8* %addr) {
; Check that loop-local prefetch ordering works for more complicated loops.
; CHECK-LABEL: test_prefetch_nested_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  br label %loop

loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop_end ]

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  br label %nest

nest:

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  br i1 undef, label %nest, label %loop_end

loop_end:

  br i1 undef, label %loop, label %exit

exit:

  ret i8 %next_sum
}

define i8 @test_prefetch_nest_carried_loop(i8* %addr) {
; Check that loop-local carried prefetch ordering works for nested loops.
; CHECK-LABEL: test_prefetch_nest_carried_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  br label %loop

loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop_end ]
; CHECK: %[[PHI:[a-z0-9_.]+]] = phi i1 [ false, %entry ], [ %[[LOAD:[a-z0-9_.]+]], %loop_end ]

  br label %nest

nest:

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  br i1 undef, label %nest, label %loop_end

loop_end:

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  br i1 undef, label %loop, label %exit

exit:

  ret i8 %next_sum
}

define i8 @test_prefetch_sibling_loop(i8* %addr) {
; Check that loop-local carried prefetch ordering works for sibling loops.
; CHECK-LABEL: test_prefetch_sibling_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  br label %loop

loop:
; CHECK: loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop_end ]
; CHECK: %[[PHI:[a-z0-9_.]+]] = phi i1 [ false, %entry ], [ %[[LOAD:[a-z0-9_.]+]], %loop_end ]

  br label %nest

nest:
; CHECK: nest:

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  br i1 undef, label %nest, label %sib

sib:

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  br i1 undef, label %sib, label %loop_end

loop_end:

  br i1 undef, label %loop, label %exit

exit:

  ret i8 %next_sum
}

define i8 @test_prefetch_parallel_loop(i8* %addr) {
; Check that loop-local carried prefetch ordering works for parallel loops.
; CHECK-LABEL: test_prefetch_parallel_loop

entry:

  %init = load i8, i8* %addr
; CHECK: %init = load i8, i8* %addr

  %pre = call i32 @llvm.csa.parallel.region.entry(i32 0)

  br label %loop

loop:
  %prev_sum = phi i8 [ %init, %entry ], [ %next_sum, %loop ]
; CHECK: %[[PHI:[a-z0-9_.]+]] = phi i1 [ false, %entry ], [ %[[LOAD:[a-z0-9_.]+]], %loop ]

  %pse = call i32 @llvm.csa.parallel.section.entry(i32 %pre)

  call void @llvm.prefetch(i8* %addr, i32 0, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr
; CHECK-NEXT: %[[LOAD]] = call i1 @llvm.csa.outord()

  %next_sum = add i8 %prev_sum, %val

  call void @llvm.csa.parallel.section.exit(i32 %pse)

  br i1 undef, label %loop, label %exit

exit:

  call void @llvm.csa.parallel.region.exit(i32 %pre)

  ret i8 %next_sum
}

define i8 @test_prefetch_nested_parallel_stores(i8* %addr) {
; Check that prefetch ordering works in a case simulating the most relevant ones
; from DLPBench.
; CHECK-LABEL: test_prefetch_nested_parallel_stores

entry:

  %val = load i8, i8* %addr
; CHECK: %val = load i8, i8* %addr

  %pre = call i32 @llvm.csa.parallel.region.entry(i32 0)

  br label %loop

loop:
; CHECK: %[[PHI:[a-z0-9_.]+]] = phi i1 [ false, %entry ], [ %[[STORE:[a-z0-9_.]+]], %loop_end ]

  %pse = call i32 @llvm.csa.parallel.section.entry(i32 %pre)

  br label %prefloop

prefloop:

  call void @llvm.prefetch(i8* %addr, i32 1, i32 3, i32 1)
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: call void @llvm.prefetch.p0i8(i8* %addr, i32 1, i32 3, i32 1)
; CHECK-NEXT: %[[PREF:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
; CHECK-NOT: %[[PREF]]

  br i1 undef, label %prefloop, label %loop_mid

loop_mid:

  br label %stloop


stloop:

  store i8 %val, i8* %addr
; CHECK: store i8 %val, i8* %addr
; CHECK-NEXT: %[[STORE]] = call i1 @llvm.csa.outord()

  br i1 undef, label %stloop, label %loop_end

loop_end:

  call void @llvm.csa.parallel.section.exit(i32 %pse)

  br i1 undef, label %loop, label %exit

exit:

  call void @llvm.csa.parallel.region.exit(i32 %pre)

  ret i8 %val
}

declare void @llvm.prefetch(i8*, i32, i32, i32)
declare i32 @llvm.csa.parallel.region.entry(i32)
declare void @llvm.csa.parallel.region.exit(i32)
declare i32 @llvm.csa.parallel.section.entry(i32)
declare void @llvm.csa.parallel.section.exit(i32)
