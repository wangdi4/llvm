; RUN: opt -S < %s -passes='hir-ssa-deconstruction,hir-vplan-vec' -vplan-force-vf=2 -disable-output \
; RUN:   -vplan-print-after-init -vplan-dump-known-bits 2>&1 | FileCheck %s

; CHECK: i64* {{%.*}} = subscript i64* %array i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i32* {{%.*}} = subscript i32* null i64 4 {{.*}} KnownBits: {Zero=-17, One=16}
; CHECK: i8* {{%.*}} = subscript i8* null i64 4 {{.*}} KnownBits: {Zero=-5, One=4}
; CHECK: i8* {{%.*}} = subscript { i8, i8 }* null i64 2 (1 ) {{.*}} KnownBits: {Zero=-6, One=5}
; CHECK: i8* {{%.*}} = subscript { i8, i8 }* null i64 {{%.*}} (1 ) {{.*}} KnownBits: {Zero=0, One=1}
; CHECK: i8* {{%.*}} = subscript inbounds i8* null i64 {{%.*}} KnownBits: {Zero=15, One=0}
; CHECK: i8* {{%.*}} = subscript inbounds i8* null i64 {{%.*}} KnownBits: {Zero=3, One=0}
; CHECK: i64* {{%.*}} = subscript inbounds i64* null i64 {{.*}} KnownBits: {Zero=127, One=0}
; CHECK: i64* {{%.*}} = subscript inbounds i64* null i64 {{.*}} KnownBits: {Zero=31, One=0}
; CHECK: i64* {{%.*}} = subscript i64* {{%.*}} i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i8* {{%.*}} = subscript i8* null i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i8* {{%.*}} = subscript inbounds i8* null i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i64* {{%.*}} = subscript inbounds i64* {{%.*}} i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i64* {{%.*}} = subscript inbounds i64* null i64 {{%.*}} {{.*}} KnownBits: {Zero=0, One=0}
; CHECK: i64* {{%.*}} = subscript [256 x i64]* null i64 0 i64 0 {{.*}} KnownBits: {Zero=-1, One=0}
; CHECK: i64* {{%.*}} = subscript [256 x i64]* null i64 0 i64 {{.*}} KnownBits: {Zero=7, One=0}
; CHECK: i64* {{%.*}} = subscript [256 x i64]* null i64 {{%.*}} i64 0 {{.*}} KnownBits: {Zero=2047, One=0}

define void @foo(i64 *%array) {
entry:
  br label %for.ph

for.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %add1, %for.body ]

  %gep = getelementptr i64, i64* %array, i64 %iv
  %a = load i64, i64* %gep, align 8
  %a.mul2 = mul i64 %a, 2
  %a.mul8 = mul i64 %a, 8

  ; Aligned GEPs with LB = 0 && Stride = sizeof(elemtype)
  ; Legend = sub.<type>[.<offset>]+
  %sub.i32.4 = getelementptr i32, i32* null, i64 4
  store i32 1, i32* %sub.i32.4, align 1
  %sub.i8.4 = getelementptr i8, i8* null, i64 4
  store i8 2, i8* %sub.i8.4, align 1
  %sub.i8i8.2.1 = getelementptr {i8, i8}, {i8, i8}* null, i64 2, i32 1
  store i8 3, i8* %sub.i8i8.2.1, align 1
  %sub.i8i8.a.1 = getelementptr {i8, i8}, {i8, i8}* null, i64 %a, i32 1
  store i8 4, i8* %sub.i8i8.a.1, align 1

  ; Aligned subscripts with LB =/= 0 || Stride =/= sizeof(elemtype)
  ; sub.<type>.<lb>.<stride>.<index>
  %sub.i8.0.2.8 = call i8* @llvm.intel.subscript.p0i8.i64.i32.p0i8.i64(i8 0, i64 0, i32 2, i8* elementtype(i8) null, i64 %a.mul8)
  store i8 5, i8* %sub.i8.0.2.8, align 8
  %sub.i8.2.2.8 = call i8* @llvm.intel.subscript.p0i8.i64.i32.p0i8.i64(i8 0, i64 %a.mul2, i32 2, i8* elementtype(i8) null, i64 %a.mul8)
  store i8 6, i8* %sub.i8.2.2.8, align 8
  %sub.i32.0.16.8 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i64(i8 0, i64 0, i32 16, i64* elementtype(i64) null, i64 %a.mul8)
  store i64 7, i64* %sub.i32.0.16.8, align 8
  %sub.i32.2.16.8 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i64(i8 0, i64 %a.mul2, i32 16, i64* elementtype(i64) null, i64 %a.mul8)
  store i64 8, i64* %sub.i32.2.16.8, align 8
  %sub.i32.8.a.a = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %a.mul8, i64 %a, i64* elementtype(i64) null, i64 %a)
  store i64 9, i64* %sub.i32.8.a.a, align 8

  ; Negative cases (unaligned GEPs and subscripts)
  %sub.unk.1 = getelementptr i64, i64* %gep, i64 %a.mul8
  store i64 10, i64* %sub.unk.1, align 8
  %sub.unk.2 = getelementptr i8, i8* null, i64 %a
  store i8 11, i8* %sub.unk.2, align 1
  %sub.unk.3 = call i8* @llvm.intel.subscript.p0i8.i64.i32.p0i8.i64(i8 0, i64 %a, i32 1, i8* elementtype(i8) null, i64 %a.mul8)
  store i8 12, i8* %sub.unk.3, align 8
  %sub.unk.4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i64(i8 0, i64 %a.mul8, i32 8, i64* elementtype(i64) %gep, i64 %a.mul8)
  store i64 13, i64* %sub.unk.4, align 8
  %sub.unk.5 = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 %a.mul8, i64 %a, i64* elementtype(i64) null, i64 %a)
  store i64 14, i64* %sub.unk.5, align 8

  ; Rank > 1 with non-ptr dimension type
  %sub.256xi64.1 = getelementptr [256 x i64], [256 x i64]* null, i64 0, i64 0
  store i64 15, i64* %sub.256xi64.1, align 8
  %sub.256xi64.2 = getelementptr [256 x i64], [256 x i64]* null, i64 0, i64 %iv
  store i64 16, i64* %sub.256xi64.2, align 8
  %sub.256xi64.3 = getelementptr [256 x i64], [256 x i64]* null, i64 %iv, i64 0
  store i64 17, i64* %sub.256xi64.3, align 8

  %add1 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add1, 1024
  br i1 %exitcond.not, label %omp.simd.end, label %for.body

omp.simd.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %ret

ret:
  ret void
}

declare i8* @llvm.intel.subscript.p0i8.i64.i32.p0i8.i64(i8, i64, i32, i8* elementtype(i8), i64) #0
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i64(i8, i64, i32, i64* elementtype(i64), i64) #0
declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, i64* elementtype(i64), i64) #0

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind readnone speculatable }
