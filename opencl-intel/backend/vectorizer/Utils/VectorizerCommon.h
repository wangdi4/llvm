// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// this file includes naming conventions and constant shared by the vectorizer passes
// this file should NOT include any environment specific data


// Maximum width (in elements) supported as input
// An AMX tile has a maximum size of 16 rows x 64 bytes. It could be flatten to
// a <1024 x i8> vector.
#define MAX_INPUT_VECTOR_WIDTH 1024

// Define estimated amount of instructions in function
#define ESTIMATED_INST_NUM 128

// Maximum supported packetization width
#define MAX_PACKET_WIDTH 16
