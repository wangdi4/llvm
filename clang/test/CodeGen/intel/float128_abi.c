// RUN: %clang_cc1 -triple -x86_64-linux-gnu %s -emit-llvm -o - | FileCheck %s -check-prefix=LINUX
// RUN: %clang_cc1 -triple -x86_64-windows-msvc -fintel-compatibility %s --extended_float_types -emit-llvm -o - | FileCheck %s -check-prefix=WINDOWS

__float128 foo(__float128 x){
// LINUX: define fp128 @foo(fp128 [[X:%.*]])
// LINUX-NEXT: entry:
// LINUX-NEXT:  [[X_ADDR:%.*]] = alloca fp128, align 16{{$}}
// LINUX-NEXT:  store fp128 [[X]], fp128* [[X_ADDR]], align 16{{$}}
// LINUX-NEXT:  [[TMP0:%.*]] = load fp128, fp128* [[X_ADDR]], align 16{{$}}
// LINUX-NEXT:  ret fp128 [[TMP0]]

// WINDOWS: define dso_local void @foo(fp128* noalias sret [[AGG_RESULT:%.*]], fp128* [[TMP0:%.*]])
// WINDOWS-NEXT: entry:
// WINDOWS-NEXT:  [[RESULT_PTR:%.*]] = alloca i8*, align 8{{$}}
// WINDOWS-NEXT:  [[X_ADDR:%.*]] = alloca fp128, align 16{{$}}
// WINDOWS-NEXT:  [[TMP1:%.*]] = bitcast fp128* [[AGG_RESULT]] to i8*
// WINDOWS-NEXT:  store i8* [[TMP1]], i8** [[RESULT_PTR]], align 8{{$}}
// WINDOWS-NEXT:  [[X:%.*]] = load fp128, fp128* [[TMP0]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[X]], fp128* [[X_ADDR]], align 16{{$}}
// WINDOWS-NEXT:  [[TMP2:%.*]] = load fp128, fp128* [[X_ADDR]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[TMP2]], fp128* [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  [[TMP3:%.*]] = load fp128, fp128* [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[TMP3]], fp128* [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  ret void

  return x;
}

