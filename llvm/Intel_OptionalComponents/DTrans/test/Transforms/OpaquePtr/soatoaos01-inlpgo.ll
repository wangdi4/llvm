; REQUIRES: asserts

; This test is to verify that when the pre-PGO inlining pass is run for a
; DTrans LTO compilation, the DTrans SOA-to-AOS heuristics are used during
; that inlining pass.
;
; Note: PGO data file is just a dummy file so that the PGO passes get
; inserted to the pass pipeline. The behavior being tested occurs before
; PGO feedback, so actual data values are not needed.

; The test was modified by rcox2 to also use -debug-only=inline because we
; now produce only a single inlining report per compilation.

; RUN: llvm-profdata merge %S/../Inputs/soatoaos01-inlpgo.proftext -o %t.profdata

; New pass manager
; RUN: opt -opaque-pointers -disable-output -passes="lto-pre-link<O2>" -pgo-kind=pgo-instr-use-pipeline -profile-file=%t.profdata -debug-only=inline -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain -pre-lto-inline-cost -enable-npm-dtrans %s 2>&1 | FileCheck --check-prefix=CHECK-DTRANS %s
; RUN: opt -opaque-pointers -disable-output -passes="lto-pre-link<O2>" -pgo-kind=pgo-instr-use-pipeline -profile-file=%t.profdata -debug-only=inline -inline-report=0xe807 -inline-for-xmain -pre-lto-inline-cost -enable-npm-dtrans %s  2>&1 | FileCheck --check-prefix=CHECK-INL %s

; CHECK-DTRANS-DAG: NOT Inlining (cost=never): {{.*}} @_ZN3ArrIPiEC2EiP3Mem
; CHECK-DTRANS-DAG: NOT Inlining (cost=never): {{.*}} @_ZN3ArrIPvEC2EiP3Mem
; CHECK-DTRANS-DAG: NOT Inlining (cost=never): {{.*}} @_ZN3ArrIPvE3getEi
; CHECK-DTRANS-DAG: NOT Inlining (cost=never): {{.*}} @_ZN3ArrIPiE3setEiS0_
; CHECK-DTRANS-DAG: NOT Inlining (cost=never): {{.*}} @_ZN1FC2Ev

; CHECK-INL: INLINE{{.*}}_ZN1FC2Ev
; CHECK-INL: INLINE{{.*}}_ZN3ArrIPiEC2EiP3Mem
; CHECK-INL: INLINE{{.*}}_ZN3ArrIPvEC2EiP3Mem
; CHECK-INL: INLINE{{.*}}_ZN3ArrIPiE3setEiS0_
; CHECK-INL: INLINE{{.*}}_ZN3ArrIPvE3getEi

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS1F.F = type { ptr, ptr, ptr, ptr }
%struct._ZTS3ArrIPiE.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct._ZTS3ArrIPvE.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct._ZTS3ArrIPfE.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct._ZTS3Mem.Mem = type { ptr }
%struct._ZTS4Arr1IPfE.Arr1 = type { %struct._ZTS3ArrIPfE.Arr.base, [4 x i8] }
%struct._ZTS3ArrIPfE.Arr.base = type <{ ptr, i32, [4 x i8], ptr, i32 }>

$_ZN1FC2Ev = comdat any

$_ZN3ArrIPiEC2EiP3Mem = comdat any

$_ZN3ArrIPvEC2EiP3Mem = comdat any

$_ZN4Arr1IPfEC2Ev = comdat any

$_ZN3ArrIPiE3setEiS0_ = comdat any

$_ZN3ArrIPvE3getEi = comdat any

$_ZN3ArrIPfEC2EiP3Mem = comdat any

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #0 {
entry:
  %myf = alloca %class._ZTS1F.F, align 8
  %i = bitcast ptr %myf to ptr
  call void @llvm.lifetime.start.p0(i64 32, ptr %i) #9
  call void @_ZN1FC2Ev(ptr noundef nonnull align 8 dereferenceable(32) %myf)
  call void @llvm.lifetime.end.p0(i64 32, ptr %i) #9
  ret i32 0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN1FC2Ev(ptr noundef nonnull align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %this) unnamed_addr #1 comdat align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !26 {
