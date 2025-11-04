Player = {}
Class(Player, GameObject)

function Player:Update()
	return GameObject.Update(self)
end

function Player:OnMouseClickInput( k, pressed )
	local s = GetObjectOnScene("player")
	print(k)
	print(pressed)
	print(s)

	return GameObject.OnMouseClickInput(self, k, pressed)
end

function Player:OnMouseMovementInputReceived( x, y )
	if self.transform then
		self.transform:MoveTo(x/100, 0, y/100)
	end
end


player = Player:new("player")
player:AddComponent(Transform)
--player.transform:MoveTo(100, 200, 300)
--print(player.transform)
player:Update()