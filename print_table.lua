function print_table_indent(t, i)
	local indent = "";
	for j = 0, i do 
		indent = indent .. '    ';
	end
	
	for k, v in pairs(t) do 
		if (type(v) == "table") then 
		    print(indent .. k .. ": {");
			print_table_indent(v, i + 1);
			print(indent .. "}");
		else
		    print(indent .. k .. ":" .. tostring(v));
		end
	end
end

function print_table(t)
    if t == nil then
        print("nil table");
    else
        print("{");
        print_table_indent(t, 0);
        print("}");
    end  
end