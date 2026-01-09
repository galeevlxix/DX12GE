--[[
    This is the core.lua file which defines basic logic of each of the basic entities, which are:
    1) GameObject - the base class of each scene entinty, it contains logic for interaction which this entetyes, events thacan be used to make tha logic and basic parametres
    ....

    'Class' function is helper function that makes inheritance a lot more easier, u just use this special word to create new class:

    Class(SampleChild, SampleParent) - after using this function the SampleChild will inheritance all functions from SampleParent!
    if SampleParent contains function:

    function SampleParent:DoFoo(a, b, c)
    ---some logic
    end

    The SampleChild will gain that function with all his logic, inputs and outputs! Later you can easily use it in events with syntax: 
    SampleChild:DoFoo(a, b, c)

    NOTES:
    You are able to redefine that function in your SampleChild, by that syntax:

    function SampleChild:DoFoo(a, b 'or Any othe params')
    ---some new logic
    end

    After doing that it is recommended to push at the end of new function return call to the original SampleParent function:

    function SampleChild:DoFoo(a, b 'or Any othe params')
    ---some new logic
        return SampleParent.DoFoo(self, a, b, c)
    end

    find helper info on https://habr.com/ru/articles/259265/
end

]]

Forward = {
	0, 0, 1
}

Backward = {
    0, 0, -1
}

Left = {
    1, 0, 0
}

Right = {
    -1, 0, 0
}

Up = {
    0, 1, 0
}

Down = {
    0, -1, 0
}

