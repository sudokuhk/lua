--[[
network interface define:
]]

package.path = "./print_table.lua";

require "print_table.lua";

function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function ffmpeg_callback(percent)
    print("ffmpeg callbback:" .. percent);
end

function send_http_request(socket, method, url)
    -- send request
    local request =
        method .. " " .. url["path"] .. " HTTP/1.1\r\n" ..
        "Host: " .. url["host"] .. "\r\n" ..
        "Date: CST " .. os.date("%Y-%m-%d %H:%M:%S", os.time()) .. "\r\n" ..
        "Connection: keep-alive\r\n" ..
        "Cache-Control: max-age=0\r\n" ..   
        "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_5) AppleWebKit/537.36" ..
        "(KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36\r\n" ..
        "DTN: 1\r\n" ..        
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n" ..
        "Accept-Encoding: gzip, deflate\r\n" ..
        "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\n" ..   
        "\r\n";
        
    return cnetwork.send_string(socket, request);
end

function receive_http_startline_headers(socket)
    response = {
        ["Connection"] = true,
    };
    
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
        if (str == nil or (string.sub(str, 1, 1) == '\r' and string.sub(str, 2, 2) == '\n')) then
            break;
        end
        
        local start, tail = string.find(str, ":");
        local key = string.sub(str, 1, start - 1);
        local value = trim(string.sub(str, tail + 1, string.len(str) - 2));
        --print("[" .. key .. "] = [" .. value .. "]");
        response[key] = value;
    end
    
    return response;
end

function http_receive_string_body_by_length(socket, length)

    local string = nil;
    
    if (length > 0) then
        local buffer = cmemory.alloc(length);
        local nrecv = cnetwork.recv_data(socket, buffer, length);
        if (nrecv > 0) then
            string = cmemory.tostring(buffer, nrecv);
        end
        cmemory.free(buffer);
    end
    
    return string;
end

function http_receive_string_body_by_chunked(socket)
    print("http_receive_string_body_by_chunked");
    
    local string = "";
    
    while (true)
    do
        local sz_length = cnetwork.recv_string(socket);
        local length = tonumber(sz_length, 16);
        --print("length:" .. length);
        if (length == nil) then --error
            break;
        end
        
        if (length == 0) then --end
            cnetwork.recv_string(socket);
            break;
        end
        
        local buffer = cmemory.alloc(length);
        local nrecv = cnetwork.recv_data(socket, buffer, length);
        --print("length:" .. length .. ", recv:" .. nrecv);
        if (nrecv ~= length) then
            break;
        end
        local thiz_str = cmemory.tostring(buffer, nrecv);
        --print("this:" .. thiz_str);
        string = string .. thiz_str;
        cmemory.free(buffer);
        cnetwork.recv_string(socket);
    end
    
    return string;
end

function http_receive_string_body(socket, headers)
    if (socket == nil or headers == nil) then
        return nil;
    end
    
    if (headers["Content-Length"] ~= nil) then
        local length = tonumber(response["Content-Length"]);       
        print("length:" .. length);
        if (length ~= nil and length > 0) then
            return http_receive_string_body_by_length(socket, length);
        end
    elseif (headers["Transfer-Encoding"] == "chunked") then
        return http_receive_string_body_by_chunked(socket);
    end
    
    return nil;
end

function http_receive_file_body(socket, headers)

    local result = false;
    if (headers == nil or headers["length"] == nil) then
        return result;
    end
    
    -- receive body
    if headers["length"] > 0 then
        local length = headers["length"];
        local file = cfile.open("test.mp4", "wb");
        local step = cmath.min(length, 4096);
        local buffer = cmemory.alloc(step);
        
        while (true) 
        do
            local nrecv = cnetwork.recv_data(socket, buffer, cmath.min(length, step));
            if nrecv > 0 then
                cfile.write_data(file, buffer, nrecv);
                length = length - nrecv;
                --print("receive length:" .. nrecv .. ", remain:" .. length);
                if length == 0 then
                    result = true;
                    break;
                end
            else
                break;
            end            
        end
        cmemory.free(buffer);
        cfile.close(file);
    end
    
    return result;
end

function test_request_error()

    print("test_request_error");
    --url = curl.parse("http://127.0.0.1/test.mp4123");   --length
    print_table(url);
    
    local socket = cnetwork.connect(url["host"], url["port"], 10);
    print("socket = " .. tostring(socket));
    
    -- send request
    send_http_request(socket, "GET", url);
    
    -- receive http startline & header
    local headers = receive_http_startline_headers(socket);
    print_table(headers);
        
    -- receive string
    local string = http_receive_string_body(socket, headers);
    print(string);
        
    cnetwork.disconnect(socket);
    socket = nil;
end

function test_download_file()

    print("test_download_file");
    url = curl.parse("http://127.0.0.1/test.mp4");
    print_table(url);
    
    local socket = cnetwork.connect(url["host"], url["port"], 10);
    print("socket = " .. tostring(socket));

    -- send request
    send_http_request(socket, "GET", url);
    
    -- receive http startline & header
    local headers = receive_http_startline_headers(socket);
    
    -- receive file
    local result = http_receive_file_body(socket, headers);
    print("receive file:" .. tostring(result));
    
    cnetwork.disconnect(socket);
    socket = nil;
end

function test_upload_file()
    print("test_upload_file");
    url = curl.parse("http://127.0.0.1/net.lua");
    
    local socket = cnetwork.connect(url["host"], url["port"], 10);
    send_http_request(socket, "PUT", url);
end

--local dir = cdir.list(".");
--print_table(dir);

test_request_error();