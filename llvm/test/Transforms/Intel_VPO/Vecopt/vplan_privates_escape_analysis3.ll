; This test verifies that private-variables escaping into unknown function
; via an intermediate write is captured as 'unsafe'. We also test a scenario where
; we capture a private-variable as unsafe on account of an intermediate bitcast,
; which is loop-invariant.

; RUN: opt -VPlanDriver -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s
; TODO: Enbale the test for HIR codegen path CMPLRLLVM-10967.

; REQUIRES:asserts

; CHECK-DAG: SOAUnsafe = arr_e.priv
; CHECK-DAG: SOAUnsafe = arr_e2.priv
; CHECK-DAG: SOASafe = ptr.priv
; CHECK-DAG: SOASafe = index.lpriv


;Source Code: test2.c
;int arr_e[1024];
;extern int helper(int *elem);
;extern int helper2(void);
;int foo(int n1, int k) {
;  int index;
;#pragma omp simd private(arr_e)
;  for (index = 1; index < 1024; index++) {
;    int *ptr = &arr_e[index];
;    ptr[k] = helper2();
;    helper(&ptr[k]);
;  }
;  return arr_e[index-k];
;}

; Compile-command: icx test2.c -o out.ll -fiopenmp -O1 -S  \
; -mllvm -disable-vplan-codegen -mllvm -vplan-entities-dump \
; -mllvm -vplan-use-entity-instr -emit-llvm


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_e = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 %n1, i32 %k) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %ptr.priv = alloca i32*, align 8
  %arr_e.priv = alloca [1024 x i32], align 4
  %arr_e2.priv = alloca [1024 x i32], align 4
  %index.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv, [1024 x i32]* %arr_e2.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %index.lpriv), "QUAL.OMP.PRIVATE"(i32** %ptr.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32** %ptr.priv to i8*
  %idxprom1 = sext i32 %k to i64
  %bc1 = bitcast [1024 x i32]* %arr_e2.priv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  store i8 10, i8* %bc1, align 1            ;; Should cause arr_e2 to be marked as unsafe.
  %2 = trunc i64 %indvars.iv.next to i32
  store i32 %2, i32* %index.lpriv, align 4
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1)
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 %indvars.iv.next
  store i32* %arrayidx, i32** %ptr.priv, align 8
  %call = call i32 @helper2()
  %3 = load i32*, i32** %ptr.priv, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %3, i64 %idxprom1
  store i32 %call, i32* %arrayidx2, align 4
  %call5 = call i32 @helper(i32* %arrayidx2)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1)
  %exitcond = icmp eq i64 %indvars.iv.next, 1023
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %4 = load i32, i32* %index.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %sub = sub nsw i32 %4, %k
  %idxprom7 = sext i32 %sub to i64
  %arrayidx8 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr_e, i64 0, i64 %idxprom7
  %5 = load i32, i32* %arrayidx8, align 4
  ret i32 %5
}


; This test makes sure that loads/stores of bitcasted privates or their aliases are marked as unsafe.

; CHECK-DAG: SOASafe = arr_e1.priv
; CHECK-DAG: SOAUnsafe = arr_e2.priv
; CHECK-DAG: SOAUnsafe = arr_e3.priv
; CHECK-DAG: SOAUnsafe = arr_e4.priv
; CHECK-DAG: SOAUnsafe = arr_e5.priv


;; Function Attrs: nounwind
define void @test_bitcast() {
  %arr_e1.priv = alloca [624 x i32], align 4
  %arr_e2.priv = alloca [624 x i32], align 4
  %arr_e3.priv = alloca [624 x i32], align 4
  %arr_e4.priv = alloca [624 x i32], align 4
  %arr_e5.priv = alloca [624 x i32], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([624 x i32]* %arr_e1.priv, [624 x i32]* %arr_e2.priv, [624 x i32]* %arr_e3.priv, [624 x i32]* %arr_e4.priv, [624 x i32]* %arr_e5.priv) ]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %se = sext i32 %index to i64
  %add = add nuw i64 %se, 1
  %bc1 = bitcast [624 x i32]* %arr_e1.priv to i8*
  %bc2 = bitcast [624 x i32]* %arr_e2.priv to i8*
  %bc3 = bitcast [624 x i32]* %arr_e3.priv to i8*
  %bc4 = bitcast [624 x i32]* %arr_e4.priv to i8*
  %bc5 = bitcast [624 x i32]* %arr_e5.priv to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc1)
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc2)
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc3)
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc4)
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc5)
  %g1 = getelementptr inbounds i8, i8* %bc2, i64 3
  %t_bc2 = bitcast i8* %g1 to i64*
  %l2 = load i64, i64* %t_bc2, align 1      ;; Should cause arr_e2 to be marked as unsafe.
  store i8 10, i8* %bc3, align 1            ;; Should cause arr_e3 to be marked as unsafe.
  %add1 = trunc i64 %add to i32
  %gep = getelementptr inbounds [624 x i32], [624 x i32]* %arr_e1.priv, i64 0, i64 0
  store i32 %add1, i32* %gep, align 4
  %gep1 = getelementptr inbounds [624 x i32], [624 x i32]* %arr_e4.priv, i64 0, i64 0
  %gep2 = getelementptr inbounds [624 x i32], [624 x i32]* %arr_e4.priv, i64 0, i64 1
  %vec1 = insertelement <2 x i32*> undef, i32* %gep1, i32 0
  %vec2 = insertelement <2 x i32*> %vec1, i32* %gep2, i32 1
  %call = call i32 @helper3(<2 x i32*> %vec2)
  ;; This private array does not escape. The following insert/extract-element sequence is safe.
  ;; However, input code might have random instructions (e.g., shuffle,select, etc.), manipulating
  ;; this vector in ways which we cannot reason about statically. Hence, we conservatively mark these
  ;; as un-safe.
  %gep3 = getelementptr inbounds [624 x i32], [624 x i32]* %arr_e5.priv, i64 0, i64 0
  %gep4 = getelementptr inbounds [624 x i32], [624 x i32]* %arr_e5.priv, i64 0, i64 1
  %vec3 = insertelement <2 x i32*> undef, i32* %gep3, i32 0
  %vec4 = insertelement <2 x i32*> %vec1, i32* %gep4, i32 1
  %E1 = extractelement <2 x i32*> %vec4, i32 1
  %t_bc3 = bitcast i32* %E1 to i64*
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc1)
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc2)
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc3)
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc4)
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc5)
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}; Test that load/store from bitcast's are identified as unsafe.

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local i32 @helper(i32*) local_unnamed_addr

