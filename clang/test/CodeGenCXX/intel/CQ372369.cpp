// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 -debug-info-kind=limited -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -debug-info-kind=limited -emit-llvm -o - %s | FileCheck %s
struct St {
  static int static_function() {return 0;}
};
// CHECK: !DISubprogram(name: "static_function",{{.+}}, isDefinition: false, {{.+}}, flags: DIFlagPrototyped | DIFlagStaticMember,

int main() {
  return St::static_function();
}
