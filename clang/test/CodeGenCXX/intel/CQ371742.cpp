// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -std=c++11 -g -o - %s | FileCheck --check-prefix LINUX %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -o - -std=c++11 -g -o - %s | FileCheck --check-prefix WIN_MSVC %s

// LINUX: %"class._ZTSZ4mainE3$_0" = type { i32* }
// WIN_MSVC: %"class.__10<lambda_0>@" = type { i32* }

int main() {
  int p = 0, _data;
  auto lambda = [&](int parameter) -> int {
    _data = parameter;
    return _data;
  };

  return lambda(p);
}

// LINUX: !DICompositeType(tag: DW_TAG_class_type, name: "_ZTSZ4mainE3$_0", scope:
// WIN_MSVC: !DICompositeType(tag: DW_TAG_class_type, name: "__10<lambda_0>@", scope:
