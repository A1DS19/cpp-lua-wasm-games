---@meta

-- ---------------------------------------------------------------------------
-- Camera
-- ---------------------------------------------------------------------------

---@class Camera
Camera = {}

---@return Camera
function Camera.get() end

---@param x number
---@param y number
function Camera.set_position(x, y) end

---@param scale number
function Camera.set_scale(scale) end

---@return number
function Camera.get_scale() end

---@return number x, number y
function Camera.get_position() end

-- ---------------------------------------------------------------------------
-- Timer
-- ---------------------------------------------------------------------------

---@class Timer
local Timer = {}

---@return Timer
function Timer() end

function Timer:start() end
function Timer:stop() end
function Timer:resume() end
function Timer:pause() end

---@return boolean
function Timer:is_running() end

---@return boolean
function Timer:is_paused() end

---@return number  Elapsed time in milliseconds
function Timer:elapsed_ms() end

---@return number  Elapsed time in seconds
function Timer:elapsed_sec() end

-- ---------------------------------------------------------------------------
-- Global functions
-- ---------------------------------------------------------------------------

---@param path string  Path to the Lua script file
---@return boolean     true on success, false on error
function j2d_run_script(path) end

---@param scripts string[]  List of script file paths to load in order
function j2d_load_script_table(scripts) end

---@return integer  Milliseconds since engine start (SDL_GetTicks)
function J2D_GetTicks() end

---@param text     string  Text to measure
---@param fontname string  Font asset name
---@return number          Width in pixels
function j2d_measure_text(text, fontname) end

---@param text      string  Text to align
---@param fontname  string  Font asset name
---@param align_pos vec2    Reference position (right edge)
---@return number           X position so the text ends at align_pos.x
function j2d_right_align_text(text, fontname, align_pos) end

---@param text      string  Text to align
---@param fontname  string  Font asset name
---@param align_pos vec2    Reference position (center)
---@return number           X position so the text is centered at align_pos.x
function j2d_center_align_text(text, fontname, align_pos) end
