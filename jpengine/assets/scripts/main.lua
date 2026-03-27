print("running lua script")

-- Test component construction
local transform = Transform(100, 200, 0)
print("transform x: " .. transform.position.x)
print("transform y: " .. transform.position.y)

local transform2 = Transform(glm.vec2(50, 50), glm.vec2(2, 2), 45)
print("transform2 scale x: " .. transform2.scale.x)

local sprite = Sprite("character.png", 16, 16, 0, 0, 0, Color(255, 255, 255, 255))
print("sprite texture: " .. sprite.texture)
print("sprite width: " .. sprite.width)

local anim = Animation(8, 12, 0, false, true)
print("anim num_frames: " .. anim.num_frames)
print("anim frame_rate: " .. anim.frame_rate)

local body = RigidBody()
body.velocity = glm.vec2(1, 0)
print("body velocity x: " .. body.velocity.x)

local g_cam = Camera.get()

return function()
	if g_cam then
		local x, y = g_cam:get_position()
		g_cam:set_position(x - 0.5, y - 0.5)
		g_cam:set_scale(g_cam:get_scale() * 1.001)
	end
end
