//
// A "packed" message is a Message serialized into a buffer (vector of chars)
// prepended by its length in a 4-byte big-endian "header". 
// These utilities allow packing and unpacking a Message into/from a buffer.
// 
#ifndef PROTOBUFPACKEDMESSAGE_H
#define PROTOBUFPACKEDMESSAGE_H

#include <vector>
#include <google/protobuf/message.h>


namespace ProtobufPackedMessage {

using ::google::protobuf::Message;


//
// The header size for packed messages
//
const unsigned HEADER_SIZE = 4;


// Given a buffer with the first HEADER_SIZE bytes representing the header,
// decode the header and return the message length. Return 0 in case of 
// an error.
//
unsigned decode_header(const std::vector<char>& buf);


// Pack the message into the given data buffer. The buffer is resized to
// exactly fit the message.
// Return false in case of an error, true if successful.
//
bool pack(std::vector<char>& buf, const Message* msg);


// Unpack message from the given packed buffer (without the header)
// Return true if unpacking successful, false otherwise.
//
bool unpack(Message* msg, const std::vector<char>& buf);


// A generic function to show contents of a container holding byte data 
// as a string with hex representation for each byte.
// Useful for debugging.
// Has to be called after <cstdio> and <string> have been included.
//
template <class CharContainer>
std::string show_hex(const CharContainer& c)
{
    std::string hex;
    char buf[16];
    typename CharContainer::const_iterator i;
    for (i = c.begin(); i != c.end(); ++i) {
        std::sprintf(buf, "%02X ", static_cast<unsigned>(*i) & 0xFF);
        hex += buf;
    }
    return hex;
}


} // namespace

#endif // PROTOBUFPACKEDMESSAGE_H
