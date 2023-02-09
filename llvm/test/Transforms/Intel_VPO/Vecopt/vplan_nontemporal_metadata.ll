; RUN: opt -opaque-pointers=0 -S -passes=vplan-vec < %s | FileCheck %s --match-full-lines

target triple = "x86_64-unknown-linux-gnu"

; CHECK: define void @test_ntmp_store_01(i64* nocapture %ary) {
define void @test_ntmp_store_01(i64* nocapture %ary) {
entry:
  %ptrint = ptrtoint i64* %ary to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  br label %region

region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %region ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  ; CHECK: store <2 x i64> %{{.*}}, <2 x i64>* %{{.*}}, align 16, !nontemporal !0
  store i64 %indvars.iv, i64* %ptr, align 8, !nontemporal !0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; CHECK: define void @test_ntmp_store_02(i64* nocapture %ary) {
define void @test_ntmp_store_02(i64* nocapture %ary) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  ; CHECK: store <2 x i64> %{{.*}}, <2 x i64>* %{{.*}}, align 8
  store i64 %indvars.iv, i64* %ptr, align 8, !nontemporal !0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

!0 = !{i32 1}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1)
