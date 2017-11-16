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

#ifndef __UTILS_D3D9_H
#define __UTILS_D3D9_H

#ifdef WIN32
#pragma comment (lib, "d3dx9d.lib")
#pragma comment (lib, "d3d9.lib")

#include "CL\cl_ext.h"

#include <string>
#include <memory>
#include <vector>
#include <d3d9.h>

// these things are defined only in the harness version we got from the Gdansk team
#define TEST_CASE_DOC(x, y, z, w)
#define TITLE
#define DESCRIPTION

class CD3D9Wrapper {
public:
  CD3D9Wrapper();
  ~CD3D9Wrapper();

  bool Init();
  LPDIRECT3D9 D3D();
  LPDIRECT3DDEVICE9 Device();
  D3DFORMAT Format();
  D3DADAPTER_IDENTIFIER9 Adapter();

private:
  bool WindowInit();
  void WindowDestroy();

  bool D3D9Init();
  void D3D9Destroy();

  static const wchar_t *WINDOW_TITLE;
  static const int WINDOW_WIDTH;
  static const int WINDOW_HEIGHT;

  HMODULE _hInstance;
  HWND _hWnd;

  LPDIRECT3D9 _d3d9;
  LPDIRECT3DDEVICE9 _d3dDevice;
  D3DDISPLAYMODE _d3ddm;
  D3DADAPTER_IDENTIFIER9 _adapter;
};

#endif
#endif  // __UTILS_D3D9_H