declare dso_local i32 @helper2() local_unnamed_addr

declare dso_local i32 @helper3(<2 x i32*>) local_unnamed_addr

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)


; Test that possible pointer-escape via indirect call is conservatively reported as
; SOA-unsafe. This test also checks that unsupported types are outrightly rejected.

; CHECK-DAG: SOASafe = ptr.priv
; CHECK-DAG: SOAUnsafe = arr_e.priv
; CHECK-DAG: SOASafe = arr_e1.priv
; CHECK-DAG: SOAUnsafe = struct.priv
; CHECK-DAG: SOAUnsafe = arr.struct.priv
; CHECK-DAG: SOASafe = arr.struct.ptr.priv
; CHECK-DAG: SOAUnsafe = vec.priv
; CHECK-DAG: SOAUnsafe = arr.vec.priv

define void @test_negative(i32 (i32 *)** nocapture readonly %p2) {
  %ptr.priv = alloca i32*, align 8
  %arr_e.priv = alloca [624 x i32], align 4
  %arr_e1.priv = alloca [624 x i32*], align 4
  %struct.priv = alloca {i32, [624 x i32]}, align 4
  %arr.struct.priv = alloca [100 x {i32}], align 4
  %arr.struct.ptr.priv = alloca [100 x {i32}*], align 4
  %vec.priv = alloca <4 x i32>, align 4
  %arr.vec.priv = alloca [100 x <4 x i32>], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32** %ptr.priv, [624 x i32]* %arr_e.priv, [624 x i32*]* %arr_e1.priv, {i32, [624 x i32]}* %struct.priv, [100 x {i32}]* %arr.struct.priv, [100 x {i32}*]* %arr.struct.ptr.priv, <4 x i32>* %vec.priv, [100 x <4 x i32>]* %arr.vec.priv) ]
  br label %simd.loop

simd.loop:
  %index = phi i64 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %bc1 = bitcast [624 x i32]* %arr_e.priv to i8*
  call void @llvm.lifetime.start.p0i8(i64 2496, i8* nonnull %bc1)
  %arrayidx = getelementptr inbounds i32 (i32 *)*, i32 (i32 *)** %p2, i64 %index
  %ld = load i32 (i32*) *, i32 (i32*)** %arrayidx, align 8
  %bc2 = bitcast [624 x i32]* %arr_e.priv to i32*
  %call = call i32 %ld(i32* %bc2)
  call void @llvm.lifetime.end.p0i8(i64 2496, i8* nonnull %bc1)
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

; This test checks that we do not enter a tight infinite loop on account of PHI instructions
; and its users.
; CHECK-DAG: SOAUnsafe = arr_e.priv
define void @test_pointer_induction_escape() {
  %arr_e.priv = alloca [1024 x i32], align 4
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 0
  %arrayidx.end = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 1023
  %as.cast1 = addrspacecast i32* %arrayidx.end to i32 addrspace(4)*
  %ptr2int1 = ptrtoint i32 addrspace(4)* %as.cast1 to i64
  br label %simd.begin.region
simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv) ]
  br label %simd.loop
simd.loop:
  %arrayidx.current = phi i32* [ %arrayidx, %simd.begin.region], [%arrayidx.next, %simd.loop.end]
  %ld = load i32, i32* %arrayidx.current
  br label %simd.loop.end
simd.loop.end:
  %as.cast2 = addrspacecast i32* %arrayidx.current to i32 addrspace(4)*
  %ptr2int2 = ptrtoint i32 addrspace(4)* %as.cast2 to i64
  %icmp = icmp ult i64 %ptr2int2, %ptr2int1
  %arrayidx.next = getelementptr inbounds i32, i32* %arrayidx.current, i64 1
  br i1 %icmp, label %simd.end.region, label %simd.loop

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.end
for.end:
  ret void
}
