local mouseSensetivity = 0.0075
local prevXMouse = 0
local prevYMouse = 0
local mDx = 0
local mDy = 0
local mAngleH = 0
local mAngleV = 0
local player = nil
local camera = nil
local xp = 0
local yp = 0

function DoThing( a, b, d )
	print("[LUA] DoThing("..a..", "..b..", "..d..") called \n")

	c = Foo(a + 10, b)
	--player = get_object_on_scene("player")
	--move_object_to_position(d, 10, 0 , 25)
	return c
end

function OnMouseMovementInputReceived( x, y )
	--print(x, " ", y)

	mDx = x - prevXMouse
	mDy = y - prevYMouse

	if camera == nil then
	--	camera = get_camera()
		--print("came inited")
	end

	mAngleH = mAngleH - mDx * mouseSensetivity
	if mAngleV + mDy * mouseSensetivity > math.pi / 2.0 and mAngleV + mDy * mouseSensetivity < math.pi / 2.0 then
		mAngleV = mAngleV + mDy * mouseSensetivity
	end
	--player:rotate_object_by_rotator(x, y, 0)

	prevXMouse = x
	prevYMouse = y
end

function Update()
	if player == nil then
		player = get_object_on_scene("player")
		print("player inited")
	end
	if camera == nil then
		camera = get_camera()
		print("came inited")
	end
	local xTar = math.cos(mAngleH) * math.sin(math.pi / 2.0 - mAngleV)
	local yTar = math.cos(math.pi/ 2.0 - mAngleV)
	local zTar = math.sin(mAngleH) * math.sin(math.pi / 2.0 - mAngleV)

	xp = xp + 0.01
	yp = yp + 0.01
	rotate_object_by_rotator(player, xp, yp, 0)
	move_object_to_position(player, xp * 10, yp * 10 , 0)
	--set_camera_target(player, xTar, yTar, zTar, 15.0)
	--print("Update")
end


