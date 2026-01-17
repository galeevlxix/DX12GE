--[[
    Core.lua определяет базовую логику сущностей:
    1) GameObject - базовый класс, содержащий логику взаимодействия, события и параметры.
    2) Вспомогательные функции и структуры (Vector3, Rotator).
    3) Компоненты (TransformComponent).

    Функция 'Class' реализует наследование.
    Пример: Class(SampleChild, SampleParent)
]]

--TYPES:

---Создает структуру вектора.
---@param x number Координата X
---@param y number Координата Y
---@param z number Координата Z
---@return table vector3 Таблица с полями [1], [2], [3] и типом "vec3"
function Vector3(x, y, z)
    local vector3 = {}
    vector3[1] = x
    vector3[2] = y
    vector3[3] = z
    vector3.type = "vec3"

    return vector3
end

---Создает структуру вращения (Rotator).
---@param y number Yaw (вращение вокруг Y)
---@param p number Pitch (вращение вокруг X)
---@param r number Roll (вращение вокруг Z)
---@return table rotator Таблица с углами и типом "rot"
function Rotator(y, p, r)
    local rotator = {}
    rotator[1] = y -- Исправлено: в оригинале было x, должно быть y (Yaw)
    rotator[2] = p -- Pitch
    rotator[3] = r -- Roll (в оригинале было z)
    rotator.type = "rot"

    return rotator
end

---Пользовательская проверка утверждений (assert).
---Выводит ошибку в консоль, если условие ложно.
---@param condition boolean Условие для проверки
---@param text string Текст ошибки
---@return boolean Результат проверки
function assert_check( condition, text )
	if not condition then
        print("Assertation Failed")
        print(text)
        return false
    end
    return true
end


NodeType = {
    ""
}

-- Глобальные векторы направлений
Forward = { 0, 0, 1 }
Backward = { 0, 0, -1 }
Left = { 1, 0, 0 }
Right = { -1, 0, 0 }
Up = { 0, 1, 0 }
Down = { 0, -1, 0 }

-- Коды клавиш для обработки ввода (соответствуют C++ KeyCode)
KeyCode = {
    ["mouse_left"] = 1,
    ["mouse_right"] = 2,
    ["mouse_middle"] = 4,
    -- ... (список клавиш пропущен для краткости, он остается прежним) ...
    ["f12"] = 123,
    ["space"] = 32,
    ["w"] = 87,
    ["a"] = 65,
    ["s"] = 83,
    ["d"] = 68,
}

-- Имена компонентов
Transform   = "transfrom" -- (Примечание: в коде опечатка 'transfrom', сохранена для совместимости)
Physics     = "physics"


---Реализует наследование классов через метатаблицы Lua.
---@param child table Таблица-наследник
---@param parent table Таблица-родитель
function Class(child, parent)
    setmetatable(child, {__index = parent})
end

-- Базовый класс игрового объекта
GameObject = {}
GameObject.__index = GameObject

---Конструктор GameObject.
---@param id string Уникальный идентификатор объекта (обычно имя)
---@return table instance Новый экземпляр класса
function GameObject:new( id )
    local instance = {}
    instance.entity = nil      -- Имя сущности в движке
    instance.id = id           -- ID скриптового объекта
    instance.components = {}   -- Список компонентов
    instance.object = nil      -- C++ userdata (указатель Node3D*)
    instance.transform = nil   -- Ссылка на компонент трансформации

    setmetatable(instance, self)
    self.__index = self;
    return instance
end

---Устанавливает имя сущности для связывания с C++ объектом.
---@param entity string Имя объекта на сцене
function GameObject:SetEntityName( entity )
    self.entity = entity
end

