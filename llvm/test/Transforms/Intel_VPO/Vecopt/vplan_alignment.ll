; RUN: opt -S -passes=vplan-vec -vplan-enable-peeling=false < %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: test_01
define void @test_01(ptr nocapture %ary) {
entry:
  %ptrint = ptrtoint ptr %ary to i64
  %maskedptr = and i64 %ptrint, 15
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  br label %region

region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %region ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, ptr %ary, i64 %indvars.iv
  ; Alignment of widened instruction is inferred from the @llvm.assume above.
  ; CHECK: store <4 x i64> %{{.*}}, ptr %{{.*}}, align 16
  store i64 %indvars.iv, ptr %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; CHECK-LABEL: test_02
define void @test_02(ptr nocapture %ary) {
entry:
  %ptrint = ptrtoint ptr %ary to i64
  %maskedptr = and i64 %ptrint, 1023
  %maskcond = icmp eq i64 %maskedptr, 0
  call void @llvm.assume(i1 %maskcond)
  br label %region

region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %region ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, ptr %ary, i64 %indvars.iv
  ; Alignment of widened instruction is limited by VF.
  ; CHECK: store <4 x i64> %{{.*}}, ptr %{{.*}}, align 32
  store i64 %indvars.iv, ptr %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; CHECK-LABEL: test_03
define void @test_03(ptr nocapture %ary) {
region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %region ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, ptr %ary, i64 %indvars.iv
  ; Alignment of the widened instruction is no less than alignment of the
  ; underlying instruction.
  ; CHECK: store <4 x i64> %{{.*}}, ptr %{{.*}}, align 8
  store i64 %indvars.iv, ptr %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1)
