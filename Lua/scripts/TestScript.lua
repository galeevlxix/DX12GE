local t = 10
local v = 15
result = {}
result.int = v + t * 10

function DoThing( a, b, d )
	print("[LUA] DoThing("..a..", "..b..", "..d..") called \n")

	c = HostFunction(a + 10, b)

	return c
end

