import struct
from debugservermessages_pb2 import ServerToClientMessage


def socket_read_n(sock, n):
    """ Read exactly n bytes from the socket.
        Raise RuntimeError if the connection closed before n bytes were read.
    """
    buf = ''
    while n > 0:
        data = sock.recv(n)
        if data == '':
            raise RuntimeError('unexpected connection close')
        buf += data
        n -= len(data)
    return buf


def send_message(sock, message):
    """ Send a serialized message (protobuf Message interface)
        to a socket, prepended by its length packed in 4
        bytes (big endian).
    """
    s = message.SerializeToString()
    packed_len = struct.pack('>L', len(s))
    sock.sendall(packed_len + s)

def send_message_wrong_size(sock, message, size):
    """ Send a serialized message (protobuf Message interface)
        to a socket, prepended by its length packed in 4
        bytes (big endian).
    """
    packed_len = struct.pack('>L', size)
    sock.sendall(packed_len + message)

def get_message(sock):
    """ Read a message from a socket. Return the deserialized
        ServerToClientMessage.
    """
    len_buf = socket_read_n(sock, 4)
    msg_len = struct.unpack('>L', len_buf)[0]
    msg_buf = socket_read_n(sock, msg_len)

    msg = ServerToClientMessage()
    msg.ParseFromString(msg_buf)
    return msg
