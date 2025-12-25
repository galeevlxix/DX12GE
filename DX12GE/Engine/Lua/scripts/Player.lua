PLAYER = {}
Class(PLAYER, GameObject)

function PLAYER:Update()
	return GameObject.Update(self)
end

function PLAYER:Start()
	return GameObject.Start(self)
end


function PLAYER:OnMouseClickInput( k, pressed )
	if self.transform ~= nil then
		local pos = self.transform:GetPosition()

		--[[local object = LoadObjectWithModel("ddd")
		if object ~= nil then
			if object.transform == nil then
				object:AddComponent(Transform)
			end
			object.transform:MoveTo(pos.x + 10, pos.y, pos.z + 10)
		end]]
	end
	

	return GameObject.OnMouseClickInput(self, k, pressed )
end

function PLAYER:OnMouseMovementInputReceived( x, y )
	if self.transform then
		self.transform:MoveTo(x/100, 10, y/100)
		--[[local object = GetClass("ddd")
		if object ~= nil then
			if object.transform == nil then
				object:AddComponent(Transform)		
			end

			object.transform:MoveTo(x/100, 10, y/100)
		end]]
	end
		
	return GameObject.OnMouseMovementInputReceived(self, x, y)
end