---Инициализация объекта. Вызывается движком при старте.
---Связывает Lua-объект с C++ объектом через API функцию GetObjectOnScene.
function GameObject:Start()
    print("Start for instance.id ", self.id)
    -- GetObjectOnScene определена в C++ (LuaManager.cpp), возвращает Node3D* (userdata)
    self.object = GetObjectOnScene(self.entity)
    
    if self.transform ~= nil and self.object ~= nil then
        -- Передаем C++ указатель в компонент трансформации
        self.transform:SetParent(self.object )
        print("set parent")
    else
        print("not set parent", self.transform ~= nil)
        print("self.object != nil  ", self.object)
    end

    if self.physics ~= nil and self.object ~= nil then
        -- Передаем C++ указатель в компонент трансформации
        self.physics:SetParent(self.object )
        print("set parent")
    else
        print("not set parent", self.transform ~= nil)
        print("self.object != nil  ", self.object)
    end
end

---Метод обновления, вызывается каждый кадр (Tick).
---Должен быть переопределен в наследниках.
function GameObject:Update()
     --   for _, component in ipairs(self.components) do
     --       component:Update()
     --   end
end

---Вызывается при удалении объекта.
function GameObject:Remove()
       -- print("Remove")
end

---Событие входа в коллизию (физика).
---@param other userdata Объект, с которым произошло столкновение
function GameObject:OnCollisionEnter( other )
      --  print("collision")
end

---Обработчик движения мыши.
---Вызывается из C++ LuaManager::ProceedMouseMovementInput.
---@param x number Координата X мыши
---@param y number Координата Y мыши
function GameObject:OnMouseMovementInputReceived( x, y )
     --   print("input mouse")
end

---Обработчик колесика мыши.
---Вызывается из C++ LuaManager::ProceedMouseWheelInput.
---@param y number Значение прокрутки
function GameObject:OnMouseWheelInput( y )
     --   print("input wheel")
end

---Обработчик нажатия клавиш клавиатуры.
---Вызывается из C++ LuaManager::ProceedKeyBoardInput.
---@param k number Код клавиши (см. таблицу KeyCode)
---@param pressed boolean True, если нажата, False, если отпущена
function GameObject:OnKeyBoardInput( k, pressed )
	  --  print("keyboard", k)
end

---Обработчик кликов мыши.
---Вызывается из C++ LuaManager::ProceedMouseClickInput.
---@param k number Код кнопки мыши
---@param pressed boolean Состояние нажатия
function GameObject:OnMouseClickInput( k, pressed )
    --    print("click")
end

