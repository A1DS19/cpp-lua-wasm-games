---@meta

-- ---------------------------------------------------------------------------
-- Color
-- ---------------------------------------------------------------------------

---@class Color
---@field r integer  (0–255)
---@field g integer  (0–255)
---@field b integer  (0–255)
---@field a integer  (0–255)
local Color = {}

---@param r integer
---@param g integer
---@param b integer
---@param a integer
---@return Color
function Color(r, g, b, a) end

---@type Color
J2D_WHITE   = nil
---@type Color
J2D_RED     = nil
---@type Color
J2D_GREEN   = nil
---@type Color
J2D_BLUE    = nil
---@type Color
J2D_BLACK   = nil
---@type Color
J2D_YELLOW  = nil
---@type Color
J2D_MAGENTA = nil

-- ---------------------------------------------------------------------------
-- UV
-- ---------------------------------------------------------------------------

---@class UV
---@field u        number
---@field v        number
---@field uv_height number
---@field uv_widht  number  (note: engine typo — matches C++ field name)
local UV = {}

---@overload fun(): UV
---@overload fun(u: number, v: number, uv_w: number, uv_h: number): UV
---@return UV
function UV(...) end
