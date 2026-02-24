-- Minimal test ViewModel
Log.info("[ViewModel] Test script loading...")

local ViewModel = {}

ViewModel.Title = "Test"
ViewModel.Counter = 0

function ViewModel:IncrementCommand()
    Log.info("[ViewModel] Increment called")
    self.Counter = self.Counter + 1
    Log.infof("[ViewModel] Counter is now: %d", self.Counter)
    
    if type(Notify) == "function" then
        Log.info("[ViewModel] Calling Notify...")
        Notify("Counter")
        Log.info("[ViewModel] Notify done")
    else
        Log.error("[ViewModel] Notify not available!")
    end
end

_G.ViewModelInstance = ViewModel

Log.info("[ViewModel] Test script loaded")
