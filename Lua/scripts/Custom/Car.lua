CAR = {}
Class(CAR, GameObject)

function CAR:Update()


	return GameObject.Update(self)
end

function CAR:Start()

	return GameObject.Start(self)
end