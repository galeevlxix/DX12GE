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

player = Player:new("player")
player:Update()