import sys, os
import socket
import struct
import time


sys.path.insert(0, os.path.join(os.path.dirname(sys.argv[0]), 'protobuf_lib.zip'))

from testlib.debugservermessages_pb2 import (
    LineInfo, ServerToClientMessage, ClientToServerMessage)

def make_socket(port):
    """ Create a socket on localhost and return it.
    """
    sockobj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockobj.connect(('localhost', port))
    return sockobj


def send_message(sock, message):
    """ Send a serialized message (protobuf Message interface)
        to a socket, prepended by its length packed in 4 bytes.
    """
    s = message.SerializeToString()
    packed_len = struct.pack('>L', len(s))
    packed_message = packed_len + s
    sock.send(packed_message)


def send_message_RUN(sock, bps):
    """ Send a RUN message with the given list of breakpoints. bps is a list
        of file,lineno pairs
    """
    msg = ClientToServerMessage()
    msg.type = ClientToServerMessage.RUN
    msg.run_msg.info = ''
    for bp in bps:
        bpinfo = msg.run_msg.breakpoints.add()
        bpinfo.file = bp[0]
        bpinfo.lineno = bp[1]
    send_message(sock, msg)


def send_message_SINGLE_STEP_IN(sock):
    msg = ClientToServerMessage()
    msg.type = ClientToServerMessage.SINGLE_STEP_IN
    send_message(sock, msg)


def send_message_START_SESSION(sock, global_id):
    """ global_id is a tuple of 3 numbers
    """
    msg = ClientToServerMessage()
    msg.type = ClientToServerMessage.START_SESSION
    msg.start_session_msg.global_id_x = global_id[0]
    msg.start_session_msg.global_id_y = global_id[1]
    msg.start_session_msg.global_id_z = global_id[2]
    send_message(sock, msg)


def send_message_GET_STACK_TRACE(sock):
    msg = ClientToServerMessage()
    msg.type = ClientToServerMessage.GET_STACK_TRACE
    send_message(sock, msg)


def send_message_GET_MEMORY_RANGE(sock, addr_start, addr_end):
    msg = ClientToServerMessage()
    msg.type = ClientToServerMessage.GET_MEMORY_RANGE
    msg.get_memory_range_msg.start_addr = addr_start
    msg.get_memory_range_msg.end_addr = addr_end
    send_message(sock, msg)


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


def get_msg_from_server(sock):
    """ Read a serialized response message from a socket.
    """
    msg = ServerToClientMessage()
    len_buf = socket_read_n(sock, 4)
    msg_len = struct.unpack('>L', len_buf)[0]
    msg_buf = socket_read_n(sock, msg_len)
    msg.ParseFromString(msg_buf)
    return msg


def main():
    try:
        # This should *always* be in lowercase for Windows
        CLNAME = '/tmp/ebenders/debug_server_code_reorg/install/Linux64/Release/bin/validation/debugger_test_type/cl_kernels/vector_values1.cl'
        server_port = 56203
        sock = make_socket(server_port)

        send_message_START_SESSION(sock, (0, 0, 0))
        msg = get_msg_from_server(sock)
        print 'Expected ack and got', msg

        send_message_SINGLE_STEP_IN(sock)
        msg = get_msg_from_server(sock)
        print 'got', msg

        send_message_RUN(sock, [(CLNAME, 13), (CLNAME, 5),])
        msg = get_msg_from_server(sock)
        print 'got', msg


        for i in range(32):
            send_message_RUN(sock, [

                (CLNAME, 12),
                (CLNAME, 5),

                #~ ('debug_tests_ocl.cl', 15),
                #~ ('debug_tests_ocl.cl', 19),

                ])
            msg = get_msg_from_server(sock)
            print 'got', msg

            print 'Sending GET_STACK_TRACE'
            send_message_GET_STACK_TRACE(sock)
            msg = get_msg_from_server(sock)
            print 'reply:', msg

            #~ print 'Sending GET_MEMORY_RANGE'
            #~ send_message_GET_MEMORY_RANGE(sock, 0x400, 0x402)
            #~ msg = get_msg_from_server(sock)
            #~ print 'reply:', msg


            time.sleep(0.2)
    except socket.error as e:
        print "{{<< SOCKET ERROR >>}}", e



#-------------------------------------------------------------------------------
if __name__ == "__main__":
    main()
