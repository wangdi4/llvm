/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
// this file includes naming conventions and constant shared by the vectorizer passes
// this file should NOT include any environment specific data


// Maximum width (in elements) supported as input
// This value was observed when scalarizing ExtractElementInst from vector of
// <128 x i8> after a bitcast <16 x double> to <128 x i8>
#define MAX_INPUT_VECTOR_WIDTH 128

// Define estimated amount of instructions in function
#define ESTIMATED_INST_NUM 128

// Maximum supported packetization width
#define MAX_PACKET_WIDTH 16
