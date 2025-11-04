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
    obj.id = id
    obj.components = {}
    Register(id)

    function GameObject:Start()
	     obj.object = GetObjectOnScene(obj.id)

         for _, component in pairs(self.components) do
            --component:SetParent(self.object)
         end
         print("start")
         self.transform:SetParent(self.object)    end

    function GameObject:Update()
        for _, component in ipairs(obj.components) do
            component:Update()
        end
     --   print("update")
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
     --   print("click")
    end

    function GameObject:AddComponent( component )
        local componentName = string.lower(component)

        if not self.components then self.components = {} end

	    if componentName == Transform then
            self.transform = TransformComponent:Add( obj.id )
            self.components[self.transform] = self.transform
        end

        if componentName == Physics then
            self.components[#self.components] = TransformComponent:Add( self.object )
            self.transform = self.components[#self.components - 1]
        end
    end

    setmetatable(obj, self)
    self.__index = self; return obj
end

TransformComponent = {}

function TransformComponent:Add( object )
	local obj = {}
    obj.id = object

    function TransformComponent:MoveTo( x, y, z )
        assert(obj.object ~= nil, "Attemp to call move to on empty object, call SetParent to set object!")

        TranslateTo(obj.object, x, y, z)
	end

    function TransformComponent:SetParent( parent )
	    obj.object = parent
    end

    function TransformComponent:Update()
        print("transform update")
	end


    setmetatable(obj, self)
    self.__index = self; return obj
end

PhysicsComponent = {}

