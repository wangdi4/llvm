//==--- utils_win32.cpp - Win-specific implementation of utils -*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "common_utils.h"

#include <Psapi.h>
#include <assert.h>
#include <climits>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

bool GetEnv(std::string &result, const std::string &name) {
  char *buf;
  size_t size;
  errno_t err = _dupenv_s(&buf, &size, name.c_str());
  if (err || (0 == size) || !buf) {
    result = "";
    return false;
  }

  result = std::string(buf);
  free(buf);
  return true;
}

// The code below is based on code from vnx/os_helpers
// TODO: remove this copy-paste and reuse vnx library at all in our tests

// MS C compiler issues warnings on `strdup'. Let us avoid them.
#define strdup _strdup

typedef std::vector<char> buffer_t;

int const _size = MAX_PATH + 1; // Initial buffer size for path.
int const _count = 8; // How many times we will try to double buffer size.

static std::string dir_sep() { return "\\"; }

static std::string exe_path(unsigned int pid) {
  buffer_t path(_size);
  int count = _count;

  while (true) {
    DWORD len = 0;
    if (!pid) {
      len = GetModuleFileNameA(nullptr, &path.front(), DWORD(path.size()));
    } else {
      HANDLE hProcess = OpenProcess(READ_CONTROL | PROCESS_VM_READ |
                                        PROCESS_QUERY_INFORMATION,
                                    FALSE, pid);
      if (hProcess != nullptr) {
        len = GetModuleFileNameEx(hProcess, nullptr, &path.front(),
                                  DWORD(path.size()));
        CloseHandle(hProcess);
      } else {
        // VNX_ERR("OpenProcess failed, cannot get executable path");
        return "";
      }
    }

    if (len == 0) {
      int err = GetLastError();
      fprintf(stderr, "ERROR: Getting executable path failed: %s\n",
              std::to_string(err).c_str());
      return "";
    }; // if

    if (len < path.size()) {
      path.resize(len);
      break;
    }; // if

    // Buffer too small.
    if (count > 0) {
      --count;
      path.resize(path.size() * 2);
    } else {
      // VNX_ERR(
      //   "ERROR: Getting executable path failed: "
      //   "Buffer of %lu bytes is still too small\n",
      //   (unsigned long) path.size()
      //);
      return "";
    }; // if
  };   // forever

  return std::string(&path.front(), path.size());
}

std::string get_exe_dir(unsigned int pid) {
  std::string path = exe_path(pid);
  if (!path.empty()) {
    size_t i = path.find_last_of("/\\");
    return path.substr(0, i) + dir_sep();
  }
  return std::string();
}

void readBinary(std::string filename, std::vector<unsigned char> &binary) {
  std::ifstream file(filename, std::fstream::binary | std::fstream::in);
  assert(file.is_open() && "Unable to open file");
  std::copy(std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(), std::back_inserter(binary));
  file.close();
  assert(binary.size() && "Unable to read binary");
}
