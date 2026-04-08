---@meta

-- ---------------------------------------------------------------------------
-- Components
-- ---------------------------------------------------------------------------

---@class Transform
---@field position vec2
---@field scale    vec2
---@field rotation number
local Transform = {}

---@overload fun(): Transform
---@overload fun(position: vec2, scale: vec2, rotation: number): Transform
---@overload fun(x: number, y: number, rotation: number): Transform
---@overload fun(x: number, y: number, sx: number, sy: number, rotation: number): Transform
---@return Transform
function Transform(...) end

---@return number x, number y
function Transform:get_position() end

-- ----

---@class Sprite
---@field texture  string
---@field width    number
---@field height   number
---@field layer    integer
---@field start_x  integer
---@field start_y  integer
---@field uvs      UV
---@field hidden   boolean
---@field color    Color
local Sprite = {}

---@overload fun(): Sprite
---@overload fun(texture: string, width: number, height: number, layer: integer, start_x: integer, start_y: integer, color: Color): Sprite
---@return Sprite
function Sprite(...) end

---@param sheet_w number  Full spritesheet width in pixels
---@param sheet_h number  Full spritesheet height in pixels
function Sprite:generate_uvs(sheet_w, sheet_h) end

-- ----

---@class TextComponent
---@field font_name string
---@field text      string
---@field color     Color
---@field hidden    boolean
local TextComponent = {}

---@overload fun(): TextComponent
---@overload fun(font: string, text: string): TextComponent
---@overload fun(font: string, text: string, color: Color): TextComponent
---@return TextComponent
function TextComponent(...) end

-- ----

---@class Identification
---@field tag       string
---@field group     string
---@field entity_id integer
local Identification = {}

---@overload fun(): Identification
---@overload fun(tag: string, group: string): Identification
---@return Identification
function Identification(...) end

-- ----

---@class RigidBody
---@field velocity     vec2
---@field max_velocity vec2
local RigidBody = {}

---@overload fun(): RigidBody
---@overload fun(velocity: vec2): RigidBody
---@return RigidBody
function RigidBody(...) end

-- ----

---@class BoxCollider
---@field width  integer
---@field height integer
---@field offset vec2
---@field active boolean
local BoxCollider = {}

---@param width  integer
---@param height integer
---@param offset vec2
---@return BoxCollider
function BoxCollider(width, height, offset) end

-- ----

---@class CircleCollider
---@field radius  number
---@field offset  vec2
---@field trigger boolean
---@field active  boolean
local CircleCollider = {}

---@overload fun(): CircleCollider
---@overload fun(radius: number, offset: vec2): CircleCollider
---@return CircleCollider
function CircleCollider(...) end

-- ----

---@class Animation
---@field num_frames    integer
---@field current_frame integer
---@field frame_offset  integer
---@field frame_rate    integer
---@field vertical      boolean
---@field looped        boolean
---@field stop          boolean
local Animation = {}

---@param num_frames   integer
---@param frame_rate   integer
---@param frame_offset integer
---@param vertical     boolean
---@param looped       boolean
---@return Animation
function Animation(num_frames, frame_rate, frame_offset, vertical, looped) end

-- ---------------------------------------------------------------------------
-- Entity
-- ---------------------------------------------------------------------------

---@class Entity
local Entity = {}

---@overload fun(): Entity
---@overload fun(registry: Registry): Entity
---@overload fun(id: integer): Entity
---@return Entity
function Entity(...) end

---@generic T
---@param component T
---@return T
function Entity:add_component(component) end

---@generic T
---@param component_type T
---@return T
function Entity:get_component(component_type) end

---@param component_type any
---@return boolean
function Entity:has_component(component_type) end

---@param component_type any
---@return boolean
function Entity:remove_component(component_type) end

function Entity:destroy() end

---@return integer
function Entity:id() end

-- ---------------------------------------------------------------------------
-- runtime_view
-- ---------------------------------------------------------------------------

---@class runtime_view
local runtime_view = {}

---@param callback fun(entity: Entity)
function runtime_view:for_each(callback) end

---@param ... any  Component types to exclude
---@return runtime_view
function runtime_view:exclude(...) end

-- ---------------------------------------------------------------------------
-- Registry
-- ---------------------------------------------------------------------------

---@class Registry
local Registry = {}

---@overload fun(): Registry
---@return Registry
function Registry(...) end

---@return Entity
function Registry:create_entity() end

---@param ... any  One or more component types
---@return runtime_view
function Registry:get_entities(...) end
