asset_defs = {
	textures = {
		{ name = "characters", path = "assets/textures/characters.png", pixel_art = true },
	},
	fonts = {
		{ name = "pixel", path = "assets/fonts/pixel.ttf", font_size = 32.0 },
	},
	soundfxs = {
		{ name = "menu-accept", path = "assets/soundfx/menu_accept.ogg" },
	},
	music = {
		{ name = "the-field-of-dreams", path = "assets/music/the_field_of_dreams.mp3" },
	},
	shaders = {
		-- all shaders are in engine memory
	},
}

function load_assets(assets)
	for kind, entries in pairs(assets) do
		for i = 1, #entries do
			local asset = entries[i]

			if kind == "textures" then
				if not AssetManager:add_texture(asset.name, asset.path, asset.pixel_art) then
					print("failed to load texture " .. asset.name)
				end
			elseif kind == "fonts" then
				if not AssetManager:add_font(asset.name, asset.path, asset.font_size) then
					print("failed to load font " .. asset.name)
				end
			elseif kind == "soundfxs" then
				if not AssetManager:add_soundfx(asset.name, asset.path) then
					print("failed to load soundfx " .. asset.name)
				end
			elseif kind == "music" then
				if not AssetManager:add_music(asset.name, asset.path) then
					print("failed to load music " .. asset.name)
				end
			end
		end
	end
end
