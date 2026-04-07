j2d_run_script("assets/scripts/defs/asset-defs.lua")
load_assets(asset_defs)

local entity_1 = Entity()
entity_1:add_component(Transform(100.0, 150.0, 1, 1, 0.0))
local sprite_1 = entity_1:add_component(Sprite("character", 32, 32, 0, 0, 0, J2D_WHITE))
sprite_1:generate_uvs(32, 32)

local entity_2 = Entity()
entity_2:add_component(Transform(450.0, 75.0, 1, 1, 0.0))
local sprite_2 = entity_2:add_component(Sprite("character", 32, 32, 0, 0, 0, J2D_RED))
sprite_2:generate_uvs(32, 32)

local registry = Registry()
local entity_3 = registry:create_entity()
entity_3:add_component(Transform(96.0, 69.0, 1, 1, 0.0))
local sprite_3 = entity_3:add_component(Sprite("character", 32, 32, 0, 0, 0, J2D_GREEN))
sprite_3:generate_uvs(32, 32)

local registry = Registry()
local entity_4 = registry:create_entity()
entity_4:add_component(Transform(200, 200, 1, 1, 0.0))
entity_4:add_component(TextComponent("pixel", "testing from lua binding"))

local view = registry:get_entities(Transform)

view:for_each(function(entity)
	local transform = entity:get_component(Transform)
	local x, y = transform:get_position()
end)

function update_keyboard()
	local transform = entity_1:get_component(Transform)
	if Keyboard.pressed(KEY_W) then
		transform.position.y = transform.position.y - 10
	elseif Keyboard.pressed(KEY_S) then
		transform.position.y = transform.position.y + 10
	elseif Keyboard.pressed(KEY_A) then
		transform.position.x = transform.position.x - 10
	elseif Keyboard.pressed(KEY_D) then
		transform.position.x = transform.position.x + 10
	end
end

function play_music()
	if Keyboard.just_pressed(KEY_ENTER) and not MusicPlayer.is_playing() then
		MusicPlayer.play("the-field-of-dreams")
	end
end

function update_mouse()
	if Mouse.just_pressed(LEFT_BTN) then
		print("left mouse btn pressed")
		local mouse_pos = Mouse.screen_position()
		print("mouse pos: [x=" .. mouse_pos.x .. "y=" .. mouse_pos.y .. "]")
	end
end

function update_gamepad()
	if not Gamepad.is_gamepad_present() then
		return
	end

	local transform = entity_1:get_component(Transform)
	if Gamepad.pressed(DPAD_UP) then
		transform.position.y = transform.position.y - 10
	elseif Gamepad.pressed(DPAD_DOWN) then
		transform.position.y = transform.position.y + 10
	elseif Gamepad.pressed(DPAD_LEFT) then
		transform.position.x = transform.position.x - 10
	elseif Gamepad.pressed(DPAD_RIGHT) then
		transform.position.x = transform.position.x + 10
	end

	local deadzone = 8000
	local axis_x = Gamepad.get_axis_position(AXIS_X1)
	local axis_y = Gamepad.get_axis_position(AXIS_Y1)

	if axis_x < -deadzone then
		transform.position.x = transform.position.x - 10
	elseif axis_x > deadzone then
		transform.position.x = transform.position.x + 10
	end

	if axis_y < -deadzone then
		transform.position.y = transform.position.y - 10
	elseif axis_y > deadzone then
		transform.position.y = transform.position.y + 10
	end
end

main = {
	update = function()
		update_keyboard()
		update_mouse()
		update_gamepad()
		play_music()
	end,
}
