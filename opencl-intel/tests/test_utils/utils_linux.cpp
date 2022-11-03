//==--- utils_linux.cpp - Linux-specific implementation of utils -*- C++ -*-==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "common_utils.h"

#include <assert.h>
#include <cerrno> // errno
#include <climits>
#include <fstream>
#include <libgen.h> // dirname
#include <string>
#include <unistd.h> // readlink
#include <vector>

bool GetEnv(std::string &result, const std::string &name) {
  char *buf;
  buf = getenv(name.c_str());
  if (!buf) {
    result = "";
    return false;
  }

  result = std::string(buf);
  return true;
}

// The code below is based on code from vnx/os_helpers
// TODO: remove this copy-paste and reuse vnx library at all in our tests

typedef std::vector<char> buffer_t;

int const _size = PATH_MAX + 1; // Initial buffer size for path.
int const _count = 8; // How many times we will try to double buffer size.

static std::string dir_sep() { return "/"; }

static std::string exe_path(unsigned int pid) {
  static std::string const exe =
      (!pid) ? "/proc/self/exe" : "/proc/" + std::to_string(pid) + "/exe";

  buffer_t path(_size);
  int count = _count; // Max number of iterations.

  while (true) {
    ssize_t len = readlink(exe.c_str(), &path.front(), path.size());

    if (len < 0) {
      // Oops.
      // int err = errno;
      // VNX_ERR(
      //    "ERROR: Getting executable path failed: "
      //    "Reading symlink `%s' failed: %s\n",
      //    exe.c_str(), vnx::err_str( err ).c_str()
      //);
      return "";
    }; // if

    // Typecast avoids warning "comparison between signed and unsigned integer
    // expressions". Typecast is safe because `len' is positive (or zero) here.
    if (size_t(len) < path.size()) {
      // We got the path.
      path.resize(len);
      break;
    }; // if

    // Oops, buffer is too small.
    if (count > 0) {
      --count;
      // Enlarge the buffer.
      path.resize(path.size() * 2);
    } else {
      // VNX_ERR(
      //     "ERROR: Getting executable path failed: "
      //     "Reading symlink `%s' failed: Buffer of %lu bytes is still too
      //     small\n", exe.c_str(), (unsigned long) path.size()
      //);
      return "";
    }; // if
  };   // forever

  return std::string(&path.front(), path.size());
} // exe_path

std::string get_exe_dir(unsigned int pid) {
  std::string path = exe_path(pid);
  if (!path.empty()) {
    buffer_t buffer(path.begin(), path.end() + 1);
    path = std::string(dirname(&buffer.front())) + dir_sep();
  }
  return path;
}

void readBinary(std::string filename, std::vector<unsigned char> &binary) {
  std::ifstream file(filename, std::fstream::binary | std::fstream::in);
  assert(file.is_open() && "Unable to open file");
  std::copy(std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(), std::back_inserter(binary));
  file.close();
  assert(binary.size() && "Unable to read binary");
}