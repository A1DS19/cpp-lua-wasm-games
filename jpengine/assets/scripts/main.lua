print("running lua script")

local g_cam = Camera.get()

-- We return the function we want the engine to call every frame.
-- This decouples the Lua logic from the C++ function names.
return function()
	if g_cam then
		local x, y = g_cam:get_position()
		g_cam:set_position(x - 0.5, y - 0.5)
		g_cam:set_scale(g_cam:get_scale() * 1.1)
	end
end