entry:
  %call = call noalias noundef nonnull ptr @_Znwm(i64 noundef 32) #10
  %i = bitcast ptr %call to ptr
  call void @_ZN3ArrIPiEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) %i, i32 noundef 10, ptr noundef null)
  %f1 = getelementptr inbounds %class._ZTS1F.F, ptr %this, i32 0, i32 1, !intel-tbaa !28
  store ptr %i, ptr %f1, align 8, !tbaa !28
  %call2 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 32) #10
  %i1 = bitcast ptr %call2 to ptr
  call void @_ZN3ArrIPvEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) %i1, i32 noundef 10, ptr noundef null)
  %f2 = getelementptr inbounds %class._ZTS1F.F, ptr %this, i32 0, i32 2, !intel-tbaa !36
  store ptr %i1, ptr %f2, align 8, !tbaa !36
  %call5 = call noalias noundef nonnull ptr @_Znwm(i64 noundef 32) #10
  %i2 = bitcast ptr %call5 to ptr
  call void @llvm.memset.p0.i64(ptr align 16 %call5, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(ptr noundef nonnull align 8 dereferenceable(28) %i2)
          to label %invoke.cont7 unwind label %lpad6

invoke.cont7:                                     ; preds = %entry
  %f3 = getelementptr inbounds %class._ZTS1F.F, ptr %this, i32 0, i32 3, !intel-tbaa !37
  store ptr %i2, ptr %f3, align 8, !tbaa !37
  %i3 = load ptr, ptr %f1, align 8, !tbaa !28
  call void @_ZN3ArrIPiE3setEiS0_(ptr noundef nonnull align 8 dereferenceable(28) %i3, i32 noundef 0, ptr noundef null)
  %i4 = load ptr, ptr %f2, align 8, !tbaa !36
  %call10 = call noundef ptr @_ZN3ArrIPvE3getEi(ptr noundef nonnull align 8 dereferenceable(28) %i4, i32 noundef 0)
  ret void

lpad6:                                            ; preds = %entry
  %i5 = landingpad { ptr, i32 }
          cleanup
  %i6 = extractvalue { ptr, i32 } %i5, 0
  %i7 = extractvalue { ptr, i32 } %i5, 1
  call void @_ZdlPv(ptr noundef %call5) #11
  resume { ptr, i32 } %i5
}

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !38 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) #2

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3ArrIPiEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, i32 noundef %c, ptr noundef "intel_dtrans_func_index"="2" %mem) unnamed_addr #3 comdat align 2 !intel.dtrans.func.type !40 {
entry:
  %mem2 = getelementptr inbounds %struct._ZTS3ArrIPiE.Arr, ptr %this, i32 0, i32 0, !intel-tbaa !41
  store ptr %mem, ptr %mem2, align 8, !tbaa !41
  %capacilty = getelementptr inbounds %struct._ZTS3ArrIPiE.Arr, ptr %this, i32 0, i32 1, !intel-tbaa !45
  store i32 %c, ptr %capacilty, align 8, !tbaa !45
  %base = getelementptr inbounds %struct._ZTS3ArrIPiE.Arr, ptr %this, i32 0, i32 3, !intel-tbaa !46
  store ptr null, ptr %base, align 8, !tbaa !46
  %size = getelementptr inbounds %struct._ZTS3ArrIPiE.Arr, ptr %this, i32 0, i32 4, !intel-tbaa !47
  store i32 0, ptr %size, align 8, !tbaa !47
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin nounwind
declare !intel.dtrans.func.type !48 dso_local void @_ZdlPv(ptr noundef "intel_dtrans_func_index"="1") #4

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3ArrIPvEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, i32 noundef %c, ptr noundef "intel_dtrans_func_index"="2" %mem) unnamed_addr #3 comdat align 2 !intel.dtrans.func.type !49 {
entry:
  %mem2 = getelementptr inbounds %struct._ZTS3ArrIPvE.Arr, ptr %this, i32 0, i32 0, !intel-tbaa !50
  store ptr %mem, ptr %mem2, align 8, !tbaa !50
  %capacilty = getelementptr inbounds %struct._ZTS3ArrIPvE.Arr, ptr %this, i32 0, i32 1, !intel-tbaa !53
  store i32 %c, ptr %capacilty, align 8, !tbaa !53
  %base = getelementptr inbounds %struct._ZTS3ArrIPvE.Arr, ptr %this, i32 0, i32 3, !intel-tbaa !54
  store ptr null, ptr %base, align 8, !tbaa !54
  %size = getelementptr inbounds %struct._ZTS3ArrIPvE.Arr, ptr %this, i32 0, i32 4, !intel-tbaa !55
  store i32 0, ptr %size, align 8, !tbaa !55
  ret void
}

