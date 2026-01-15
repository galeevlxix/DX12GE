CAR = {}
Class(CAR, GameObject)

function CAR:Update()
	local rot = Vector3(0, 0.01, 0)
	self.transform:MoveByVector(rot)

	self.transform:RotateByVector(rot)

	return GameObject.Update(self)
end

function CAR:Start()
	self.frameCounter = 0
	self.goingForward = true

	self:AddComponet(Transform)

	return GameObject.Start(self)
end