---Добавляет компонент к объекту.
---@param component string Имя компонента ("transfrom" или "physics")
function GameObject:AddComponent( component )
    local componentName = string.lower(component)
        
    if not self.components then self.components = {} end

    -- Добавление Transform
    if componentName == Transform then
        print("component added for id ", self.id)
        self.transform = TransformComponent:Add( self.id )
        self.components[self.transform] = self.transform
    end

    -- Добавление Physics (логика выглядит неполной в оригинале, но задокументирована как есть)
    if componentName == Physics then
        -- Внимание: здесь используется self.object, который может быть nil до вызова Start()
        self.components[#self.components] = PhysicsComponent:Add( self.id )
        self.transform = self.components[#self.components - 1]
    end
end

-- Класс компонента трансформации (Позиция, Вращение)
TransformComponent = {}
TransformComponent.__index = TransformComponent

---Создает новый компонент трансформации.
---@param id string/userdata ID владельца или объект
---@return table instance Экземпляр TransformComponent
function TransformComponent:Add( id )
    print("transform added")
	local instance = {}
    instance.id = id
    instance.object = nil -- Здесь будет храниться C++ Node3D*

    setmetatable(instance, self)
    self.__index = self; 
    return instance
end

---Перемещает объект в абсолютные координаты, используя вектор-таблицу.
---@param table table Таблица {x, y, z}
function TransformComponent:MoveToVector( vec )
    assert_check(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
    -- Проверка на тип vec3
    assert_check(vec.type == "vec3", "ERROR : You called MoveToVector, yet you call with NO vec3")

    TranslateTo(self.object, vec[1], vec[2], vec[3])
end

---Перемещает объект относительно текущей позиции (сдвиг), используя вектор-таблицу.
---@param table table Таблица {x, y, z} с типом "vec3"
function TransformComponent:MoveByVector( table )
    assert_check(self.object ~= nil, "ERROR : Attemp to call move to on empty object, call SetParent to set object!")
    -- Проверка на тип vec3 (как и просили, аналогично MoveByVector)
    assert_check(vec.type == "vec3", "ERROR : You called move by Vector, yet you call with NO vec3")

    TranslateBy(self.object, vec[1], vec[2], vec[3])
end

---Перемещает объект в абсолютные координаты (покомпонентно).
---@param x number
---@param y number
---@param z number
function TransformComponent:MoveToPos( x, y, z )
    assert_check(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
    -- Вызов C++ функции: lua_transform_move_to
    TranslateTo(self.object, x, y, z)
end

---Перемещает объект относительно текущей позиции (покомпонентно).
---@param x number Сдвиг по X
---@param y number Сдвиг по Y
---@param z number Сдвиг по Z
function TransformComponent:MoveByPos( x, y, z )       
   assert_check(self.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")   
   
   -- Вызов C++ функции: lua_transform_move_by
   TranslateBy(self.object, x, y, z)
end

---Устанавливает родительский C++ объект для компонента.
---Необходимо вызвать перед использованием функций перемещения.
---@param parent userdata Указатель на Node3D (из C++)
function TransformComponent:SetParent( parent )
    assert_check(parent ~= nil, "Attemp to call SetParent to on empty object, call SetParent to set object!") 
    self.object = parent
end

---Вращает объект, используя вектор-таблицу.
---@param table table Таблица {y, p, r} (Yaw, Pitch, Roll)
function TransformComponent:RotateByVector( table )   
    assert_check(self.object ~= nil, "ERROR: Attempt to call rotate on empty object!")
    -- Проверка на тип rot
    assert_check(rot_val.type == "rot", "ERROR : You called RotateByVector, yet you call with NO rot")
    
    -- Вызов C++ функции RotateBy (lua_rotate_object_by_rotator)
    RotateBy(self.object, rot_val[1], rot_val[2], rot_val[3])
end

---Вращает объект по углам Эйлера (покомпонентно).
---@param x number Yaw (Y-axis)
---@param y number Pitch (X-axis) - обратите внимание на соответствие осям в C++ коде
---@param z number Roll (Z-axis)
function TransformComponent:RotateByCoords( x, y, z ) 
    RotateBy(self.object, x, y, z)
end

---Обновление трансформации (заглушка).
function TransformComponent:Update()
  --  print("transform update")
end

---Получает текущую позицию объекта из движка.
---@return table|nil map Таблица {x=..., y=..., z=...} или nil, если объект не привязан
function TransformComponent:GetPosition( )
    if self.object ~= nil then
        -- Вызов C++ функции: lua_get_object_pos -> std::map<string, float>
        return GetTransfromPosition(self.object)
    end
end

PhysicsComponent = {}
PhysicsComponent.__index = PhysicsComponent

function PhysicsComponent:Add( id )
    print("transform added")
	local instance = {}
    instance.id = id
    instance.object = nil
    instance.on = false
    setmetatable(instance, self)
    self.__index = self; 
    return instance
end

function PhysicsComponent:SetParent( parent )
    assert_check(parent ~= nil, "Attemp to call SetParent to on empty object, call SetParent to set object!")

    self.object = parent
     instance.on = true
end

print("core loaded")


--NodeTypes
NODE_TYPE_NODE3D = 0
NODE_TYPE_OBJECT3D = 1
NODE_TYPE_FIRST_PERSON_PLAYER = 2
NODE_TYPE_THIRD_PERSON_PLAYER = 3
NODE_TYPE_CAMERA = 4
NODE_TYPE_ENVIRONMENT = 5
NODE_TYPE_DIRECTIONAL_LIGHT = 6
NODE_TYPE_POINT_LIGHT = 7
NODE_TYPE_SPOT_LIGHT = 8
NODE_TYPE_PARTICLES = 9
NODE_TYPE_SKYBOX = 10
NODE_TYPE_AI = 11
NODE_TYPE_PHYSICAL_OBJECT3D = 12
NODE_TYPE_AUDIO_LISTENER = 13
NODE_TYPE_AUDIO_EMITTER = 14