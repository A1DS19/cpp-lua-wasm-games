---@meta

-- ---------------------------------------------------------------------------
-- MusicPlayer
-- ---------------------------------------------------------------------------

---@class MusicPlayer
MusicPlayer = {}

---@overload fun(name: string)
---@overload fun(name: string, loops: integer)
---@param name  string   Music asset name (loaded via AssetManager)
---@param loops integer  Number of loops (-1 = infinite, default -1)
function MusicPlayer.play(name, loops) end

function MusicPlayer.stop() end
function MusicPlayer.pause() end
function MusicPlayer.resume() end

---@param volume number  0.0 – 1.0
function MusicPlayer.set_volume(volume) end

---@return boolean
function MusicPlayer.is_playing() end

-- ---------------------------------------------------------------------------
-- SoundPlayer
-- ---------------------------------------------------------------------------

---@class SoundPlayer
SoundPlayer = {}

---@overload fun(name: string)
---@overload fun(name: string, loops: integer, channel: integer)
---@param name    string   Sound-effect asset name
---@param loops   integer  Number of loops (0 = play once)
---@param channel integer  Mixer channel (-1 = first free)
function SoundPlayer.play(name, loops, channel) end

---@param channel integer
function SoundPlayer.stop(channel) end

---@param volume  number   0.0 – 1.0
---@param channel integer
function SoundPlayer.set_volume(volume, channel) end

---@param channel integer
---@return boolean
function SoundPlayer.is_playing(channel) end
