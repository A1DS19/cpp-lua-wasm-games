j2d_run_script("assets/scripts/defs/asset-defs.lua")
j2d_run_script("assets/scripts/utils.lua")

load_assets(asset_defs)

local gplayer = Entity()
local transform = gplayer:add_component(Transform(vec2(50, 50), vec2(2, 2), 0))
local sprite = gplayer:add_component(Sprite("characters", 32, 32, 0, 0, 1, J2D_WHITE))
sprite:generate_uvs(736, 128)

local circle_collider = gplayer:add_component(CircleCollider(8, vec2(0, 0)))
local physics_attr = PhysicsAttributes()
physics_attr.etype = BodyType.Dynamic
physics_attr.density = 100.0
physics_attr.friction = 0.0
physics_attr.restitution = 0.0
physics_attr.position = transform.position
physics_attr.radius = circle_collider.radius
physics_attr.bcircle = true
physics_attr.bfixed_rotation = true

gplayer:add_component(PhysicsComp(physics_attr))

local gplayer2 = Entity()
local transform2 = gplayer2:add_component(Transform(vec2(50, 300), vec2(2, 2), 0))
local sprite2 = gplayer2:add_component(Sprite("characters", 32, 32, 0, 0, 1, J2D_WHITE))
sprite2:generate_uvs(736, 128)

local circle_collider2 = gplayer2:add_component(CircleCollider(8, vec2(0, 0)))
local physics_attr2 = PhysicsAttributes()
physics_attr2.etype = BodyType.Static
physics_attr2.density = 1000.0
physics_attr2.friction = 0.0
physics_attr2.restitution = 1.0
physics_attr2.position = transform2.position
physics_attr2.radius = circle_collider2.radius
physics_attr2.bcircle = true
physics_attr2.bfixed_rotation = true

gplayer2:add_component(PhysicsComp(physics_attr2))

main = {
	update = function()
		draw_rect(Rect(vec2(50, 50), vec2(100, 100), Color(200, 175, 23, 255), true))
		draw_rect(Rect(vec2(200, 50), vec2(80, 80), J2D_RED, false))

		draw_triangle(Triangle(vec2(320, 60), 100, 100, J2D_GREEN, false))
		draw_triangle(Triangle(vec2(450, 60), 100, 100, J2D_BLUE, true))

		draw_circle(Circle(vec2(620, 100), 40, J2D_MAGENTA, 64, false))
		draw_circle(Circle(vec2(720, 100), 40, J2D_YELLOW, 64, true))

		draw_polygon(Polygon({
			vec2(100, 300), vec2(160, 260), vec2(220, 300),
			vec2(200, 370), vec2(120, 370),
		}, J2D_GREEN, false))

		draw_polygon(Polygon({
			vec2(300, 300), vec2(360, 260), vec2(420, 300),
			vec2(400, 370), vec2(320, 370),
		}, J2D_WHITE, true))

		draw_line(Line(vec2(500, 260), vec2(700, 380), J2D_RED))
	end,
}
