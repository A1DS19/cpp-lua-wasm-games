---@meta

-- ---------------------------------------------------------------------------
-- BodyType enum
-- ---------------------------------------------------------------------------

---@class BodyType
---@field Static    integer
---@field Kinematic integer
---@field Dynamic   integer
BodyType = {}

-- ---------------------------------------------------------------------------
-- ObjectData
-- ---------------------------------------------------------------------------

---@class ObjectData
---@field tag              string
---@field group            string
---@field collider         boolean
---@field trigger          boolean
---@field is_friendly      boolean
---@field entity_id        integer
---@field contact_entities integer[]  (read-only)
local ObjectData = {}

---@overload fun(tbl: table): ObjectData
---@overload fun(tag: string, group: string, collider: boolean, trigger: boolean, is_friendly: boolean, entity_id: integer): ObjectData
---@return ObjectData
function ObjectData(...) end

-- ---------------------------------------------------------------------------
-- PhysicsAttributes
-- ---------------------------------------------------------------------------

---@class PhysicsAttributes
---@field etype           integer   BodyType value
---@field density         number
---@field friction        number
---@field restitution     number
---@field radius          number
---@field gravity_scale   number
---@field position        vec2
---@field scale           vec2
---@field box_size        vec2
---@field offset          vec2
---@field bcircle         boolean
---@field bbox_shape      boolean
---@field bfixed_rotation boolean
---@field bsensor         boolean
---@field bbullet         boolean
---@field buse_filters    boolean
---@field filter_category integer
---@field filter_mask     integer
---@field group_index     integer
---@field object_data     ObjectData
local PhysicsAttributes = {}

---@overload fun(): PhysicsAttributes
---@overload fun(tbl: table): PhysicsAttributes
---@return PhysicsAttributes
function PhysicsAttributes(...) end

-- ---------------------------------------------------------------------------
-- PhysicsComp
-- ---------------------------------------------------------------------------

---@class PhysicsComp
local PhysicsComp = {}

---@param attr PhysicsAttributes
---@return PhysicsComp
function PhysicsComp(attr) end

---@param impulse vec2
function PhysicsComp:linear_impulse(impulse) end

---@param impulse number
function PhysicsComp:angular_impulse(impulse) end

---@param velocity vec2
function PhysicsComp:set_linear_velocity(velocity) end

---@return vec2
function PhysicsComp:get_linear_velocity() end

---@param angular_velocity number
function PhysicsComp:set_angular_velocity(angular_velocity) end

---@return number
function PhysicsComp:get_angular_velocity() end

---@param gravity_scale number
function PhysicsComp:set_gravity_scale(gravity_scale) end

---@return number
function PhysicsComp:get_gravity_scale() end

---@param position vec2
function PhysicsComp:set_transform(position) end

---@param etype integer  BodyType value
function PhysicsComp:set_body_type(etype) end

---@param bullet boolean
function PhysicsComp:set_bullet(bullet) end

---@return boolean
function PhysicsComp:is_bullet() end

---@param p1 vec2
---@param p2 vec2
---@return ObjectData|nil  nil if no hit
function PhysicsComp:cast_ray(p1, p2) end

---@param lower vec2
---@param upper vec2
---@return ObjectData[]|nil  nil if no hits
function PhysicsComp:box_trace(lower, upper) end

---@return ObjectData
function PhysicsComp:object_data() end
