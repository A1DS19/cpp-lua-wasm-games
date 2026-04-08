---@meta

-- ---------------------------------------------------------------------------
-- vec2
-- ---------------------------------------------------------------------------

---@class vec2
---@field x number
---@field y number
---@operator mul(vec2|number): vec2
---@operator div(vec2|number): vec2
---@operator add(vec2|number): vec2
---@operator sub(vec2|number): vec2
local vec2 = {}

---@overload fun(v: number): vec2
---@overload fun(x: number, y: number): vec2
---@return vec2
function vec2(...) end

---@return number
function vec2:length() end

---@return number
function vec2:length_sq() end

---@return vec2
function vec2:normalize() end

-- ---------------------------------------------------------------------------
-- vec3
-- ---------------------------------------------------------------------------

---@class vec3
---@field x number
---@field y number
---@field z number
---@operator mul(vec3|number): vec3
---@operator div(vec3|number): vec3
---@operator add(vec3|number): vec3
---@operator sub(vec3|number): vec3
local vec3 = {}

---@overload fun(v: number): vec3
---@overload fun(x: number, y: number, z: number): vec3
---@return vec3
function vec3(...) end

---@return number
function vec3:length() end

---@return number
function vec3:length_sq() end

---@return vec3
function vec3:normalize() end