KeyCode = {
    -- Кнопки мыши
    ["mouse_left"] = 1,
    ["mouse_right"] = 2,
    ["mouse_middle"] = 4,
    ["mouse_xbutton1"] = 5,
    ["mouse_xbutton2"] = 6,
    
    -- Специальные клавиши
    ["escape"] = 27,
    ["backspace"] = 8,
    ["tab"] = 9,
    ["enter"] = 13,
    ["space"] = 32,
    ["capslock"] = 20,
    ["numlock"] = 144,
    ["scrolllock"] = 145,
    ["pause"] = 19,
    ["cancel"] = 3,
    
    -- Модификаторы
    ["shift"] = 16,
    ["lshift"] = 160,
    ["rshift"] = 161,
    ["ctrl"] = 17,
    ["lctrl"] = 162,
    ["rctrl"] = 163,
    ["alt"] = 18,
    ["lalt"] = 164,
    ["ralt"] = 165,
    ["lwin"] = 91,
    ["rwin"] = 92,
    ["context_menu"] = 93,
    ["sleep"] = 95,
    
    -- Навигация
    ["home"] = 36,
    ["end"] = 35,
    ["pageup"] = 33,
    ["pagedown"] = 34,
    ["clear"] = 12,
    ["select"] = 41,
    ["print"] = 42,
    ["execute"] = 43,
    ["printscreen"] = 44,
    ["insert"] = 45,
    ["delete"] = 46,
    ["help"] = 47,
    
    -- Стрелки
    ["left"] = 37,
    ["up"] = 38,
    ["right"] = 39,
    ["down"] = 40,
    
    -- Цифры
    ["0"] = 48,
    ["1"] = 49,
    ["2"] = 50,
    ["3"] = 51,
    ["4"] = 52,
    ["5"] = 53,
    ["6"] = 54,
    ["7"] = 55,
    ["8"] = 56,
    ["9"] = 57,
    
    -- Буквы (A-Z)
    ["a"] = 65,
    ["b"] = 66,
    ["c"] = 67,
    ["d"] = 68,
    ["e"] = 69,
    ["f"] = 70,
    ["g"] = 71,
    ["h"] = 72,
    ["i"] = 73,
    ["j"] = 74,
    ["k"] = 75,
    ["l"] = 76,
    ["m"] = 77,
    ["n"] = 78,
    ["o"] = 79,
    ["p"] = 80,
    ["q"] = 81,
    ["r"] = 82,
    ["s"] = 83,
    ["t"] = 84,  -- Исправлено: должно быть 84 вместо 83
    ["u"] = 85,
    ["v"] = 86,
    ["w"] = 87,
    ["x"] = 88,
    ["y"] = 89,
    ["z"] = 90,
    
    -- Цифровая клавиатура
    ["numpad0"] = 96,
    ["numpad1"] = 97,
    ["numpad2"] = 98,
    ["numpad3"] = 99,
    ["numpad4"] = 100,
    ["numpad5"] = 101,
    ["numpad6"] = 102,
    ["numpad7"] = 103,
    ["numpad8"] = 104,
    ["numpad9"] = 105,
    ["numpad_multiply"] = 106,
    ["numpad_add"] = 107,
    ["numpad_separator"] = 108,
    ["numpad_subtract"] = 109,
    ["numpad_decimal"] = 110,
    ["numpad_divide"] = 111,
    ["numpad_enter"] = 13,
    
    -- Функциональные клавиши
    ["f1"] = 112,
    ["f2"] = 113,
    ["f3"] = 114,
    ["f4"] = 115,
    ["f5"] = 116,
    ["f6"] = 117,
    ["f7"] = 118,
    ["f8"] = 119,
    ["f9"] = 120,
    ["f10"] = 121,
    ["f11"] = 122,
    ["f12"] = 123,
    ["f13"] = 124,
    ["f14"] = 125,
    ["f15"] = 126,
    ["f16"] = 127,
    ["f17"] = 128,
    ["f18"] = 129,
    ["f19"] = 130,
    ["f20"] = 131,
    ["f21"] = 132,
    ["f22"] = 133,
    ["f23"] = 134,
    ["f24"] = 135,
    
    -- Браузер и медиа клавиши
    ["browser_back"] = 166,
    ["browser_forward"] = 167,
    ["browser_refresh"] = 168,
    ["browser_stop"] = 169,
    ["browser_search"] = 170,
    ["browser_favorites"] = 171,
    ["browser_home"] = 172,
    ["volume_mute"] = 173,
    ["volume_down"] = 174,
    ["volume_up"] = 175,
    ["media_next_track"] = 176,
    ["media_previous_track"] = 177,
    ["media_stop"] = 178,
    ["media_play_pause"] = 179,
    ["launch_mail"] = 180,
    ["launch_media_select"] = 181,
    ["launch_app1"] = 182,
    ["launch_app2"] = 183,
    
    -- Символьные клавиши (OEM клавиши)
    ["oem_1"] = 186,           -- ; :
    ["oem_plus"] = 187,        -- = +
    ["oem_comma"] = 188,       -- , <
    ["oem_minus"] = 189,       -- - _
    ["oem_period"] = 190,      -- . >
    ["oem_2"] = 191,           -- / ?
    ["oem_3"] = 192,           -- `
    ["oem_4"] = 219,           -- [ {
    ["oem_5"] = 220,           -- \ |
    ["oem_6"] = 221,           -- ] }
    ["oem_7"] = 222,           -- ' "
    
    -- Дополнительные (если нужны)
    ["abnt_c1"] = 193,
    ["abnt_c2"] = 194,
}

Transform   = "transfrom"
Physics     = "physics"


function Class(child, parent)
    setmetatable(child, {__index = parent})
end

GameObject = {}
GameObject.__index = GameObject

function GameObject:new( id )
    local instance = {}
    instance.entity = nil
    instance.id = id
    instance.components = {}
    instance.object = nil
    instance.transform = nil


    setmetatable(instance, self)
    self.__index = self;
    return instance
end

function GameObject:SetEntityName( entity )
    self.entity = entity
end

function GameObject:Start()
    print("Start for instance.id ", self.id)
    self.object = GetObjectOnScene(self.entity)
    if  self.transform ~= nil and self.object ~= nil then
        self.transform:SetParent(self.object )
    end
end

function GameObject:Update()
     --   for _, component in ipairs(self.components) do
     --       component:Update()
     --   end
end

function GameObject:Remove()
       -- print("Remove")
end

function GameObject:OnCollisionEnter( other )
      --  print("collision")
end

function GameObject:OnMouseMovementInputReceived( x, y )
     --   print("input mouse")
end

function GameObject:OnMouseWheelInput( y )
     --   print("input wheel")
end

function GameObject:OnKeyBoardInput( k, pressed )
	  --  print("keyboard", k)
end

function GameObject:OnMouseClickInput( k, pressed )
    --    print("click")
end

function GameObject:AddComponent( component )
    local componentName = string.lower(component)
        
    if not self.components then self.components = {} end

    if componentName == Transform then
        print("component added for id ", self.id)
        self.transform = TransformComponent:Add( self.id )
        self.components[self.transform] = self.transform
    end

    if componentName == Physics then
        self.components[#self.components] = TransformComponent:Add( self.object )
        self.transform = self.components[#self.components - 1]
    end
end

TransformComponent = {}
TransformComponent.__index = TransformComponent

function TransformComponent:Add( id )
    print("transform added")
	local instance = {}
    instance.id = id
    instance.object = nil

    setmetatable(instance, self)
    self.__index = self; 
    return instance
end

function TransformComponent:MoveToVector( table )
    assert(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
    TranslateTo(self.object, table[1], table[2], table[3])
end

function TransformComponent:MoveByVector( table )
   -- assert(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
    TranslateBy(self.object, table[1], table[2], table[3])
end

function TransformComponent:MoveToPos( x, y, z )
    assert(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
    TranslateTo(self.object, x, y, z)
end

function TransformComponent:MoveByPos( x, y, z )       
    assert(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")     
    TranslateBy(self.object, x, y, z)
end

function TransformComponent:SetParent( parent )
    assert(parent ~= nil, "Attemp to call SetParent to on empty object, call SetParent to set object!") 
    print("set parent for transform") 
    self.object = parent
end

function TransformComponent:RotateByVector( table )   
    RotateBy(self.object, table[1], table[2], table[3])
end

function TransformComponent:RotateByCoords( x, y, z ) 
    RotateBy(self.object, x, y, z)
end

function TransformComponent:Update()
    print("transform update")
end

function TransformComponent:GetPosition( )
    if self.object ~= nil then
        return GetTransfromPosition(self.object)
    end
end

PhysicsComponent = {}


