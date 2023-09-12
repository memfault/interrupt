import requests
import struct
import socket

INTERFACE = 'utun3'
LOCAL_PORT = 20001
BUFFER_SIZE = 508 + 28

MEMFAULT_IPV6_GROUP_6 = 'ff05::f417'

addrinfo = socket.getaddrinfo(MEMFAULT_IPV6_GROUP_6, None)[0]

print(addrinfo)

UDPServerSocket = socket.socket(addrinfo[0], type=socket.SOCK_DGRAM)
UDPServerSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
UDPServerSocket.bind(('', LOCAL_PORT))

group_bin = socket.inet_pton(addrinfo[0], addrinfo[4][0])
mreq = group_bin + struct.pack('@I', socket.if_nametoindex(INTERFACE))
UDPServerSocket.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

print(f"Listening on interface {INTERFACE} port {LOCAL_PORT}")

def post_chunk(project_key, chunk, device_serial):
    CHUNK_BASE_URL = "https://chunks.memfault.com"
    CHUNK_API_HEADERS = {
        "Memfault-Project-Key": project_key,
        "Content-Type": "application/octet-stream",
    }
    url = "{}/api/v0/chunks/{}".format(CHUNK_BASE_URL, device_serial)
    response = requests.post(url, data=chunk, headers=CHUNK_API_HEADERS)
    if response.status_code == 202:
        print("Success")
    else:
        print(response.status_code)
        print(response.text)

while(True):
    message, address = UDPServerSocket.recvfrom(BUFFER_SIZE)
    version_bytes, project_key_bytes, device_serial_bytes, chunk_bytes = message.split(b"\x00", 3)
    version = version_bytes.decode('utf-8')
    project_key = project_key_bytes.decode('utf-8')
    device_serial = device_serial_bytes.decode('utf-8')
    print("{}: version {}, project key {}, device_serial {}, chunk {}".format(address, version, project_key, device_serial, chunk_bytes))
    post_chunk(project_key, chunk_bytes, device_serial)


