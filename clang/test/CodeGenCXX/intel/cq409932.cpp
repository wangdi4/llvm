// RUN: %clang_cc1 -std=c++11 -O1 -mconstructor-aliases -fcxx-exceptions -fexceptions -fblocks -fintel-compatibility -triple x86_64-apple-macosx10.8  %s -emit-llvm -o - | FileCheck %s

namespace std {inline namespace __1 {
  class  facet
  {
  protected:
    __attribute__ ((__always_inline__))
      virtual ~facet();
  };
  template <class _CharT, bool _International = false>
    class  moneypunct : public facet {
    protected:
      __attribute__ ((__always_inline__))
        ~moneypunct() {}
    };
  extern template class  moneypunct<wchar_t, false>;
} }
struct Mwf : public std::moneypunct<wchar_t> {} Reqwf;
class mp_ : public std::moneypunct<wchar_t> {};
int main()
{
  mp_* mpp = new mp_;
  return 0;
}

// CHECK: define linkonce_odr void @_ZN3mp_D2Ev
// check: call void @_ZN3mp_D2Ev
