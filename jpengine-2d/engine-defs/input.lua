---@meta

-- ---------------------------------------------------------------------------
-- Keyboard
-- ---------------------------------------------------------------------------

---@class Keyboard
Keyboard = {}

---@param key integer
---@return boolean
function Keyboard.pressed(key) end

---@param key integer
---@return boolean
function Keyboard.just_pressed(key) end

---@param key integer
---@return boolean
function Keyboard.just_released(key) end

-- Typewriter keys
---@type integer
KEY_A = nil
---@type integer
KEY_B = nil
---@type integer
KEY_C = nil
---@type integer
KEY_D = nil
---@type integer
KEY_E = nil
---@type integer
KEY_F = nil
---@type integer
KEY_G = nil
---@type integer
KEY_H = nil
---@type integer
KEY_I = nil
---@type integer
KEY_J = nil
---@type integer
KEY_K = nil
---@type integer
KEY_L = nil
---@type integer
KEY_M = nil
---@type integer
KEY_N = nil
---@type integer
KEY_O = nil
---@type integer
KEY_P = nil
---@type integer
KEY_Q = nil
---@type integer
KEY_R = nil
---@type integer
KEY_S = nil
---@type integer
KEY_T = nil
---@type integer
KEY_U = nil
---@type integer
KEY_V = nil
---@type integer
KEY_W = nil
---@type integer
KEY_X = nil
---@type integer
KEY_Y = nil
---@type integer
KEY_Z = nil

-- Number keys
---@type integer
KEY_0 = nil
---@type integer
KEY_1 = nil
---@type integer
KEY_2 = nil
---@type integer
KEY_3 = nil
---@type integer
KEY_4 = nil
---@type integer
KEY_5 = nil
---@type integer
KEY_6 = nil
---@type integer
KEY_7 = nil
---@type integer
KEY_8 = nil
---@type integer
KEY_9 = nil

-- Special keys
---@type integer
KEY_ENTER = nil
---@type integer
KEY_BACKSPACE = nil
---@type integer
KEY_ESC = nil
---@type integer
KEY_SPACE = nil
---@type integer
KEY_LCTRL = nil
---@type integer
KEY_RCTRL = nil
---@type integer
KEY_LALT = nil
---@type integer
KEY_RALT = nil
---@type integer
KEY_LSHIFT = nil
---@type integer
KEY_RSHIFT = nil

-- Arrow keys
---@type integer
KEY_UP = nil
---@type integer
KEY_DOWN = nil
---@type integer
KEY_LEFT = nil
---@type integer
KEY_RIGHT = nil

-- Function keys
---@type integer
KEY_F1 = nil
---@type integer
KEY_F2 = nil
---@type integer
KEY_F3 = nil
---@type integer
KEY_F4 = nil
---@type integer
KEY_F5 = nil
---@type integer
KEY_F6 = nil
---@type integer
KEY_F7 = nil
---@type integer
KEY_F8 = nil
---@type integer
KEY_F9 = nil
---@type integer
KEY_F10 = nil
---@type integer
KEY_F11 = nil
---@type integer
KEY_F12 = nil

-- Punctuation
---@type integer
KEY_COLON = nil
---@type integer
KEY_SEMICOLON = nil
---@type integer
KEY_QUOTE = nil
---@type integer
KEY_SLASH = nil
---@type integer
KEY_BACKSLASH = nil

-- Numpad
---@type integer
KP_KEY_0 = nil
---@type integer
KP_KEY_1 = nil
---@type integer
KP_KEY_2 = nil
---@type integer
KP_KEY_3 = nil
---@type integer
KP_KEY_4 = nil
---@type integer
KP_KEY_5 = nil
---@type integer
KP_KEY_6 = nil
---@type integer
KP_KEY_7 = nil
---@type integer
KP_KEY_8 = nil
---@type integer
KP_KEY_9 = nil
---@type integer
KP_KEY_ENTER = nil
---@type integer
KP_KEY_PLUS = nil
---@type integer
KP_KEY_MINUS = nil
---@type integer
KP_KEY_MULTIPLY = nil
---@type integer
KP_KEY_DIVIDE = nil
---@type integer
KP_KEY_PERIOD = nil

-- ---------------------------------------------------------------------------
-- Mouse
-- ---------------------------------------------------------------------------

---@class Mouse
Mouse = {}

---@param button integer
---@return boolean
function Mouse.pressed(button) end

---@param button integer
---@return boolean
function Mouse.just_pressed(button) end

---@param button integer
---@return boolean
function Mouse.just_released(button) end

---@return vec2
function Mouse.screen_position() end

---@return number
function Mouse.wheel_x() end

---@return number
function Mouse.wheel_y() end

---@type integer
LEFT_BTN   = nil
---@type integer
MIDDLE_BTN = nil
---@type integer
RIGHT_BTN  = nil

-- ---------------------------------------------------------------------------
-- Gamepad
-- ---------------------------------------------------------------------------

---@class Gamepad
Gamepad = {}

---@return boolean
function Gamepad.is_gamepad_present() end

---@param button integer
---@return boolean
function Gamepad.pressed(button) end

---@param button integer
---@return boolean
function Gamepad.just_pressed(button) end

---@param button integer
---@return boolean
function Gamepad.just_released(button) end

---@param axis integer
---@return integer
function Gamepad.get_axis_position(axis) end

---@return integer
function Gamepad.get_hat_value() end

-- Face buttons
---@type integer
GP_BTN_A = nil
---@type integer
GP_BTN_B = nil
---@type integer
GP_BTN_X = nil
---@type integer
GP_BTN_Y = nil

-- System buttons
---@type integer
GP_BTN_BACK  = nil
---@type integer
GP_BTN_GUIDE = nil
---@type integer
GP_BTN_START = nil

-- Shoulder / stick buttons
---@type integer
GP_LSTICK    = nil
---@type integer
GP_RSTICK    = nil
---@type integer
GP_LSHOULDER = nil
---@type integer
GP_RSHOULDER = nil

-- D-Pad
---@type integer
DPAD_UP    = nil
---@type integer
DPAD_DOWN  = nil
---@type integer
DPAD_LEFT  = nil
---@type integer
DPAD_RIGHT = nil

-- Axes (left stick, right stick, triggers)
---@type integer
AXIS_X1 = nil
---@type integer
AXIS_Y1 = nil
---@type integer
AXIS_X2 = nil
---@type integer
AXIS_Y2 = nil
---@type integer
AXIS_Z1 = nil
---@type integer
AXIS_Z2 = nil
