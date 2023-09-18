// Constants for use with ImGui (https://github.com/ocornut/imgui) im functions.

// From: https://github.com/ocornut/imgui/blob/master/imgui.h

namespace im

// To be passed to im.init()
enum_flags config:
    config_none = 0
    // Warning: turing this on stops the game from receiving keys.
    config_nav_enable_keyboard = 1
    config_nav_enable_gamepad
    config_nav_enable_set_mouse_pos
    config_nav_no_capture_keyboard
    config_no_mouse
    config_no_mouse_cursor_change
    config_docking_enable
    config_viewport_enable = 1 << 10
    config_dpi_enable_scale_viewports = 1 << 14
    config_dpi_enable_scale_fonts
    config_is_srgb = 1 << 20
    config_is_touch_screen

// To be passed to im.window()
enum_flags window:
    window_none = 0
    window_no_titlebar = 1             // Disable title-bar
    window_no_resize                   // Disable user resizing with the lower-right grip
    window_no_move                     // Disable user moving the window
    window_no_scrollbar                // Disable scrollbars (window can still scroll with mouse or programatically)
    window_no_scroll_with_mouse        // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    window_no_collapse                 // Disable user collapsing window by double-clicking on it
    window_always_autoresize           // Resize every window to its content every frame
    window_no_background               // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    window_no_saved_settings           // Never load/save settings in .ini file
    window_no_mouse_inputs             // Disable catching mouse, hovering test with pass through.
    window_menubar                     // Has a menu-bar
    window_horizontal_scrollbar        // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    window_no_focus_on_appearing       // Disable taking focus when transitioning from hidden to visible state
    window_no_bringtofront_on_focus    // Disable bringing window to front when taking focus (e.g. clicking on it or programatically giving it focus)
    window_always_vertical_scrollbar   // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    window_always_horizontal_scrollbar // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    window_always_use_window_padding   // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
    window_UNUSED1
    window_no_nav_inputs               // No gamepad/keyboard navigation within the window
    window_no_nav_focus                // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    window_unsaved_document            // Append '*' to title without affecting the ID, as a convenience to avoid using the ### operator. When used in a tab/docking context, tab is selected on closure and closure is deferred by one frame to allow code to cancel the closure (with a confirmation popup, etc.) without flicker.
    window_no_docking                  // Disable docking of this window

enum_flags treenode:
    treenode_none = 0
    treenode_selected = 1         // Draw as selected
    treenode_framed               // Draw frame with background (e.g. for CollapsingHeader)
    treenode_allow_item_overlap   // Hit testing to allow subsequent widgets to overlap this one
    treenode_no_tree_push_on_open // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    treenode_no_auto_open_on_log  // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    treenode_default_open         // Default node to be open
    treenode_open_on_double_click // Need double-click to open node
    treenode_open_on_arrow        // Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
    treenode_leaf                 // No collapsing, no arrow (use as a convenience for leaf nodes).
    treenode_bullet               // Display a bullet instead of arrow
    treenode_frame_padding        // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
    treenode_span_avail_width     // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
    treenode_span_full_width      // Extend hit box to the left-most and right-most edges (bypass the indented area).

enum_flags tabitem:
    tabitem_none = 0
    tabitem_unsaved_document = 1               // Display a dot next to the title + tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    tabitem_set_selected                       // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    tabitem_no_close_with_middle_mouse_button  // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    tabitem_no_push_id                         // Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()
    tabitem_no_tooltip                         // Disable tooltip for the given tab
    tabitem_no_reorder                         // Disable reordering this tab or having another tab cross over this tab
    tabitem_leading                            // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    tabitem_trailing                           // Enforce the tab position to the right of the tab bar (before the scrolling buttons)

enum_flags drag_drop:
    drag_drop_none = 0,
    drag_drop_source_no_preview_tooltip = 1   // Disable preview tooltip. By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
    drag_drop_source_no_disable_hover         // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
    drag_drop_source_no_hold_to_open_others   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    drag_drop_source_allow_null_id            // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.
    drag_drop_source_extern                   // External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    drag_drop_source_auto_expire_payload      // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
    drag_drop_accept_before_delivery          // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    drag_drop_accept_no_draw_default_rect     // Do not draw the default highlight rectangle when hovering over target.
    drag_drop_accept_no_preview_tooltip       // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.

