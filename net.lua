--[[
network interface define:
]]

print("network interface test");

buffer = cmemory.alloc(100);
cmemory.free(buffer);

socket = cnetwork.connect("127.0.0.1", 80, 10);
print("socket = " .. socket);
cnetwork.send_string(socket, "hello, world");
recvstr = cnetwork.recv_string(socket);
print(recvstr);

cnetwork.disconnect(socket);
