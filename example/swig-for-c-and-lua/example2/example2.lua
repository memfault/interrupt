inspect = require "inspect"

assert(type(bindings) == 'table', "Binding module not loaded")

function processStruct(struct)
    -- Print info about structure
    print("[Lua] struct: " .. inspect(struct))
    print("[Lua] struct metatable: " .. inspect(getmetatable(struct)))

    -- Print our structure
    print("[Lua] struct.level = " .. tostring(struct.level))
    print("[Lua] struct.priority = " .. tostring(struct.priority))
    print("[Lua] struct.message = " .. struct.message)
    print("[Lua] struct.isReady = " .. tostring(struct.isReady))

    -- Now let's modify the structure
    struct.level = bindings.LEVEL_NONE
    struct.priority = 99
    struct.message = "Hey from Lua"
    struct.isReady = false
end