enum color_idx:
    col_text
    col_text_disabled
    col_window_bg              // Background of normal windows
    col_child_bg               // Background of child windows
    col_popup_bg               // Background of popups, menus, tooltips windows
    col_border
    col_border_shadow
    col_frame_bg               // Background of checkbox, radio button, plot, slider, text input
    col_frame_bg_hovered
    col_frame_bg_active
    col_title_bg
    col_title_bg_active
    col_title_bg_collapsed
    col_menubar_bg
    col_scrollbar_bg
    col_scrollbar_grab
    col_scrollbar_grab_hovered
    col_scrollbar_grab_active
    col_check_mark
    col_slider_grab
    col_slider_grab_active
    col_button
    col_button_hovered
    col_button_active
    col_header                 // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    col_header_hovered
    col_header_active
    col_separator
    col_separator_hovered
    col_separator_active
    col_resize_grip            // Resize grip in lower-right and lower-left corners of windows.
    col_resize_grip_hovered
    col_resize_grip_active
    col_tab                    // TabItem in a TabBar
    col_tab_hovered
    col_tab_active
    col_tab_unfocused
    col_tab_unfocused_active
    col_docking_preview        // Preview overlay color when about to docking something
    col_docking_empty_bg       // Background color for empty node (e.g. CentralNode with no window docked into it)
    col_plot_lines
    col_plot_lines_hovered
    col_plot_histogram
    col_plot_histogram_hovered
    col_table_header_bg        // Table header background
    col_table_border_strong    // Table outer and header borders (prefer using Alpha=1.0 here)
    col_table_border_light     // Table inner borders (prefer using Alpha=1.0 here)
    col_table_row_bg           // Table row background (even rows)
    col_table_row_bg_alt       // Table row background (odd rows)
    col_text_selected_bg
    col_drag_drop_target       // Rectangle highlighting a drop target
    col_nav_highlight          // Gamepad/keyboard: current highlighted item
    col_nav_windowing_highlight// Highlight window when using CTRL+TAB
    col_nav_windowing_dim_bg   // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    col_modal_window_dim_bg    // Darken/colorize entire screen behind a modal window, when one is active
    col_COUNT


// Utility functions that are more convenient than calling start/end yourself.

def frame(body):
    frame_start()
    body()
    frame_end()

def window(title, flags, body):
    window_start(title, flags, false)
    body()
    window_end()

def window_dock(title, flags, body):
    window_start(title, flags, true)
    body()
    window_end()

def treenode(label, body):
    let open = treenode_start(label, 0)
    if open:
        body()
        treenode_end()
    return open

def treenode_open(label, body):
    let open = treenode_start(label, treenode_default_open)
    if open:
        body()
        treenode_end()
    return open

def treenode_open_if(label, cond, body):
    let open = treenode_start(label, if cond: treenode_default_open else: 0)
    if open:
        body()
        treenode_end()
    return open

def tooltip_multi(body):
    let open = tooltip_multi_start()
    if open:
        body()
        tooltip_multi_end()
    return open

def tab_bar(label, body):
    let open = tab_bar_start(label)
    if open:
        body()
        tab_bar_end()
    return open

def tab(label, body):
    let open = tab_start(label, tabitem_none)
    if open:
        body()
        tab_end()
    return open

def tab(label, flags, body):
    let open = tab_start(label, flags)
    if open:
        body()
        tab_end()
    return open

def menu_bar(main, body):
    let open = menu_bar_start(main)
    if open:
        body()
        menu_bar_end(main)
    return open

def menu(label, disabled, body):
    let open = menu_start(label, disabled)
    if open:
        body()
        menu_end()
    return open

def menu(label, body):
    let open = menu_start(label, false)
    if open:
        body()
        menu_end()
    return open

def id(label, body):
    id_start(label)
    body()
    id_end()

// Call im.popup_open (from e.g. im.button) with same label as this, followed by im.popup() with body.
def popup(label, body):
    let open = popup_start(label, 0, false)
    if open:
        body()
        popup_end()
    return open

// Call this right after any item for which you want RMB to generate a popup.
def popup_rmb_prev_item(label, body):
    let open = popup_start(label, 0, true)
    if open:
        body()
        popup_end()
    return open

def disabled(disabled, body):
    disabled_start(disabled)
    body()
    disabled_end()

def button_repeat(repeat, body):
    button_repeat_start(repeat)
    body()
    button_repeat_end()

def drag_drop_source(flags, body):
    let open = drag_drop_source_start(flags)
    if open:
        body()
        drag_drop_source_end()
    return open

def drag_drop_target(body):
    let open = drag_drop_target_start()
    if open:
        body()
        drag_drop_target_end()
    return open

def width(width, body):
    width_start(width)
    body()
    width_end()

def child(title, size, body):
    child_start(title, size, 0)
    body()
    child_end()

def child(title, size, flags, body):
    child_start(title, size, flags)
    body()
    child_end()

def group(body):
    group_start()
    body()
    group_end()

private let stack_traces = []
// Collect stack traces for the location this function is called from,
// as a rudimentary form of debugging.
// Pass positive n to collect the first n.
// Pass negative n to colledt the last n.
// Pass 0 to collect all (warning: may be slow if many!)
def stack_trace(n):
    if n > 0:
        if stack_traces.length >= n:
            return
    elif n < 0:
        if stack_traces.length >= -n: stack_traces.remove(0)
    let st = get_stack_trace()
    if st.length: stack_traces.push(st)

// Show the stack traces collected last frame, and clear them afterwards.
def show_stack_traces():
    treenode("Stack Traces"):
        for(stack_traces) st, i:
            treenode("{i}"):
                text(st)
    while stack_traces.length: stack_traces.pop()
