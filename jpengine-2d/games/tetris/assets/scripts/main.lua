j2d_run_script("assets/scripts/defs/asset-defs.lua")
j2d_run_script("assets/scripts/defs/level-defs.lua")
j2d_run_script("assets/scripts/block.lua")
j2d_run_script("assets/scripts/grid.lua")
j2d_run_script("assets/scripts/utils.lua")
j2d_run_script("assets/scripts/tetromino.lua")
j2d_run_script("assets/scripts/game.lua")

load_assets(asset_defs)

ggame = Game:create()

main = {
	update = function()
		ggame:update()
	end,
}
