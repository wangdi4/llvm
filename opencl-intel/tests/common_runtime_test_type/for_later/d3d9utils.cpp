/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2011 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code (Material) are owned by Intel Corporation or its
 * suppliers or licensors.
 *
 * Title to the Material remains with Intel Corporation or its suppliers and
 * licensors. The Material contains trade secrets and proprietary and
 * confidential information of Intel or its suppliers and licensors.
 * The Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed,
 * or disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure
 * or delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 *
 ******************************************************************************/

#include "d3d9utils.h"

#ifdef WIN32
#include <d3d9.h>
#include <iostream>
#include <limits.h>
#include <math.h>
#include <vector>

const wchar_t *CD3D9Wrapper::WINDOW_TITLE = L"cl_khr_d3d9_sharing_conformance";
const int CD3D9Wrapper::WINDOW_WIDTH = 256;
const int CD3D9Wrapper::WINDOW_HEIGHT = 256;

static void print_error(const std::string &errorMessage) {
  std::cout << errorMessage << std::endl;
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                              LPARAM lParam) {
  switch (msg) {
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_PAINT:
    ValidateRect(hWnd, 0);
    return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CD3D9Wrapper::WindowInit() {
  _hInstance = GetModuleHandle(NULL);
  static WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L,   0L,
                          _hInstance,         NULL,       NULL,    NULL, NULL,
                          WINDOW_TITLE,       NULL};

  RegisterClassEx(&wc);

  _hWnd =
      CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 0, 0,
                   WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, wc.hInstance, NULL);

  if (!_hWnd) {
    print_error("Failed to create window");
    return false;
  }

  ShowWindow(_hWnd, SW_SHOWDEFAULT);
  UpdateWindow(_hWnd);

  return true;
}

CD3D9Wrapper::CD3D9Wrapper()
    : _hInstance(NULL), _hWnd(NULL), _d3d9(NULL), _d3dDevice(NULL) {}

CD3D9Wrapper::~CD3D9Wrapper() {
  D3D9Destroy();
  WindowDestroy();
}

void CD3D9Wrapper::WindowDestroy() {
  if (_hWnd)
    DestroyWindow(_hWnd);
  _hWnd = NULL;
}

bool CD3D9Wrapper::D3D9Init() {
  if (!_hWnd) {
    print_error("Window is not created");
    return false;
  }

  _d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
  if (!_d3d9) {
    print_error("Direct3DCreate9 failed");
    return false;
  }

  unsigned int adapterIdx = 0;
  for (; adapterIdx < _d3d9->GetAdapterCount(); adapterIdx++) {
    D3DCAPS9 caps;
    if (FAILED(_d3d9->GetDeviceCaps(adapterIdx, D3DDEVTYPE_HAL, &caps)))
      continue;

    if (FAILED(_d3d9->GetAdapterIdentifier(adapterIdx, 0, &_adapter))) {
      print_error("GetAdapterIdentifier failed");
      D3D9Destroy();
      return false;
    }

    break;
  }

  if (adapterIdx == _d3d9->GetAdapterCount()) {
    print_error("DX9-OCL interop is not supported for any devices");
    D3D9Destroy();
    return false;
  }

  RECT rect;
  GetClientRect(_hWnd, &rect);

  _d3d9->GetAdapterDisplayMode(adapterIdx, &_d3ddm);

  D3DPRESENT_PARAMETERS d3dParams;
  ZeroMemory(&d3dParams, sizeof(d3dParams));

  d3dParams.Windowed = TRUE;
  d3dParams.BackBufferCount = 1;
  d3dParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dParams.hDeviceWindow = _hWnd;
  d3dParams.BackBufferWidth = WINDOW_WIDTH;
  d3dParams.BackBufferHeight = WINDOW_WIDTH;
  d3dParams.BackBufferFormat = _d3ddm.Format;

  if (FAILED(_d3d9->CreateDevice(adapterIdx, D3DDEVTYPE_HAL, _hWnd,
                                 D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                 &d3dParams, &_d3dDevice))) {
    print_error("CreateDevice failed");
    D3D9Destroy();
    return false;
  }

  _d3dDevice->BeginScene();
  _d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
  _d3dDevice->EndScene();

  return true;
}

void CD3D9Wrapper::D3D9Destroy() {
  if (_d3dDevice)
    _d3dDevice->Release();
  _d3dDevice = 0;

  if (_d3d9)
    _d3d9->Release();
  _d3d9 = 0;
}

bool CD3D9Wrapper::Init() {
  if (!WindowInit())
    return false;

  if (!D3D9Init())
    return false;

  return true;
}

LPDIRECT3D9 CD3D9Wrapper::D3D() { return _d3d9; }

LPDIRECT3DDEVICE9 CD3D9Wrapper::Device() { return _d3dDevice; }

D3DFORMAT CD3D9Wrapper::Format() { return _d3ddm.Format; }

D3DADAPTER_IDENTIFIER9 CD3D9Wrapper::Adapter() { return _adapter; }
#endif