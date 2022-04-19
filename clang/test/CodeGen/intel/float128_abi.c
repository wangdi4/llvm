// RUN: %clang_cc1 -triple -x86_64-linux-gnu %s -opaque-pointers -emit-llvm -o - | FileCheck %s -check-prefix=LINUX
// RUN: %clang_cc1 -triple -x86_64-windows-msvc -fintel-compatibility %s --extended_float_types -emit-llvm -opaque-pointers -o - | FileCheck %s -check-prefix=WINDOWS

__float128 foo(__float128 x){
// LINUX: define{{.*}}fp128 @foo(fp128 noundef [[X:%.*]])
// LINUX-NEXT: entry:
// LINUX-NEXT:  [[X_ADDR:%.*]] = alloca fp128, align 16{{$}}
// LINUX-NEXT:  store fp128 [[X]], ptr [[X_ADDR]], align 16{{$}}
// LINUX-NEXT:  [[TMP0:%.*]] = load fp128, ptr [[X_ADDR]], align 16{{$}}
// LINUX-NEXT:  ret fp128 [[TMP0]]

// WINDOWS: define dso_local void @foo(ptr noalias sret(fp128) {{(align 16 )?}}[[AGG_RESULT:%.*]], ptr noundef [[TMP0:%.*]])
// WINDOWS-NEXT: entry:
// WINDOWS-NEXT:  [[RESULT_PTR:%.*]] = alloca ptr, align 8{{$}}
// WINDOWS-NEXT:  [[X_ADDR:%.*]] = alloca fp128, align 16{{$}}
// WINDOWS-NEXT:  store ptr [[AGG_RESULT]], ptr [[RESULT_PTR]], align 8{{$}}
// WINDOWS-NEXT:  [[X:%.*]] = load fp128, ptr [[TMP0]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[X]], ptr [[X_ADDR]], align 16{{$}}
// WINDOWS-NEXT:  [[TMP2:%.*]] = load fp128, ptr [[X_ADDR]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[TMP2]], ptr [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  [[TMP3:%.*]] = load fp128, ptr [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  store fp128 [[TMP3]], ptr [[AGG_RESULT]], align 16{{$}}
// WINDOWS-NEXT:  ret void

  return x;
}

