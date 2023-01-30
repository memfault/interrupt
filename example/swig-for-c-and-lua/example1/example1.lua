assert(type(bindings) == 'table', "Binding module not loaded")
print("[Lua] Result of multiply -2 * 5 = " .. bindings.multiply(-2, 5))
