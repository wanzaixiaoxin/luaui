-- ScrollViewer Test ViewModel
-- Demonstrates ScrollViewer scrolling

Log.info("[ScrollViewerTestViewModel] Initializing...")

local ViewModel = AutoViewModel.new()
ViewModel = ViewModel:EnableAutoNotify()

-- Test metadata
ViewModel.TestTitle = "ScrollViewer Layout Test"
ViewModel.TestDescription = "Demonstrates scrolling for content larger than the viewport"

-- Register globally
_G.ScrollViewerTestViewModel = ViewModel

Log.info("[ScrollViewerTestViewModel] Ready")
