; RUN: opt -S -csa-memop-ordering <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_simple_store(i32 %len) {
; Does a simple pool with a single store work?
; CHECK-LABEL: test_simple_store
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool = call i8* @CsaMemAlloc(i32 20)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool = call i8* @CsaMemAlloc(i32 20)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]

  %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 1, i32 20)
  ; The token take call should be explicitly ordered after the CsaMemAlloc.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 1, i32 20)
; CHECK-NEXT: %[[TOKENTAKE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  store i8 0, i8* %pooli
  ; The store should be implicitly ordered after the token take.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i8 0, i8* %pooli
; CHECK-NEXT: %[[STORE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
  ; The token return should be explicitly ordered after the store.
; CHECK: call void @llvm.csa.inord(i1 %[[STORE]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
; CHECK-NEXT: %[[TOKENRETURN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool)
  ; The CsaMemFree should be explicitly ordered after the token return.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENRETURN]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret void
}

define i32 @test_store_load(i32 %len) {
; What about one with a load too?
; CHECK-LABEL: test_store_load
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool = call i8* @CsaMemAlloc(i32 80)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool = call i8* @CsaMemAlloc(i32 80)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
  %cursum = phi i32 [ 0, %entry ], [ %nextsum, %loop ]

  %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 4, i32 20)
  ; The token take call should be explicitly ordered after the CsaMemAlloc.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 4, i32 20)
; CHECK-NEXT: %[[TOKENTAKE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pooli32 = bitcast i8* %pooli to i32*
  store i32 0, i32* %pooli32, align 4
  ; The store should be implicitly ordered after the token take.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i32 0, i32* %pooli32, align 4
; CHECK-NEXT: %[[STORE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %loaded = load i32, i32* %pooli32, align 4
  ; The load should be explicitly ordered after the store.
; CHECK: call void @llvm.csa.inord(i1 %[[STORE]])
; CHECK-NEXT: %loaded = load i32, i32* %pooli32, align 4
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
  ; The token return should be explicitly ordered after the load.
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
; CHECK-NEXT: %[[TOKENRETURN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %nextsum = add i32 %cursum, %loaded
  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool)
  ; The CsaMemFree should be explicitly ordered after the token return.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENRETURN]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret i32 %nextsum
}

define i32 @test_simple_array(i32 %len) {
; An array of local values?
; CHECK-LABEL: test_simple_array
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool = call i8* @CsaMemAlloc(i32 160)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool = call i8* @CsaMemAlloc(i32 160)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
  %cursum = phi i32 [ 0, %entry ], [ %nextsum, %loop ]

  %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 8, i32 20)
  ; The token take call should be explicitly ordered after the CsaMemAlloc.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 8, i32 20)
; CHECK-NEXT: %[[TOKENTAKE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pooli32 = bitcast i8* %pooli to i32*
  store i32 0, i32* %pooli32, align 4
  ; The first store should be implicitly ordered after the token take.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i32 0, i32* %pooli32, align 4
; CHECK-NEXT: %[[STORE0:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pool1 = getelementptr inbounds i32, i32* %pooli32, i32 1
  store i32 1, i32* %pool1, align 4
  ; As should the second.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i32 1, i32* %pool1, align 4
; CHECK-NEXT: %[[STORE1:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %loadidx = and i32 %i, 1
  %loadaddr = getelementptr inbounds i32, i32* %pooli32, i32 %loadidx
  %loaded = load i32, i32* %loadaddr, align 4
  ; The load should be explicitly ordered after both stores.
; CHECK: %[[STORES:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %[[STORE0]], i1 %[[STORE1]])
; CHECK: call void @llvm.csa.inord(i1 %[[STORES]])
; CHECK-NEXT: %loaded = load i32, i32* %loadaddr, align 4
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
  ; The token return should be explicitly ordered after the load.
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
; CHECK-NEXT: %[[TOKENRETURN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %nextsum = add i32 %cursum, %loaded
  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool)
  ; The CsaMemFree should be explicitly ordered after the token return.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENRETURN]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret i32 %nextsum
}

define i32 @test_call(i32 %len) {
; What about calls?
; CHECK-LABEL: test_call
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool = call i8* @CsaMemAlloc(i32 160)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool = call i8* @CsaMemAlloc(i32 160)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
  %cursum = phi i32 [ 0, %entry ], [ %nextsum, %loop ]

  %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 8, i32 20)
  ; The token take call should be explicitly ordered after the CsaMemAlloc.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 8, i32 20)
; CHECK-NEXT: %[[TOKENTAKE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pooli32 = bitcast i8* %pooli to i32*
  call void @init_array(i32* %pooli32)
  ; This call should be explicitly ordered with the token take.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENTAKE]])
; CHECK-NEXT: call void @init_array(i32* %pooli32)
; CHECK-NEXT: %[[CALL:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %loadidx = and i32 %i, 1
  %loadaddr = getelementptr inbounds i32, i32* %pooli32, i32 %loadidx
  %loaded = load i32, i32* %loadaddr, align 4
  ; The load should be explicitly ordered with the call.
; CHECK: call void @llvm.csa.inord(i1 %[[CALL]])
; CHECK-NEXT: %loaded = load i32, i32* %loadaddr, align 4
; CHECK-NEXT: %[[LOAD:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
  ; The token return should be explicitly ordered after the load.
; CHECK: call void @llvm.csa.inord(i1 %[[LOAD]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
; CHECK-NEXT: %[[TOKENRETURN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %nextsum = add i32 %cursum, %loaded
  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool)
  ; The CsaMemFree should be explicitly ordered after the token return.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENRETURN]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret i32 %nextsum
}

declare void @init_array(i32*)

define void @test_firstprivate(i32 %len) {
; A firstprivate initialization?
; CHECK-LABEL: test_firstprivate
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool = call i8* @CsaMemAlloc(i32 20)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool = call i8* @CsaMemAlloc(i32 20)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @memset(i8* %pool, i8 0, i64 20)
  ; The memset call should be explicitly ordered after the CsaMemAlloc call.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: call void @memset(i8* %pool, i8 0, i64 20)
; CHECK-NEXT: %[[MEMSET:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]

  %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 1, i32 20)
  ; The token take call should be explicitly ordered after the memset.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMSET]])
; CHECK-NEXT: %pooli = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool, i32 1, i32 20)
; CHECK-NEXT: %[[TOKENTAKE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  store i8 0, i8* %pooli
  ; The store should be implicitly ordered after the token take.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i8 0, i8* %pooli
; CHECK-NEXT: %[[STORE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
  ; The token return should be explicitly ordered after the store.
; CHECK: call void @llvm.csa.inord(i1 %[[STORE]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool, i8* %pooli)
; CHECK-NEXT: %[[TOKENRETURN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool)
  ; The CsaMemFree should be explicitly ordered after the token return.
; CHECK: call void @llvm.csa.inord(i1 %[[TOKENRETURN]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret void
}

declare void @memset(i8*, i8, i64)

define void @test_memalloc_coalesce(i32 %len) {
; Coalesced CsaMemAlloc calls?
; CHECK-LABEL: test_memalloc_coalesce
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  %pool0 = call i8* @CsaMemAlloc(i32 320)
  ; The CsaMemAlloc call should be explicitly ordered after the mementry.
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: %pool0 = call i8* @CsaMemAlloc(i32 320)
; CHECK-NEXT: %[[CSAMEMALLOC:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pool1 = getelementptr inbounds i8, i8* %pool0, i32 160

  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]

  %pool0i = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool0, i32 8, i32 20)
  ; The first token take call should be explicitly ordered after the CsaMemAlloc.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pool0i = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool0, i32 8, i32 20)
; CHECK-NEXT: %[[TOKENTAKE0:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pool1i = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool1, i32 8, i32 20)
  ; The second token take call should be explicitly ordered after the CsaMemAlloc as well.
; CHECK: call void @llvm.csa.inord(i1 %[[CSAMEMALLOC]])
; CHECK-NEXT: %pool1i = call i8* @llvm.csa.pipeline.depth.token.take(i8* %pool1, i32 8, i32 20)
; CHECK-NEXT: %[[TOKENTAKE1:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pool0i32 = bitcast i8* %pool0i to i32*
  call void @init_array(i32* %pool0i32)
  ; This call should be explicitly ordered with the first token take, along with
  ; the second function call since we conservatively model functions as possibly
  ; touching any memory if they aren't readonly/readnone. This will be through a
  ; phi node and isn't that important for what this is testing, so just check
  ; that there is a value there.
; CHECK: %[[TAKEANDCALL0:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %{{[a-z0-9_.]+}}, i1 %[[TOKENTAKE0]])
; CHECK: call void @llvm.csa.inord(i1 %[[TAKEANDCALL0]])
; CHECK-NEXT: call void @init_array(i32* %pool0i32)
; CHECK-NEXT: %[[CALL0:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %pool1i32 = bitcast i8* %pool1i to i32*
  call void @init_array(i32* %pool1i32)
  ; This call should be explicitly ordered with the second token take,
  ; along with the first function call.
; CHECK: %[[TAKEANDCALL1:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %[[TOKENTAKE1]], i1 %[[CALL0]])
; CHECK: call void @llvm.csa.inord(i1 %[[TAKEANDCALL1]])
; CHECK-NEXT: call void @init_array(i32* %pool1i32)
; CHECK-NEXT: %[[CALL1:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool0, i8* %pool0i)
  ; The first token return should be explicitly ordered after the first call.
; CHECK: call void @llvm.csa.inord(i1 %[[CALL0]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool0, i8* %pool0i)
; CHECK-NEXT: %[[TOKENRETURN0:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  call void @llvm.csa.pipeline.depth.token.return(i8* %pool1, i8* %pool1i)
  ; The second token return should be explicitly ordered after the second call.
; CHECK: call void @llvm.csa.inord(i1 %[[CALL1]])
; CHECK-NEXT: call void @llvm.csa.pipeline.depth.token.return(i8* %pool1, i8* %pool1i)
; CHECK-NEXT: %[[TOKENRETURN1:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:

  call void @CsaMemFree(i8* %pool0)
  ; The CsaMemFree should be explicitly ordered after both token returns.
; CHECK: %[[BOTHRETURNS:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %[[TOKENRETURN0]], i1 %[[TOKENRETURN1]])
; CHECK: call void @llvm.csa.inord(i1 %[[BOTHRETURNS]])
; CHECK-NEXT: call void @CsaMemFree(i8* %pool0)
; CHECK-NEXT: %[[CSAMEMFREE:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ; The return should be explicitly ordered after the CsaMemFree, but there
  ; might be an all0 involved. Just check that the dependency token is used.
; CHECK: %[[CSAMEMFREE]]

  ret void
}

declare i8* @CsaMemAlloc(i32)
declare void @CsaMemFree(i8*)
declare i8* @llvm.csa.pipeline.depth.token.take(i8*, i32, i32)
declare void @llvm.csa.pipeline.depth.token.return(i8*, i8*)
