--[[
Core.lua — Core logic for game entities in the Lua engine.
Contains:
1. Base GameObject class with lifecycle methods.
2. Helper structures: Vector3, Rotator.
3. Components: TransformComponent, PhysicsComponent.
4. Inheritance utilities, global constants, and key codes.
5. Full description of all C++ ? Lua API functions registered via sol2.

Automatically loaded by the engine on startup.
]]

-- ===================================================================
-- TYPES AND STRUCTURES
-- ===================================================================

--- Creates a 3D vector as a table with type "vec3".
---@param x number X coordinate
---@param y number Y coordinate
---@param z number Z coordinate
---@return table vector3 Table { [1]=x, [2]=y, [3]=z, type="vec3" }
function Vector3(x, y, z)
    local vector3 = {}
    vector3[1] = x
    vector3[2] = y
    vector3[3] = z
    vector3.type = "vec3"
    return vector3
end

--- Creates a rotator (Euler angles) as a table with type "rot".
---@param yaw   number Rotation around Y axis (left/right)
---@param pitch number Rotation around X axis (up/down)
---@param roll  number Rotation around Z axis (tilt)
---@return table rotator Table { [1]=yaw, [2]=pitch, [3]=roll, type="rot" }
function Rotator(yaw, pitch, roll)
    local rotator = {}
    rotator[1] = yaw
    rotator[2] = pitch
    rotator[3] = roll
    rotator.type = "rot"
    return rotator
end

--- Custom assertion utility that prints an error message if condition fails.
---@param condition boolean Condition that must be true
---@param text      string Error message to display
---@return boolean true if condition passed, false otherwise
function assert_check(condition, text)
    if not condition then
        print("Assertion Failed:")
        print(text)
        return false
    end
    return true
end

-- ===================================================================
-- GLOBAL CONSTANTS
-- ===================================================================

-- Direction vectors
Forward  = Vector3(0, 0, 1)
Backward = Vector3(0, 0, -1)
Left     = Vector3(-1, 0, 0)
Right    = Vector3(1, 0, 0)
Up       = Vector3(0, 1, 0)
Down     = Vector3(0, -1, 0)

-- Key codes (match C++ KeyCode::Key enum)
KeyCode = {
    ["mouse_left"]   = 1,
    ["mouse_right"]  = 2,
    ["mouse_middle"] = 4,
    ["space"]        = 32,
    ["w"]            = 87,
    ["a"]            = 65,
    ["s"]            = 83,
    ["d"]            = 68,
    -- ... other keys (F1–F12 etc.) ...
    ["f12"]          = 123,
}

-- Component names (note: "transform" was misspelled as "transfrom" in older versions)
Transform = "transform"
Physics   = "physics"

-- Scene node types
NODE_TYPE_NODE3D               = 0
NODE_TYPE_OBJECT3D             = 1
NODE_TYPE_FIRST_PERSON_PLAYER  = 2
NODE_TYPE_THIRD_PERSON_PLAYER  = 3
NODE_TYPE_CAMERA               = 4
NODE_TYPE_ENVIRONMENT          = 5
NODE_TYPE_DIRECTIONAL_LIGHT    = 6
NODE_TYPE_POINT_LIGHT          = 7
NODE_TYPE_SPOT_LIGHT           = 8
NODE_TYPE_PARTICLES            = 9
NODE_TYPE_SKYBOX               = 10
NODE_TYPE_AI                   = 11
NODE_TYPE_PHYSICAL_OBJECT3D    = 12
NODE_TYPE_AUDIO_LISTENER       = 13
NODE_TYPE_AUDIO_EMITTER        = 14

-- ===================================================================
-- INHERITANCE SYSTEM
-- ===================================================================

--- Implements simple inheritance using Lua metatables.
---@param child  table Child class (table)
---@param parent table Parent class (table)
function Class(child, parent)
    setmetatable(child, { __index = parent })
end

-- ===================================================================
-- BASE CLASS: GameObject
-- ===================================================================

GameObject = {}
GameObject.__index = GameObject

--- GameObject constructor.
---@param id string Unique script identifier
---@return table instance New GameObject instance
function GameObject:new(id)
    local instance = {}
    instance.entity = nil
    instance.id = id
    instance.components = {}
    instance.object = nil
    instance.transform = nil
    instance.physics = nil
    setmetatable(instance, self)
    self.__index = self
    return instance
end

--- Binds the Lua object to a C++ scene entity by name.
---@param entity string Name of the object in the scene
function GameObject:SetEntityName(entity)
    self.entity = entity
end

