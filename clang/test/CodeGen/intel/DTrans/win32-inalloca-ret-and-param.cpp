// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-pc-windows-msvc -emit-dtrans-info -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,W32
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-pc-windows-msvc -emit-dtrans-info -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,W64
using size_t = unsigned long long;
class _String_val {
public:

    _String_val() noexcept : _Bx() {}
    static constexpr size_t _BUF_SIZE = 16;
    union _Bxty {
        _Bxty() noexcept  {}

        ~_Bxty() noexcept {}

        char _Buf[_BUF_SIZE];
        char *_Ptr;
    } _Bx;
};

_String_val _Makestr(_String_val _Message);

void use()
{
  _Makestr(_String_val{});
}

// W32: declare !intel.dtrans.func.type ![[MAKE_STR:[0-9]+]]{{.*}} "intel_dtrans_func_index"="1" ptr @"?_Makestr@@YA?AV_String_val@@V1@@Z"(ptr inalloca(<{ ptr, %"class..?AV_String_val@@._String_val" }>) "intel_dtrans_func_index"="2")
// Win64 doesn't pass this as inalloca, only win32 does, but testing for
// completeness.
// W64: declare !intel.dtrans.func.type ![[MAKE_STR:[0-9]+]]{{.*}} void @"?_Makestr@@YA?AV_String_val@@V1@@Z"(ptr sret(%"class..?AV_String_val@@._String_val"){{.*}} "intel_dtrans_func_index"="1", ptr noundef "intel_dtrans_func_index"="2")
// W32: ![[MAKE_STR]] = distinct !{![[SV_PTR:[0-9]+]], ![[INALLOCA_LITERAL:[0-9]+]]}
// W64: ![[MAKE_STR]] = distinct !{![[SV_PTR:[0-9]+]], ![[SV_PTR]]}
// CHECK: ![[SV_PTR]] = !{%"class..?AV_String_val@@._String_val" zeroinitializer, i32 1}
// W32: ![[INALLOCA_LITERAL]] = !{!"L", i32 2, ![[SV_PTR]], ![[SV:[0-9]+]]}
// W32: ![[SV]] = !{%"class..?AV_String_val@@._String_val" zeroinitializer, i32 0}
