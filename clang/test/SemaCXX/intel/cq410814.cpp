// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-ms-compatibility -DMS -verify %s
#ifdef MS
// expected-no-diagnostics
#endif

typedef struct HWND__ *HWND;

class CWindow {
public:
  CWindow(HWND);
  operator HWND();
#ifndef MS
    // expected-note@-2 {{candidate function}}
#endif
};

template <class TBase>
class CButtonT : public TBase {
#ifndef MS
    // expected-note@-2 {{candidate constructor (the implicit copy constructor) not viable}}
#endif
public:
  CButtonT(HWND hWnd = 0) : TBase(hWnd) {}
#ifndef MS
    // expected-note@-2 {{candidate constructor not viable}}
#endif
};

typedef CButtonT<CWindow> CButton;

class COpenDlg {
public:
  CWindow GetDlgItem(int);

  void foo(){
    CButton btnCheck1(GetDlgItem(1000));

    CButton btnCheck2 = GetDlgItem(1000);
#ifndef MS
    // expected-error@-2 {{no viable conversion from 'CWindow' to 'CButton' (aka 'CButtonT<CWindow>')}}
#endif
  }
};
