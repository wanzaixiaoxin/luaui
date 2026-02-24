-- Test script to verify logging works
Log.info("===== TEST LOG START =====")
Log.info("This is a test message")
Log.infof("Test format: %s", "hello")
Log.info("===== TEST LOG END =====")

-- Test global
_G.TEST_VAR = "Hello from Lua"

return "Script executed successfully"
