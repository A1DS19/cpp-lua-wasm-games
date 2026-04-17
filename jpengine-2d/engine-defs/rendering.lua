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

-- ---------------------------------------------------------------------------
-- Shapes
-- ---------------------------------------------------------------------------

---@class Rect
---@field position   vec2
---@field size       vec2
---@field color      Color
---@field bwireframe boolean
local Rect = {}

---@param position   vec2
---@param size       vec2
---@param color      Color
---@param bwireframe boolean
---@return Rect
function Rect(position, size, color, bwireframe) end

---@class Circle
---@field center     vec2
---@field radius     number
---@field color      Color
---@field segments   integer
---@field bwireframe boolean
local Circle = {}

---@param center     vec2
---@param radius     number
---@param color      Color
---@param segments   integer
---@param bwireframe boolean
---@return Circle
function Circle(center, radius, color, segments, bwireframe) end

---@class Triangle
---@field position   vec2
---@field base       number
---@field height     number
---@field color      Color
---@field bwireframe boolean
local Triangle = {}

---@param position   vec2
---@param base       number
---@param height     number
---@param color      Color
---@param bwireframe boolean
---@return Triangle
function Triangle(position, base, height, color, bwireframe) end

---@class Polygon
---@field points     vec2[]
---@field color      Color
---@field bwireframe boolean
local Polygon = {}

---@param points     vec2[]
---@param color      Color
---@param bwireframe boolean
---@return Polygon
function Polygon(points, color, bwireframe) end

---@class Line
---@field points vec2[]   (two points)
---@field color  Color
local Line = {}

---@param p1    vec2
---@param p2    vec2
---@param color Color
---@return Line
function Line(p1, p2, color) end

-- ---------------------------------------------------------------------------
-- Shape draw submitters (enqueue for the current frame)
-- ---------------------------------------------------------------------------

---@param rect Rect
function draw_rect(rect) end

---@param circle Circle
function draw_circle(circle) end

---@param triangle Triangle
function draw_triangle(triangle) end

---@param polygon Polygon
function draw_polygon(polygon) end

---@param line Line
function draw_line(line) end
