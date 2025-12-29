CAR = {}
Class(CAR, GameObject)

function CAR:Update()
	if CAR.frameCounter < 700 and CAR.goingForward then
		CAR.frameCounter = CAR.frameCounter + 1

		if self.transform then
			self.transform:MoveByPos(0, 0, 0.01)
		end
	else
		
		CAR.goingForward = false

		if self.transform then
			self.transform:MoveByPos(0, 0, -0.01)
		end

		CAR.frameCounter = CAR.frameCounter - 1
		if CAR.frameCounter <= 0 then
			CAR.frameCounter = 0
			CAR.goingForward = true
		end
	end
	


	return GameObject.Update(self)
end

function CAR:Start()
	CAR.frameCounter = 0
	CAR.goingForward = true
	return GameObject.Start(self)
end