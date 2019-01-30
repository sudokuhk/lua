--[[
network interface define:
]]

package.path = "./print_table.lua";

require "print_table.lua";

function trim(str)
    return trim:match("^%s+(.-)%s+$")
end

print("network interface test");

print("url parse");
url = curl.parse("http://127.0.0.1/test.mp4");
print_table(url);
print("url parse done");

socket = cnetwork.connect(url["host"], url["port"], 10);
print("socket = " .. socket);

while (true)
do
    -- send request
    local request =
        "GET " .. url["path"] .. " HTTP/1.1\r\n" ..
        "Accept-Encoding: gzip, deflate\r\n" ..
        "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n" ..
        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_5) AppleWebKit/537.36" ..
        " (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36\r\n" ..
        "Host: " .. url["host"] ..
        "Date: CST " .. os.date("%Y-%m-%d %H:%M:%S", os.time()) ..
        "Connection: keep-alive\r\n" ..
        "\r\n";

    
    cnetwork.send_string(socket, request);
    
    response = {
        --keepalive" = true,
    };
    
    -- receive start line
    str = cnetwork.recv_string(socket);
    if str ~= nil then
        --print(str);
        local i1 = string.find(str, ' ');
        local i2 = string.find(str, ' ', i1 + 1);
        local version = string.sub(str, 1, i1 - 1);
        local rescode = string.sub(str, i1 + 1, i2 - 1);
        local resaon = string.sub(str, i2 + 1, -3); -- -3, skip \r\n
        
        if version == "HTTP/1.0" then
            response["version"] = 10;
        else
            response["version"] = 11;
        end
        
        response["rescode"] = tonumber(rescode);
    end
    
    -- receive http headers
    while (true) 
    do
        str = cnetwork.recv_string(socket);
        --print(tostring(str));
                
        -- \r\n, end of header
        if (string.sub(str, 1, 1) == '\r' and string.sub(str, 2, 2) == '\n') then
            break;
        end
        
        local start, tail = string.find(str, "Length:");        
        if  tail ~= nil then
            --print("start:" .. start .. ", end:" .. tail);
            local length = tonumber(string.sub(str, tail + 1, -1));
            --print(length);
            response["length"] = length;
        end
        
        local start, tail = string.find(str, "Connection:");
        if tail ~= nil then
            local i, j = string.find(str, "close");
            if j ~= nil then
                response["keepalive"] = false;
            end
        end
    end
    
    if response["keepalive"] == nil and response["version"] == 10 then
        response["keepalive"] = false;
    end
    
    -- receive body
    if response["length"] > 0 then
        local length = response["length"];
        local file = cfile.open("test.mp4", "wb");
        buffer = cmemory.alloc(length);
        nrecv = cnetwork.recv_data(socket, buffer, length);
        print("receive length:" .. nrecv);
        cfile.write_data(file, buffer, nrecv);
        cmemory.free(buffer);
        cfile.close(file);
    end
    
    if response["keepalive"] == false then
        print("keepalive false, disconnect");
        cnetwork.disconnect(socket);
        socket = nil;
    end
    
    break;
end

cnetwork.disconnect(socket);

local dir = cdir.list(".");
print_table(dir);