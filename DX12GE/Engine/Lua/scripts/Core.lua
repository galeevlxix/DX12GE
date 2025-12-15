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

Transform   = "transfrom"
Physics     = "physics"


function Class(child, parent)
    setmetatable(child, {__index = parent})
end

GameObject = {}

function GameObject:new( id )
    local obj = {}
    obj.entity = nil
    obj.id = id
    obj.components = {}
    obj.object = nil
    obj.transform = nil

    function GameObject:SetEntityName( entity )
        obj.entity = entity
    end


    function GameObject:Start()
        print("Start for obj.id ", obj.id)
	    obj.object = GetObjectOnScene(obj.entity)
        if  obj.transform ~= nil and obj.object ~= nil then
            obj.transform:SetParent(obj.object )
        end
    end

    function GameObject:Update()
     --   for _, component in ipairs(obj.components) do
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
	 --   print("keyboard")
    end

    function GameObject:OnMouseClickInput( k, pressed )
    --    print("click")
    end

    function GameObject:AddComponent( component )
      local componentName = string.lower(component)
        
        if not obj.components then obj.components = {} end

	    if componentName == Transform then
            print("component added for id ", obj.id)
             obj.transform = TransformComponent:Add( obj.id )
            obj.components[obj.transform] = obj.transform
        end

        if componentName == Physics then
            obj.components[#obj.components] = TransformComponent:Add( obj.object )
            obj.transform = obj.components[#obj.components - 1]
        end
    end

    setmetatable(obj, self)
    self.__index = self; return obj
end

TransformComponent = {}

function TransformComponent:Add( id )
    print("transform added")
	local obj = {}
    obj.id = id
    obj.object = nil
    function TransformComponent:MoveTo( x, y, z )
        assert(obj.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")

        TranslateTo(obj.object, x, y, z)
	end

    function TransformComponent:MoveBy( x, y, z )
        assert(obj.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")
        TranslateBy(obj.object, x, y, z)
	end

    function TransformComponent:SetParent( parent )
        assert(parent ~= nil, "Attemp to call SetParent to on empty object, call SetParent to set object!")
        print("set parent for transform")
	    obj.object = parent
    end

    function TransformComponent:Update()
        print("transform update")
	end

    function TransformComponent:GetPosition( )
        if obj.object ~= nil then
            return GetTransfromPosition(obj.object)
        end
    end

    setmetatable(obj, self)
    self.__index = self; return obj
end

PhysicsComponent = {}