; Function Attrs: inlinehint uwtable
define linkonce_odr dso_local void @_ZN4Arr1IPfEC2Ev(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this) unnamed_addr #5 comdat align 2 !intel.dtrans.func.type !56 {
entry:
  %i = bitcast ptr %this to ptr
  call void @_ZN3ArrIPfEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) %i, i32 noundef 1, ptr noundef null)
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN3ArrIPiE3setEiS0_(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, i32 noundef %i, ptr noundef "intel_dtrans_func_index"="2" %val) #6 comdat align 2 !intel.dtrans.func.type !57 {
entry:
  %base = getelementptr inbounds %struct._ZTS3ArrIPiE.Arr, ptr %this, i32 0, i32 3, !intel-tbaa !46
  %i1 = load ptr, ptr %base, align 8, !tbaa !46
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8, !tbaa !59
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPvE3getEi(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="2" %this, i32 noundef %i) #6 comdat align 2 !intel.dtrans.func.type !61 {
entry:
  %base = getelementptr inbounds %struct._ZTS3ArrIPvE.Arr, ptr %this, i32 0, i32 3, !intel-tbaa !54
  %i1 = load ptr, ptr %base, align 8, !tbaa !54
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx, align 8, !tbaa !62
  ret ptr %i2
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN3ArrIPfEC2EiP3Mem(ptr noundef nonnull align 8 dereferenceable(28) "intel_dtrans_func_index"="1" %this, i32 noundef %c, ptr noundef "intel_dtrans_func_index"="2" %mem) unnamed_addr #3 comdat align 2 !intel.dtrans.func.type !64 {
entry:
  %mem2 = getelementptr inbounds %struct._ZTS3ArrIPfE.Arr, ptr %this, i32 0, i32 0, !intel-tbaa !66
  store ptr %mem, ptr %mem2, align 8, !tbaa !66
  %capacilty = getelementptr inbounds %struct._ZTS3ArrIPfE.Arr, ptr %this, i32 0, i32 1, !intel-tbaa !69
  store i32 %c, ptr %capacilty, align 8, !tbaa !69
  %base = getelementptr inbounds %struct._ZTS3ArrIPfE.Arr, ptr %this, i32 0, i32 3, !intel-tbaa !70
  store ptr null, ptr %base, align 8, !tbaa !70
  %size = getelementptr inbounds %struct._ZTS3ArrIPfE.Arr, ptr %this, i32 0, i32 4, !intel-tbaa !71
  store i32 0, ptr %size, align 8, !tbaa !71
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #7

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #8

