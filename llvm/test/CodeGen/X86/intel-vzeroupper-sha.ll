; Check that VZEROUPPER instructions are inserted before SHA instructions in
; functions compiled to AVX targets whenever possible
; RUN: llc < %s -mattr=+sha,+avx2 -mtriple=x86_64-unknown-unknown | FileCheck %s

; YMMs are used before SHA instruction. The YMMs are dead at the SHA
; instruction and a VZEROUPPER is inserted.
define void @test_ymm_before_sha(ptr %src1, ptr %src2, ptr %dst1, ptr %dst2, i64 %n) nounwind uwtable {
; CHECK-LABEL: test_ymm_before_sha:
; CHECK: vzeroupper
; CHECK-NEXT: sha256msg1
entry:
  br label %loop

loop:
  %result = phi <8 x i32> [ zeroinitializer, %entry ], [ %result.next, %loop ]
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %loop ]

  %gep.src1 = getelementptr inbounds i32, ptr %src1, i64 %iv
  %gepload.src1 = load <8 x i32>, ptr %gep.src1, align 4
  %result.next = add <8 x i32> %gepload.src1, %result

  %gep.src2 = getelementptr inbounds i32, ptr %src2, i64 %iv
  %gepload.src2 = load <8 x i32>, ptr %gep.src2, align 4
  %result2 = add <8 x i32> %gepload.src1, %gepload.src2
  %gep.dst = getelementptr inbounds i32, ptr %dst1, i64 %iv
  store <8 x i32> %result2, ptr %gep.dst

  %iv.next = add nuw nsw i64 %iv, 8
  %condloop = icmp sgt i64 %iv.next, %n
  br i1 %condloop, label %sha, label %loop

sha:
  %result.i32 = tail call i32 @llvm.vector.reduce.add.v8i32(<8 x i32> %result.next)
  %dst2.ptr2 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 1
  store i32 %result.i32, ptr %dst2.ptr2

  %sha.src1 = load <4 x i32>, ptr %src1
  %sha.src2 = load <4 x i32>, ptr %src2
  %result.sha = tail call <4 x i32> @llvm.x86.sha256msg1(<4 x i32> %sha.src1, <4 x i32> %sha.src2)
  %dst2.ptr1 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 0
  store <4 x i32> %result.sha, ptr %dst2.ptr1
  ret void
}

; A YMM register lives through the SHA instruction. Inserting an VZEROUPPER is
; incorrect in this case.
define void @test_ymm_lives_through_sha(ptr %src1, ptr %src2, ptr %dst1, ptr %dst2, i64 %n) nounwind uwtable {
; CHECK-LABEL: test_ymm_lives_through_sha:
; CHECK: vzeroupper
; CHECK-NOT: sha256msg1
; CHECK-NEXT: ret
entry:
  br label %loop

loop:
  %result = phi <8 x i32> [ zeroinitializer, %entry ], [ %result.next, %loop ]
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %loop ]

  %gep.src1 = getelementptr inbounds i32, ptr %src1, i64 %iv
  %gepload.src1 = load <8 x i32>, ptr %gep.src1, align 4
  %result.next = add <8 x i32> %gepload.src1, %result

  %gep.src2 = getelementptr inbounds i32, ptr %src2, i64 %iv
  %gepload.src2 = load <8 x i32>, ptr %gep.src2, align 4
  %result2 = add <8 x i32> %gepload.src1, %gepload.src2
  %gep.dst = getelementptr inbounds i32, ptr %dst1, i64 %iv
  store <8 x i32> %result2, ptr %gep.dst

  %iv.next = add nuw nsw i64 %iv, 8
  %condloop = icmp sgt i64 %iv.next, %n
  br i1 %condloop, label %sha, label %loop

sha:
  %sha.src1 = load <4 x i32>, ptr %src1
  %sha.src2 = load <4 x i32>, ptr %src2
  %result.sha = tail call <4 x i32> @llvm.x86.sha256msg1(<4 x i32> %sha.src1, <4 x i32> %sha.src2)
  %dst2.ptr1 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 0
  store <4 x i32> %result.sha, ptr %dst2.ptr1

  %result.i32 = tail call i32 @llvm.vector.reduce.add.v8i32(<8 x i32> %result.next)
  %dst2.ptr2 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 1
  store i32 %result.i32, ptr %dst2.ptr2
  ret void
}

; YMM registers are used both before and after the SHA instruction. VZEROUPPER
; insertion is valid because no YMM lives through it.
define void @test_ymm_after_sha(ptr %src1, ptr %src2, ptr %dst1, ptr %dst2, i64 %n) nounwind uwtable {
; CHECK-LABEL: test_ymm_after_sha:
; CHECK: vzeroupper
; CHECK-NEXT: sha256msg1
; CHECK: vzeroupper
; CHECK-NEXT: ret
entry:
  br label %loop

loop:
  %result = phi <8 x i32> [ zeroinitializer, %entry ], [ %result.next, %loop ]
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %loop ]

  %gep.src1 = getelementptr inbounds i32, ptr %src1, i64 %iv
  %gepload.src1 = load <8 x i32>, ptr %gep.src1, align 4
  %result.next = add <8 x i32> %gepload.src1, %result

  %gep.src2 = getelementptr inbounds i32, ptr %src2, i64 %iv
  %gepload.src2 = load <8 x i32>, ptr %gep.src2, align 4
  %result2 = add <8 x i32> %gepload.src1, %gepload.src2
  %gep.dst = getelementptr inbounds i32, ptr %dst1, i64 %iv
  store <8 x i32> %result2, ptr %gep.dst

  %iv.next = add nuw nsw i64 %iv, 8
  %condloop = icmp sgt i64 %iv.next, %n
  br i1 %condloop, label %sha, label %loop

sha:
  %result.i32 = tail call i32 @llvm.vector.reduce.add.v8i32(<8 x i32> %result.next)
  %dst2.ptr2 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 1
  store i32 %result.i32, ptr %dst2.ptr2

  %sha.src1 = load <4 x i32>, ptr %src1
  %sha.src2 = load <4 x i32>, ptr %src2
  %result.sha = tail call <4 x i32> @llvm.x86.sha256msg1(<4 x i32> %sha.src1, <4 x i32> %sha.src2)
  %dst2.ptr1 = getelementptr inbounds { <4 x i32>, i32 }, ptr %dst2, i32 0, i32 0
  store <4 x i32> %result.sha, ptr %dst2.ptr1

  %broadcast.insert = insertelement <8 x i32> undef, i32 %result.i32, i32 0
  %broadcast = shufflevector <8 x i32> %broadcast.insert, <8 x i32> undef, <8 x i32> zeroinitializer
  br label %loop.broadcast

loop.broadcast:
  %iv.broadcast = phi i64 [ 0, %sha ], [ %iv.next, %loop.broadcast ]

  %gep.broadcast = getelementptr inbounds i32, ptr %src2, i64 %iv
  store <8 x i32> %broadcast, ptr %gep.broadcast

  %iv.broadcast.next = add nuw nsw i64 %iv.broadcast, 8
  %condloop.broadcast = icmp sgt i64 %iv.broadcast.next, %n
  br i1 %condloop.broadcast, label %return, label %loop.broadcast

return:
  ret void
}

declare <4 x i32> @llvm.x86.sha256msg1(<4 x i32>, <4 x i32>) nounwind readnone
declare i32 @llvm.vector.reduce.add.v8i32(<8 x i32>) nocallback nofree nosync nounwind willreturn memory(none)
