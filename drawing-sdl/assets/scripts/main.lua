print("running lua script")

function move_box()
	local x = get_x_cpp()
	local y = get_y_cpp()
	move_box_cpp(x + 1, y + 1)
end

