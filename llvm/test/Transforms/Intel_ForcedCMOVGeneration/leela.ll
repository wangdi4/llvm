; Source code
;
; #include <boost/tr1/array.hpp>
;
; static const int MAXBOARDSIZE = 19;
; static const int MAXSQ = ((MAXBOARDSIZE + 2) * (MAXBOARDSIZE + 2));
; enum square_t {
;    BLACK = 0, WHITE = 1, EMPTY = 2, INVAL = 3
; };
; std::tr1::array<unsigned short, MAXSQ+1> m_libs;
; std::tr1::array<int, 4>                  m_dirs;
; std::tr1::array<square_t,  MAXSQ>        m_square;
; std::tr1::array<unsigned short, MAXSQ+1> m_parent;
;
; std::pair<int, int> nbr_criticality(int color, int vertex) {
;     std::tr1::array<int, 4> color_libs;
;     color_libs[0] = 8;
;     color_libs[1] = 8;
;     color_libs[2] = 8;
;     color_libs[3] = 8;
;     for (int k = 0; k < 1; k++) {
;         int ai = vertex + m_dirs[k];
;         int lc = m_libs[m_parent[ai]];
;         if (lc < color_libs[m_square[ai]])
;         {
;             color_libs[m_square[ai]] = lc;
;         }
;     }
;     return std::make_pair(color_libs[color], color_libs[!color]);
; }
;
; RUN: opt -passes="forced-cmov-generation" -aa-pipeline="basic-aa" -S 2>&1 < %s | FileCheck %s
; CHECK:  [[DUMMY:%[0-9]+]] = alloca i32
; CHECK:  [[ADDR:%[0-9]+]] = select i1 %cmp14, ptr %arrayidx.i35, ptr [[DUMMY]]
; CHECK-DAG: store i32 %conv9, ptr [[ADDR]], align 4
; CHECK-DAG: br label %if.end
; CHECK-DAG: if.end:
;
; ModuleID = 'test_orig_2.cpp'
source_filename = "test_orig_2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.boost::array" = type { [442 x i16] }
%"class.boost::array.0" = type { [4 x i32] }
%"class.boost::array.1" = type { [441 x i32] }

@m_libs = dso_local local_unnamed_addr global %"class.boost::array" zeroinitializer, align 2
@m_dirs = dso_local local_unnamed_addr global %"class.boost::array.0" zeroinitializer, align 4
@m_square = dso_local local_unnamed_addr global %"class.boost::array.1" zeroinitializer, align 4
@m_parent = dso_local local_unnamed_addr global %"class.boost::array" zeroinitializer, align 2
; Function Attrs: nounwind readonly uwtable
define dso_local i64 @_Z15nbr_criticalityii(i32 %color, i32 %vertex) local_unnamed_addr #0 {
entry:
  %color_libs = alloca <4 x i32>, align 16
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %color_libs) #2
  store <4 x i32> <i32 8, i32 8, i32 8, i32 8>, ptr %color_libs, align 16
  %0 = load i32, ptr @m_dirs, align 4
  %add = add nsw i32 %0, %vertex
  %conv5 = sext i32 %add to i64
  %arrayidx.i38 = getelementptr inbounds %"class.boost::array", ptr @m_parent, i64 0, i32 0, i64 %conv5
  %1 = load i16, ptr %arrayidx.i38, align 2
  %conv7 = zext i16 %1 to i64
  %arrayidx.i37 = getelementptr inbounds %"class.boost::array", ptr @m_libs, i64 0, i32 0, i64 %conv7
  %2 = load i16, ptr %arrayidx.i37, align 2
  %conv9 = zext i16 %2 to i32
  %arrayidx.i36 = getelementptr inbounds %"class.boost::array.1", ptr @m_square, i64 0, i32 0, i64 %conv5
  %3 = load i32, ptr %arrayidx.i36, align 4
  %conv12 = zext i32 %3 to i64
  %arrayidx.i35 = getelementptr inbounds %"class.boost::array.0", ptr %color_libs, i64 0, i32 0, i64 %conv12
  %4 = load i32, ptr %arrayidx.i35, align 4
  %cmp14 = icmp sgt i32 %4, %conv9
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 %conv9, ptr %arrayidx.i35, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %conv19 = sext i32 %color to i64
  %arrayidx.i41 = getelementptr inbounds %"class.boost::array.0", ptr %color_libs, i64 0, i32 0, i64 %conv19
  %5 = load i32, ptr %arrayidx.i41, align 4
  %tobool = icmp eq i32 %color, 0
  %conv21 = zext i1 %tobool to i64
  %arrayidx.i40 = getelementptr inbounds %"class.boost::array.0", ptr %color_libs, i64 0, i32 0, i64 %conv21
  %6 = load i32, ptr %arrayidx.i40, align 4
  %retval.sroa.2.0.insert.ext.i = zext i32 %6 to i64
  %retval.sroa.2.0.insert.shift.i = shl nuw i64 %retval.sroa.2.0.insert.ext.i, 32
  %retval.sroa.0.0.insert.ext.i = zext i32 %5 to i64
  %retval.sroa.0.0.insert.insert.i = or i64 %retval.sroa.2.0.insert.shift.i, %retval.sroa.0.0.insert.ext.i
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %color_libs) #2
  ret i64 %retval.sroa.0.0.insert.insert.i
}
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1
; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
