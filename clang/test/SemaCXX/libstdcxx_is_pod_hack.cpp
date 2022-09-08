<<<<<<< HEAD
// RUN: %clang_cc1 -fsyntax-only -verify %s
// RUN: %clang_cc1 -fsyntax-only -DINTEL_CUSTOMIZATION -verify %s
=======
// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx98-14 %s
>>>>>>> 83ea47acd7116bf50274534ba9b3bd3035c01da6

// This is a test for an egregious hack in Clang that works around
// issues with GCC's evolution. libstdc++ 4.2.x uses __is_pod as an
// identifier (to declare a struct template like the one below), while
// GCC 4.3 and newer make __is_pod a keyword. Clang treats __is_pod as
// a keyword *unless* it is introduced following the struct keyword.

template<typename T>
struct __is_pod { // expected-warning {{keyword '__is_pod' will be made available as an identifier}}
  __is_pod() {}
};

__is_pod<int> ipi;

// Ditto for __is_same.
template<typename T>
struct __is_same { // expected-warning {{keyword '__is_same' will be made available as an identifier}}
};

__is_same<int> isi;

#if INTEL_CUSTOMIZATION
// CQ414772: ensure __is_signed is still an identifier in libstdc++
// even if the return type is hidden by a typedef.
typedef bool _B;
struct test_is_signed {
  static const _B __is_signed = true;  // expected-warning {{keyword '__is_signed' will be made available as an identifier}}
};
#else
// Another, similar egregious hack for __is_signed, which is a type
// trait in Embarcadero's compiler but is used as an identifier in
// libstdc++.
struct test_is_signed {
  static const bool __is_signed = true; // expected-warning {{keyword '__is_signed' will be made available as an identifier}}
};
#endif // INTEL_CUSTOMIZATION


bool check_signed = test_is_signed::__is_signed;

template<bool B> struct must_be_true {};
template<> struct must_be_true<false>;

void foo() {
  bool b = __is_pod(int);
  must_be_true<__is_pod(int)> mbt;
}

// expected-warning@+1 {{declaration does not declare anything}}
struct // expected-error {{declaration of anonymous struct must be a definition}}
#pragma pack(pop)
    S {
};

#if !__has_feature(is_pod)
#  error __is_pod should still be available.
#endif
