function get_random_color()
	local val = math.random(5)
	if val == 1 then
		return J2D_RED
	elseif val == 2 then
		return J2D_GREEN
	elseif val == 3 then
		return J2D_BLUE
	elseif val == 4 then
		return J2D_YELLOW
	elseif val == 5 then
		return J2D_MAGENTA
	end

	return J2D_GREEN
end

function shallow_clone(tbl)
	local clone = {}
	for k, v in pairs(tbl) do
		clone[k] = v
	end

	return clone
end

function deep_clone(tbl)
	local clone = {}
	for k, v in pairs(tbl) do
		if type(v) == "table" then
			clone[k] = deep_clone(v)
		else
			clone[k] = v
		end
	end

	setmetatable(clone, getmetatable(tbl))
	return clone
end