--- Returns the parent of this object (C++ Node3D*).
---@return userdata|nil Parent Node3D pointer or nil
function GameObject:GetParent()
    assert_check(self.object ~= nil, "Attempt to call GetParent on nil object")
    local parent = GetParent(self.object)
    return parent
end

--- Returns a named child of this object (C++ Node3D*).
---@param name string Name of the child node
---@return userdata|nil Child Node3D pointer or nil
function GameObject:GetChild(name)
    assert_check(self.object ~= nil, "Attempt to call GetChild on nil object")
    local child = GetChild(self.object, name)
    return child
end

--- Called once during object initialization.
function GameObject:Start()
    print("Start for instance.id", self.id)
    self.object = GetObjectOnScene(self.entity)
    if self.transform and self.object then
        self.transform:SetParent(self.object)
        print("Set parent for transform")
    else
        print("Failed to set parent for transform:", self.transform ~= nil, self.object ~= nil)
    end
    if self.physics and self.object then
        self.physics:SetParent(self.object)
        print("Set parent for physics")
    else
        print("Failed to set parent for physics:", self.physics ~= nil, self.object ~= nil)
    end
end

--- Called every frame (Tick).
function GameObject:Update()
    -- Override in subclasses
end

--- Called when the object is destroyed.
function GameObject:Remove()
    -- Stub
end

--- Triggered when entering a collision.
---@param other userdata C++ pointer to the colliding object
function GameObject:OnCollisionEnter(other)
    -- Stub
end

--- Handles mouse movement input.
---@param x number Mouse X coordinate
---@param y number Mouse Y coordinate
function GameObject:OnMouseMovementInputReceived(x, y)
    -- Stub
end

--- Handles mouse wheel input.
---@param delta number Scroll amount/direction
function GameObject:OnMouseWheelInput(delta)
    -- Stub
end

--- Handles keyboard input.
---@param key_code number Key code (see KeyCode table)
---@param pressed  boolean true if pressed, false if released
function GameObject:OnKeyBoardInput(key_code, pressed)
    -- Stub
end

--- Handles mouse button clicks.
---@param button  number Button code (e.g., KeyCode.mouse_left)
---@param pressed boolean true if pressed, false if released
function GameObject:OnMouseClickInput(button, pressed)
    -- Stub
end

--- Adds a component to the object.
---@param component_name string "transform" or "physics"
function GameObject:AddComponent(component_name)
    local name = string.lower(component_name)
    if not self.components then self.components = {} end

    if name == Transform then
        print("Added Transform component to", self.id)
        self.transform = TransformComponent:Add(self.id)
        self.components["transform"] = self.transform
    end

    if name == Physics then
        print("Added Physics component to", self.id)
        self.physics = PhysicsComponent:Add(self.id)
        self.components["physics"] = self.physics
    end
end

-- ===================================================================
-- COMPONENT: TransformComponent
-- ===================================================================

TransformComponent = {}
TransformComponent.__index = TransformComponent

--- Creates a new transform component.
---@param id string/userdata Owner ID
---@return table instance TransformComponent instance
function TransformComponent:Add(id)
    local instance = {}
    instance.id = id
    instance.object = nil
    setmetatable(instance, self)
    self.__index = self
    return instance
end

--- Sets the C++ parent object (Node3D*).
---@param parent userdata C++ Node3D pointer
function TransformComponent:SetParent(parent)
    assert_check(parent ~= nil, "SetParent called with nil parent!")
    self.object = parent
    print("Transform parent set")
end

--- Moves object to absolute world coordinates (via vector).
---@param vec table Vector of type "vec3"
function TransformComponent:MoveToVector(vec)
    assert_check(self.object ~= nil, "MoveToVector: object not set! Call SetParent first.")
    assert_check(vec.type == "vec3", "MoveToVector expects a 'vec3' type vector.")
    TranslateTo(self.object, vec[1], vec[2], vec[3])
end

--- Moves object relative to current position (via vector).
---@param vec table Vector of type "vec3"
function TransformComponent:MoveByVector(vec)
    assert_check(self.object ~= nil, "MoveByVector: object not set! Call SetParent first.")
    assert_check(vec.type == "vec3", "MoveByVector expects a 'vec3' type vector.")
    TranslateBy(self.object, vec[1], vec[2], vec[3])
end

--- Moves object to absolute world coordinates (by components).
---@param x number
---@param y number
---@param z number
function TransformComponent:MoveToPos(x, y, z)
    assert_check(self.object ~= nil, "MoveToPos: object not set! Call SetParent first.")
    TranslateTo(self.object, x, y, z)
