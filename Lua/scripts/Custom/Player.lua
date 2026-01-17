PLAYER = {}
Class(PLAYER, GameObject)

function PLAYER:Update()
	if PLAYER.moveVector ~= nil then
		self.transform:MoveByPos(
		PLAYER.moveVector.x, 
		PLAYER.moveVector.y, 
		PLAYER.moveVector.z)
	end

	return GameObject.Update(PLAYER)
end

function PLAYER:Start()
	PLAYER.moveVector = {}
	PLAYER.moveVector.x = 0
	PLAYER.moveVector.y = 0
	PLAYER.moveVector.z = 0
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

	if pressed == false then
		PLAYER.moveVector = {}
	elseif k == KeyCode.w then
		if self.transform then
			PLAYER.moveVector.x = 0.01
			PLAYER.moveVector.y = 0
			PLAYER.moveVector.z = 0
		end
	elseif k == KeyCode.s then
		if self.transform then
			PLAYER.moveVector.x = -0.01
			PLAYER.moveVector.y = 0
			PLAYER.moveVector.z = 0
		end
	elseif k == KeyCode.a then
		if self.transform then
			PLAYER.moveVector.x = 0
			PLAYER.moveVector.y = 0
			PLAYER.moveVector.z = 0.01
		end
	elseif k == KeyCode.d then
		if self.transform then
			PLAYER.moveVector.x = 0
			PLAYER.moveVector.y = 0
			PLAYER.moveVector.z = -0.01
		end
	elseif k == KeyCode.q then
		if self.transform then
			PLAYER.moveVector.x = 0
			PLAYER.moveVector.y = 0.01
			PLAYER.moveVector.z = 0
		end
	elseif k == KeyCode.e then
		if self.transform then
			PLAYER.moveVector.x = 0
			PLAYER.moveVector.y = -0.01
			PLAYER.moveVector.z = 0
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
