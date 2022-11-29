inspect = require "inspect"

assert(type(bindings) == 'table', "Binding module not loaded")

function processStruct(struct)
    -- Print info about structure
    print("[Lua] struct.priorities: " .. inspect(struct.priorities))
    print("[Lua] struct.priorities metatable: " .. inspect(getmetatable(struct.priorities)))
    -- This modification will not take effect
    struct.priorities[1] = 0
end
