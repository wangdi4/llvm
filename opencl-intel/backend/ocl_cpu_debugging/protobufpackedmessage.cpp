/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <string>
#include "protobufpackedmessage.h"

namespace ProtobufPackedMessage {


// Encodes the side into a header at the beginning of buf
//
static void encode_header(std::vector<char>& buf, unsigned size)
{
    assert(buf.size() >= HEADER_SIZE);
    buf[0] = static_cast<char>((size >> 24) & 0xFF);
    buf[1] = static_cast<char>((size >> 16) & 0xFF);
    buf[2] = static_cast<char>((size >> 8) & 0xFF);
    buf[3] = static_cast<char>(size & 0xFF);
}


bool pack(std::vector<char>& buf, const Message* msg)
{
    unsigned msg_size = msg->ByteSize();
    buf.resize(HEADER_SIZE + msg_size);
    encode_header(buf, msg_size);
    return msg->SerializeToArray(&buf[HEADER_SIZE], msg_size);
}


unsigned decode_header(const std::vector<char>& buf)
{
    if (buf.size() < HEADER_SIZE)
        return 0;
    unsigned msg_size = 0;
    for (unsigned i = 0; i < HEADER_SIZE; ++i)
        msg_size = msg_size * 256 + (static_cast<unsigned>(buf[i]) & 0xFF);
    return msg_size;
}


bool unpack(Message* msg, const std::vector<char>& buf)
{
    return msg->ParseFromArray(&buf[0], buf.size());
}

} // namespace
