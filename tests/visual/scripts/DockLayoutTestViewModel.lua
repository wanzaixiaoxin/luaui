-- Dock Layout Test ViewModel
-- Demonstrates VSCode-like docking with nested splits and tab groups

Log.info("[DockLayoutTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "Dock Layout Test"
ViewModel.TestDescription = "VSCode-like docking with nested splits, tab groups, and proportional sizing"

-- Register globally
_G.DockLayoutTestViewModel = ViewModel

Log.info("[DockLayoutTestViewModel] Ready")
