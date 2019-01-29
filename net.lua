--[[
network interface define:
]]

package.path = "./print_table.lua";

require "print_table.lua";

print("network interface test");

print("url parse");
url = curl.parse("http://127.0.0.1/test.mp4");
print_table(url);
print("url parse done");

buffer = cmemory.alloc(100);
cmemory.free(buffer);

socket = cnetwork.connect(url["host"], url["port"], 10);
print("socket = " .. socket);
cnetwork.send_string(socket, "hello, world");

while (true)
do
    str = cnetwork.recv_string(socket);
    --print(tostring(string.len(str)) .. ":" .. str);
    --print("byte 0:" .. string.byte(string.sub(str, 1, 1)) .. ", 1-2" .. string.byte(string.sub(str, 2, 2)));
    if (string.sub(str, 1, 1) == '\r' and string.sub(str, 2, 2) == '\n') then
        break;
    end
end

cnetwork.disconnect(socket);
