-- Test ViewModel for integration tests
-- This simulates a typical user-written ViewModel

Log.info("[TestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Properties
ViewModel.Title = "Test Application"
ViewModel.Counter = 0
ViewModel.IsActive = false
ViewModel.Message = "Hello from Lua!"

-- Computed property
ViewModel:DefineComputed("StatusText", 
    {"Counter", "IsActive"},
    function(self)
        local status = self.IsActive and "Active" or "Inactive"
        return string.format("Counter: %d | Status: %s", self.Counter, status)
    end
)

-- Commands
function ViewModel:IncrementCommand()
    Log.info("[TestViewModel] IncrementCommand called")
    self.Counter = self.Counter + 1
end

function ViewModel:DecrementCommand()
    Log.info("[TestViewModel] DecrementCommand called")
    self.Counter = self.Counter - 1
end

function ViewModel:ResetCommand()
    Log.info("[TestViewModel] ResetCommand called")
    self.Counter = 0
    self.IsActive = false
end

function ViewModel:ToggleActiveCommand()
    Log.info("[TestViewModel] ToggleActiveCommand called")
    self.IsActive = not self.IsActive
end

-- Register globally
_G.ViewModelInstance = ViewModel

Log.info("[TestViewModel] Ready")