attributes #0 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { inlinehint uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { argmemonly nofree nosync nounwind willreturn }
attributes #8 = { argmemonly nofree nounwind willreturn writeonly }
attributes #9 = { nounwind }
attributes #10 = { builtin allocsize(0) }
attributes #11 = { builtin nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !10, !14, !18, !20, !22, !24}
!llvm.ident = !{!25}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %class._ZTS1F.F zeroinitializer, i32 4, !6, !7, !8, !9}
!6 = !{%struct._ZTS3Mem.Mem zeroinitializer, i32 1}
!7 = !{%struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 1}
!8 = !{%struct._ZTS3ArrIPvE.Arr zeroinitializer, i32 1}
!9 = !{%struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 1}
!10 = !{!"S", %struct._ZTS3Mem.Mem zeroinitializer, i32 1, !11}
!11 = !{!12, i32 2}
!12 = !{!"F", i1 true, i32 0, !13}
!13 = !{i32 0, i32 0}
!14 = !{!"S", %struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 6, !6, !13, !15, !17, !13, !15}
!15 = !{!"A", i32 4, !16}
!16 = !{i8 0, i32 0}
!17 = !{i32 0, i32 2}
!18 = !{!"S", %struct._ZTS3ArrIPvE.Arr zeroinitializer, i32 6, !6, !13, !15, !19, !13, !15}
!19 = !{i8 0, i32 2}
!20 = !{!"S", %struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 2, !21, !15}
!21 = !{%struct._ZTS3ArrIPfE.Arr.base zeroinitializer, i32 0}
!22 = !{!"S", %struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 6, !6, !13, !15, !23, !13, !15}
!23 = !{float 0.000000e+00, i32 2}
!24 = !{!"S", %struct._ZTS3ArrIPfE.Arr.base zeroinitializer, i32 5, !6, !13, !15, !23, !13}
!25 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!26 = distinct !{!27}
!27 = !{%class._ZTS1F.F zeroinitializer, i32 1}
!28 = !{!29, !33, i64 8}
!29 = !{!"struct@_ZTS1F", !30, i64 0, !33, i64 8, !34, i64 16, !35, i64 24}
!30 = !{!"pointer@_ZTSP3Mem", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C++ TBAA"}
!33 = !{!"pointer@_ZTSP3ArrIPiE", !31, i64 0}
!34 = !{!"pointer@_ZTSP3ArrIPvE", !31, i64 0}
!35 = !{!"pointer@_ZTSP4Arr1IPfE", !31, i64 0}
!36 = !{!29, !34, i64 16}
!37 = !{!29, !35, i64 24}
!38 = distinct !{!39}
!39 = !{i8 0, i32 1}
!40 = distinct !{!7, !6}
!41 = !{!42, !30, i64 0}
!42 = !{!"struct@_ZTS3ArrIPiE", !30, i64 0, !43, i64 8, !44, i64 16, !43, i64 24}
!43 = !{!"int", !31, i64 0}
!44 = !{!"pointer@_ZTSPPi", !31, i64 0}
!45 = !{!42, !43, i64 8}
!46 = !{!42, !44, i64 16}
!47 = !{!42, !43, i64 24}
!48 = distinct !{!39}
!49 = distinct !{!8, !6}
!50 = !{!51, !30, i64 0}
!51 = !{!"struct@_ZTS3ArrIPvE", !30, i64 0, !43, i64 8, !52, i64 16, !43, i64 24}
!52 = !{!"pointer@_ZTSPPv", !31, i64 0}
!53 = !{!51, !43, i64 8}
!54 = !{!51, !52, i64 16}
!55 = !{!51, !43, i64 24}
!56 = distinct !{!9}
!57 = distinct !{!7, !58}
!58 = !{i32 0, i32 1}
!59 = !{!60, !60, i64 0}
!60 = !{!"pointer@_ZTSPi", !31, i64 0}
!61 = distinct !{!39, !8}
!62 = !{!63, !63, i64 0}
!63 = !{!"pointer@_ZTSPv", !31, i64 0}
!64 = distinct !{!65, !6}
!65 = !{%struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 1}
!66 = !{!67, !30, i64 0}
!67 = !{!"struct@_ZTS3ArrIPfE", !30, i64 0, !43, i64 8, !68, i64 16, !43, i64 24}
!68 = !{!"pointer@_ZTSPPf", !31, i64 0}
!69 = !{!67, !43, i64 8}
!70 = !{!67, !68, i64 16}
!71 = !{!67, !43, i64 24}
