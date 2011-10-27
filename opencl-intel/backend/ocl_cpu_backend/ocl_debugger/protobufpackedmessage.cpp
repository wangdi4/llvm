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

