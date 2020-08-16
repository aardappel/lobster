// Constants for use with ImGui (https://github.com/ocornut/imgui) im_ functions.

// From: https://github.com/ocornut/imgui/blob/master/imgui.h

// To be passed to im_init()
enum_flags im_config:
    im_config_none = 0
    // Warning: turing this on stops the game from receiving keys.
    im_config_nav_enable_keyboard = 1
    im_config_nav_enable_gamepad
    im_config_nav_enable_set_mouse_pos
    im_config_nav_no_capture_keyboard
    im_config_no_mouse
    im_config_no_mouse_cursor_change
    im_config_is_srgb = 1048576
    im_config_is_touch_screen

// To be passed to im_window()
enum_flags im_window:
    im_window_none = 0
    im_window_no_titlebar = 1             // Disable title-bar
    im_window_no_resize                   // Disable user resizing with the lower-right grip
    im_window_no_move                     // Disable user moving the window
    im_window_no_scrollbar                // Disable scrollbars (window can still scroll with mouse or programatically)
    im_window_no_scroll_with_mouse        // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    im_window_no_collapse                 // Disable user collapsing window by double-clicking on it
    im_window_always_autoresize           // Resize every window to its content every frame
    im_window_no_background               // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    im_window_no_saved_settings           // Never load/save settings in .ini file
    im_window_no_mouse_inputs             // Disable catching mouse, hovering test with pass through.
    im_window_menubar                     // Has a menu-bar
    im_window_horizontal_scrollbar        // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    im_window_no_focus_on_appearing       // Disable taking focus when transitioning from hidden to visible state
    im_window_no_bringtofront_on_focus    // Disable bringing window to front when taking focus (e.g. clicking on it or programatically giving it focus)
    im_window_always_vertical_scrollbar   // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    im_window_always_horizontal_scrollbar // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    im_window_always_use_window_padding   // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
    im_window_UNUSED1
    im_window_no_nav_inputs               // No gamepad/keyboard navigation within the window
    im_window_no_nav_focus                // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    im_window_unsaved_document            // Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. When used in a tab/docking context, tab is selected on closure and closure is deferred by one frame to allow code to cancel the closure (with a confirmation popup, etc.) without flicker.

