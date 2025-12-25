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

function PLAYER:OnKeyBoardInput( k, pressed )
	if k == KeyCode.w then
		if self.transform then
			self.transform:MoveByVector(Forward)
		end
	elseif k == KeyCode.s then
		if self.transform then
			self.transform:MoveByVector(Backward)
		end
	elseif k == KeyCode.a then
		if self.transform then
			self.transform:MoveByVector(Left)
		end
	elseif k == KeyCode.d then
		if self.transform then
			self.transform:MoveByVector(Right)
		end
	end

	return GameObject.OnKeyBoardInput(self, k, pressed)
end


function PLAYER:OnMouseMovementInputReceived( x, y )
	if self.transform then
		self.transform:RotateByCoords(x/100, 10, y/100)
	end
		
	return GameObject.OnMouseMovementInputReceived(self, x, y)
end