end

--- Moves object relative to current position (by components).
---@param x number
---@param y number
---@param z number
function TransformComponent:MoveByPos(x, y, z)
    assert_check(self.object ~= nil, "MoveByPos: object not set! Call SetParent first.")
    TranslateBy(self.object, x, y, z)
end

--- Rotates object by Euler angles (via rotator vector).
---@param rot table Rotator of type "rot"
function TransformComponent:RotateByVector(rot)
    assert_check(self.object ~= nil, "RotateByVector: object not set! Call SetParent first.")
    assert_check(rot.type == "rot", "RotateByVector expects a 'rot' type rotator.")
    RotateBy(self.object, rot[1], rot[2], rot[3])
end

--- Rotates object by Euler angles (by components: yaw, pitch, roll).
---@param yaw   number
---@param pitch number
---@param roll  number
function TransformComponent:RotateByCoords(yaw, pitch, roll)
    RotateBy(self.object, yaw, pitch, roll)
end

--- Gets current world position of the object.
---@return table|nil {x=..., y=..., z=...} or nil
function TransformComponent:GetPosition()
    if self.object then
        return GetTransfromPosition(self.object)
    end
    return nil
end

--- Component update hook (stub).
function TransformComponent:Update()
    -- Can be used for custom logic
end

-- ===================================================================
-- COMPONENT: PhysicsComponent
-- ===================================================================

PhysicsComponent = {}
PhysicsComponent.__index = PhysicsComponent

--- Creates a new physics component.
---@param id string/userdata Owner ID
---@return table instance PhysicsComponent instance
function PhysicsComponent:Add(id)
    local instance = {}
    instance.id = id
    instance.object = nil
    instance.on = false
    setmetatable(instance, self)
    self.__index = self
    return instance
end

--- Sets the C++ parent object (PhysicalObjectNode*).
---@param parent userdata C++ PhysicalObjectNode pointer
function PhysicsComponent:SetParent(parent)
    assert_check(parent ~= nil, "SetParent called with nil parent!")
    self.object = parent
    self.on = true
end

--- Applies an impulse to the object.
---@param vec table Vector of type "vec3"
function PhysicsComponent:AddImpulse(vec)
    assert_check(self.object ~= nil, "AddImpulse: object not set! Call SetParent first.")
    assert_check(self.on, "AddImpulse: physics not active!")
    AddImpulse(self.object, vec[1], vec[2], vec[3])
end

--- Gets current velocity of the object.
---@return table|nil {x=..., y=..., z=...}
function PhysicsComponent:GetVelocity()
    assert_check(self.object ~= nil, "GetVelocity: object not set! Call SetParent first.")
    assert_check(self.on, "GetVelocity: physics not active!")
    return GetVelocity(self.object)
end

-- ===================================================================
-- C++ ? LUA API (REGISTERED FUNCTIONS)
-- ===================================================================
--[[
The following functions are exported from C++ via sol2 and available in Lua:

| Lua Function Name       | C++ Function                     | Description |
|-------------------------|----------------------------------|-------------|
| GetObjectOnScene        | lua_get_object_on_scene         | Returns Node3D* by scene entity name |
| GetTransfromPosition    | lua_get_object_pos              | Returns object position as {x,y,z} |
| TranslateTo             | lua_transform_move_to           | Sets absolute world position |
| TranslateBy             | lua_transform_move_by           | Moves object relative to current position |
| RotateBy                | lua_rotate_object_by_rotator    | Rotates object by Euler angles (yaw, pitch, roll) |
| GetVelocity             | lua_get_velocity                | Returns velocity of a physical object |
| AddImpulse              | lua_add_impulse                 | Applies linear impulse to a physical object |
| GetNodeType             | lua_get_node_type               | Returns node type as integer |
| GetWorldDirection       | lua_get_object_world_direction  | Returns forward direction in world space |
| GetChild                | lua_get_child                   | Returns child node by name |
| GetParent               | lua_get_parent                  | Returns parent node |
| CallError               | lua_call_assert                 | Prints error message to C++ console |
| Register                | lua_register_class              | Registers a Lua class in the active list |
| LoadObjectWithModel     | lua_load_object_with_model      | Loads a model and auto-creates a Lua object |
| GetClass                | get_lua_class                   | Returns Lua class table by name (rarely used) |

Examples:
  local obj = GetObjectOnScene("Player")
  TranslateTo(obj, 10, 0, 5)
  local pos = GetTransfromPosition(obj)  --> {x=10, y=0, z=5}
]]

print("Core system loaded successfully.")