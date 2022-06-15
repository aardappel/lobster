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
    im_config_is_srgb = 1 << 20
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

// Utility functions that are more convenient than calling start/end yourself.

def im_frame(body):
    im_frame_start()
    body()
    im_frame_end()

def im_window(title, flags, body):
    im_window_start(title, flags)
    body()
    im_window_end()

def im_treenode(label, body):
    let open = im_treenode_start(label)
    if open:
        body()
        im_treenode_end()

def im_group(label, body):
    im_group_start(label)
    body()
    im_group_end()

def im_width(width, body):
    im_width_start(width)
    body()
    im_width_end()

private let stack_traces = []
// Collect stack traces for the location this function is called from,
// as a rudimentary form of debugging.
// Pass positive n to collect the first n.
// Pass negative n to colledt the last n.
// Pass 0 to collect all (warning: may be slow if many!)
def im_stack_trace(n):
    if n > 0:
        if stack_traces.length >= n:
            return
    elif n < 0:
        if stack_traces.length >= -n: stack_traces.remove(0)
    let st = get_stack_trace()
    if st.length: stack_traces.push(st)

// Show the stack traces collected last frame, and clear them afterwards.
def im_show_stack_traces():
    im_treenode("Stack Traces"):
        for(stack_traces) st, i:
            im_treenode("{i}"):
                im_text(st)
    while stack_traces.length: stack_traces.pop()
