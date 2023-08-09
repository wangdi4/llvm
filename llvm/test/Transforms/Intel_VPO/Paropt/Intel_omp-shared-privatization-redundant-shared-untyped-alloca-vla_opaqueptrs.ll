; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-shared-privatization -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-shared-privatization)' -S <%s 2>&1 | FileCheck %s

; CHECK:  "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(ptr null), "QUAL.OMP.SHARED"(ptr null), "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, [10 x i64] zeroinitializer, i64 %0), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr, i64 0, i64 150)

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @_Z1ai(i32 noundef %n) local_unnamed_addr #0 {
DIR.OMP.PARALLEL.33:
  %omp.vla.tmp = alloca i64, align 8
  %0 = zext i32 %n to i64
  %vla = alloca [10 x i64], i64 %0, align 16
  %arr = alloca [10 x i64], i64 15, align 16
  store i64 %0, ptr %omp.vla.tmp, align 8
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %DIR.OMP.PARALLEL.33
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(ptr %vla),
    "QUAL.OMP.SHARED"(ptr %arr) ]
  br label %DIR.OMP.PARALLEL.35

DIR.OMP.PARALLEL.35:                              ; preds = %DIR.OMP.PARALLEL.2
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.5, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.35
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.PARALLEL.35, %DIR.OMP.PARALLEL.3
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.56

DIR.OMP.END.PARALLEL.56:                          ; preds = %DIR.OMP.END.PARALLEL.4
  ret i32 undef
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
