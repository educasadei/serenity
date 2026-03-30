/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "OutOfProcessWebView.h"
#include "WebContentClient.h"
#include <AK/ByteString.h>
#include <LibFileSystemAccessClient/Client.h>
#include <LibGUI/Application.h>
#include <LibGUI/Desktop.h>
#include <LibGUI/Painter.h>
#include <LibGUI/Scrollbar.h>
#include <LibGUI/Window.h>
#include <LibGfx/Font/FontDatabase.h>
#include <LibGfx/Palette.h>
#include <LibGfx/SystemTheme.h>
#include <LibWeb/Crypto/Crypto.h>
#include <LibWeb/Worker/WebWorkerClient.h>

REGISTER_WIDGET(WebView, OutOfProcessWebView)

namespace WebView {

OutOfProcessWebView::OutOfProcessWebView()
{
    set_focus_policy(GUI::FocusPolicy::StrongFocus);

    initialize_client(CreateNewClient::Yes);

    on_ready_to_paint = [this]() {
        update();
    };

    on_request_file = [this](auto const& path, auto request_id) {
        auto file = FileSystemAccessClient::Client::the().request_file_read_only_approved(window(), path);

        if (file.is_error())
            client().async_handle_file_return(m_client_state.page_index, file.error().code(), {}, request_id);
        else
            client().async_handle_file_return(m_client_state.page_index, 0, IPC::File::adopt_file(file.release_value().release_stream()), request_id);
    };

    on_cursor_change = [this](auto cursor) {
        set_override_cursor(cursor);
    };

    // FIXME: Set on_request_tooltip_override, on_stop_tooltip_override.

    on_enter_tooltip_area = [](auto tooltip) {
        GUI::Application::the()->show_tooltip(MUST(String::from_byte_string(tooltip)), nullptr);
    };

    on_leave_tooltip_area = []() {
        GUI::Application::the()->hide_tooltip();
    };

    on_finish_handling_key_event = [this](auto const& event) {
        finish_handling_key_event(event);
    };

    on_finish_handling_drag_event = [this](auto const& event) {
        finish_handling_drag_event(event);
    };

    on_request_worker_agent = []() {
        auto worker_client = MUST(Web::HTML::WebWorkerClient::try_create());
        return worker_client->dup_socket();
    };
}

OutOfProcessWebView::~OutOfProcessWebView() = default;

void OutOfProcessWebView::initialize_client(WebView::ViewImplementation::CreateNewClient)
{
    // FIXME: Don't create a new process when CreateNewClient is false
    //        We should create a new tab/window in the UI instead, and re-use the existing WebContentClient object.
    m_client_state = {};

    m_client_state.client = WebContentClient::try_create(*this).release_value_but_fixme_should_propagate_errors();
    m_client_state.client->on_web_content_process_crash = [this] {
        deferred_invoke([this] {
            handle_web_content_process_crash();
        });
    };

    m_client_state.client_handle = Web::Crypto::generate_random_uuid().release_value_but_fixme_should_propagate_errors();
    client().async_set_window_handle(m_client_state.page_index, m_client_state.client_handle);

    client().async_update_system_theme(m_client_state.page_index, Gfx::current_system_theme_buffer());
    client().async_update_system_fonts(m_client_state.page_index, Gfx::FontDatabase::default_font_query(), Gfx::FontDatabase::fixed_width_font_query(), Gfx::FontDatabase::window_title_font_query());

    Vector<Web::DevicePixelRect> screen_rects;
    for (auto const& screen_rect : GUI::Desktop::the().rects()) {
        screen_rects.append(screen_rect.to_type<Web::DevicePixels>());
    }
    client().async_update_screen_rects(m_client_state.page_index, screen_rects, GUI::Desktop::the().main_screen_index());
}

void OutOfProcessWebView::paint_event(GUI::PaintEvent& event)
{
    Super::paint_event(event);

    // If the content size is empty, we don't have a front or back bitmap to draw.
    if (content_size().is_empty())
        return;

    GUI::Painter painter(*this);
    painter.add_clip_rect(event.rect());

    if (auto* bitmap = m_client_state.has_usable_bitmap ? m_client_state.front_bitmap.bitmap.ptr() : m_backup_bitmap.ptr()) {
        painter.add_clip_rect(frame_inner_rect());
        painter.translate(frame_thickness(), frame_thickness());
        if (m_content_scales_to_viewport) {
            auto bitmap_rect = Gfx::IntRect {
                {},
                m_client_state.has_usable_bitmap
                    ? m_client_state.front_bitmap.last_painted_size
                    : m_backup_bitmap_size
            };
            painter.draw_scaled_bitmap(rect(), *bitmap, bitmap_rect);
        } else {
            painter.blit({ 0, 0 }, *bitmap, bitmap->rect());
        }
        return;
    }

    painter.fill_rect(frame_inner_rect(), palette().base());
}

void OutOfProcessWebView::resize_event(GUI::ResizeEvent& event)
{
    Super::resize_event(event);
    client().async_set_viewport_size(m_client_state.page_index, content_size().to_type<Web::DevicePixels>());
    handle_resize();
}

Web::DevicePixelSize OutOfProcessWebView::viewport_size() const
{
    return content_size().to_type<Web::DevicePixels>();
}

Gfx::IntPoint OutOfProcessWebView::to_content_position(Gfx::IntPoint widget_position) const
{
    return widget_position;
}

Gfx::IntPoint OutOfProcessWebView::to_widget_position(Gfx::IntPoint content_position) const
{
    return content_position;
}

void OutOfProcessWebView::update_zoom()
{
    client().async_set_device_pixels_per_css_pixel(m_client_state.page_index, m_device_pixel_ratio * m_zoom_level);
    // FIXME: Refactor this into separate update_viewport_rect() + request_repaint() like in Ladybird
    handle_resize();
}

void OutOfProcessWebView::keydown_event(GUI::KeyEvent& event)
{
    enqueue_native_event(Web::KeyEvent::Type::KeyDown, event);
}

void OutOfProcessWebView::keyup_event(GUI::KeyEvent& event)
{
    enqueue_native_event(Web::KeyEvent::Type::KeyUp, event);
}

void OutOfProcessWebView::mousedown_event(GUI::MouseEvent& event)
{
    enqueue_native_event(Web::MouseEvent::Type::MouseDown, event);
}

void OutOfProcessWebView::mouseup_event(GUI::MouseEvent& event)
{
    enqueue_native_event(Web::MouseEvent::Type::MouseUp, event);

    if (event.button() == GUI::MouseButton::Backward) {
        if (on_navigate_back)
            on_navigate_back();
    } else if (event.button() == GUI::MouseButton::Forward) {
        if (on_navigate_forward)
            on_navigate_forward();
    }
}

void OutOfProcessWebView::mousemove_event(GUI::MouseEvent& event)
{
    enqueue_native_event(Web::MouseEvent::Type::MouseMove, event);
}

void OutOfProcessWebView::mousewheel_event(GUI::MouseEvent& event)
{
    enqueue_native_event(Web::MouseEvent::Type::MouseWheel, event);
}

void OutOfProcessWebView::doubleclick_event(GUI::MouseEvent& event)
{
    enqueue_native_event(Web::MouseEvent::Type::DoubleClick, event);
}

void OutOfProcessWebView::theme_change_event(GUI::ThemeChangeEvent& event)
{
    Super::theme_change_event(event);
    client().async_update_system_theme(m_client_state.page_index, Gfx::current_system_theme_buffer());
}

void OutOfProcessWebView::screen_rects_change_event(GUI::ScreenRectsChangeEvent& event)
{
    Vector<Web::DevicePixelRect> screen_rects;
    for (auto const& screen_rect : event.rects()) {
        screen_rects.append(screen_rect.to_type<Web::DevicePixels>());
    }
    client().async_update_screen_rects(m_client_state.page_index, screen_rects, event.main_screen_index());
}

OrderedHashMap<String, String> OutOfProcessWebView::get_local_storage_entries()
{
    return client().get_local_storage_entries(m_client_state.page_index);
}

OrderedHashMap<String, String> OutOfProcessWebView::get_session_storage_entries()
{
    return client().get_session_storage_entries(m_client_state.page_index);
}

void OutOfProcessWebView::set_content_filters(Vector<String> filters)
{
    client().async_set_content_filters(m_client_state.page_index, move(filters));
}

void OutOfProcessWebView::set_autoplay_allowed_on_all_websites()
{
    client().async_set_autoplay_allowed_on_all_websites(m_client_state.page_index);
}

void OutOfProcessWebView::set_autoplay_allowlist(Vector<String> allowlist)
{
    client().async_set_autoplay_allowlist(m_client_state.page_index, move(allowlist));
}

void OutOfProcessWebView::set_proxy_mappings(Vector<ByteString> proxies, HashMap<ByteString, size_t> mappings)
{
    client().async_set_proxy_mappings(m_client_state.page_index, move(proxies), move(mappings));
}

void OutOfProcessWebView::connect_to_webdriver(ByteString const& webdriver_ipc_path)
{
    client().async_connect_to_webdriver(m_client_state.page_index, webdriver_ipc_path);
}

void OutOfProcessWebView::set_window_position(Gfx::IntPoint position)
{
    client().async_set_window_position(m_client_state.page_index, position.to_type<Web::DevicePixels>());
}

void OutOfProcessWebView::set_window_size(Gfx::IntSize size)
{
    client().async_set_window_size(m_client_state.page_index, size.to_type<Web::DevicePixels>());
}

void OutOfProcessWebView::focusin_event(GUI::FocusEvent&)
{
    client().async_set_has_focus(m_client_state.page_index, true);
}

void OutOfProcessWebView::focusout_event(GUI::FocusEvent&)
{
    client().async_set_has_focus(m_client_state.page_index, false);
}

void OutOfProcessWebView::set_system_visibility_state(bool visible)
{
    client().async_set_system_visibility_state(m_client_state.page_index, visible);
}

void OutOfProcessWebView::show_event(GUI::ShowEvent&)
{
    set_system_visibility_state(true);
}

void OutOfProcessWebView::hide_event(GUI::HideEvent&)
{
    set_system_visibility_state(false);
}

void OutOfProcessWebView::drag_enter_event(GUI::DragEvent& event)
{
    if (!event.mime_data().has_urls())
        return;

    enqueue_native_event(Web::DragEvent::Type::DragStart, event);
    event.accept();
}

void OutOfProcessWebView::drag_move_event(GUI::DragEvent& event)
{
    enqueue_native_event(Web::DragEvent::Type::DragMove, event);
    event.accept();
}

void OutOfProcessWebView::drag_leave_event(GUI::Event&)
{
    Web::DragEvent event {};
    event.type = Web::DragEvent::Type::DragEnd;

    enqueue_input_event(move(event));
}

void OutOfProcessWebView::drop_event(GUI::DropEvent& event)
{
    enqueue_native_event(Web::DragEvent::Type::Drop, event);
    event.accept();
}

static constexpr Web::UIEvents::MouseButton web_button_from_gui_button(GUI::MouseButton button)
{
    switch (button) {
    case GUI::MouseButton::None:
        return Web::UIEvents::MouseButton::None;
    case GUI::MouseButton::Primary:
        return Web::UIEvents::MouseButton::Primary;
    case GUI::MouseButton::Secondary:
        return Web::UIEvents::MouseButton::Secondary;
    case GUI::MouseButton::Middle:
        return Web::UIEvents::MouseButton::Middle;
    case GUI::MouseButton::Backward:
        return Web::UIEvents::MouseButton::Backward;
    case GUI::MouseButton::Forward:
        return Web::UIEvents::MouseButton::Forward;
    }
    VERIFY_NOT_REACHED();
}

static constexpr Web::UIEvents::MouseButton web_buttons_from_gui_buttons(unsigned buttons)
{
    auto result = Web::UIEvents::MouseButton::None;

    if ((buttons & GUI::MouseButton::Primary) != 0)
        result |= Web::UIEvents::MouseButton::Primary;
    if ((buttons & GUI::MouseButton::Secondary) != 0)
        result |= Web::UIEvents::MouseButton::Secondary;
    if ((buttons & GUI::MouseButton::Middle) != 0)
        result |= Web::UIEvents::MouseButton::Middle;
    if ((buttons & GUI::MouseButton::Backward) != 0)
        result |= Web::UIEvents::MouseButton::Backward;
    if ((buttons & GUI::MouseButton::Forward) != 0)
        result |= Web::UIEvents::MouseButton::Forward;

    return result;
}

static constexpr Web::UIEvents::KeyModifier web_modifiers_from_gui_modifiers(unsigned modifiers)
{
    auto result = Web::UIEvents::KeyModifier::Mod_None;

    if (modifiers & KeyModifier::Mod_Alt)
        result |= Web::UIEvents::KeyModifier::Mod_Alt;
    if (modifiers & KeyModifier::Mod_Ctrl)
        result |= Web::UIEvents::KeyModifier::Mod_Ctrl;
    if (modifiers & KeyModifier::Mod_Super)
        result |= Web::UIEvents::KeyModifier::Mod_Shift;
    if (modifiers & KeyModifier::Mod_Shift)
        result |= Web::UIEvents::KeyModifier::Mod_Super;
    if (modifiers & KeyModifier::Mod_AltGr)
        result |= Web::UIEvents::KeyModifier::Mod_AltGr;
    if (modifiers & KeyModifier::Mod_Keypad)
        result |= Web::UIEvents::KeyModifier::Mod_Keypad;

    return result;
}

void OutOfProcessWebView::enqueue_native_event(Web::MouseEvent::Type type, GUI::MouseEvent const& event)
{
    auto position = to_content_position(event.position()).to_type<Web::DevicePixels>();
    auto screen_position = (event.position() + (window()->position() + relative_position())).to_type<Web::DevicePixels>();

    auto button = web_button_from_gui_button(event.button());
    auto buttons = web_buttons_from_gui_buttons(event.buttons());
    auto modifiers = web_modifiers_from_gui_modifiers(event.modifiers());

    // FIXME: This wheel delta step size multiplier is used to remain the old scroll behaviour, in future use system step size.
    static constexpr int SCROLL_STEP_SIZE = 24;
    auto wheel_delta_x = event.wheel_delta_x() * SCROLL_STEP_SIZE;
    auto wheel_delta_y = event.wheel_delta_y() * SCROLL_STEP_SIZE;

    enqueue_input_event(Web::MouseEvent { type, position, screen_position, button, buttons, modifiers, wheel_delta_x, wheel_delta_y, nullptr });
}

struct DragData : Web::ChromeInputData {
    explicit DragData(GUI::DropEvent const& event)
        : event(make<GUI::DropEvent>(event))
    {
    }

    NonnullOwnPtr<GUI::DropEvent> event;
};

void OutOfProcessWebView::enqueue_native_event(Web::DragEvent::Type type, GUI::DropEvent const& event)
{
    auto position = to_content_position(event.position()).to_type<Web::DevicePixels>();
    auto screen_position = (event.position() + (window()->position() + relative_position())).to_type<Web::DevicePixels>();

    auto button = web_button_from_gui_button(event.button());
    auto buttons = web_buttons_from_gui_buttons(event.buttons());
    auto modifiers = web_modifiers_from_gui_modifiers(event.modifiers());

    Vector<Web::HTML::SelectedFile> files;
    OwnPtr<DragData> chrome_data;

    if (type == Web::DragEvent::Type::DragStart) {
        VERIFY(event.mime_data().has_urls());

        for (auto const& url : event.mime_data().urls()) {
            auto file_path = URL::percent_decode(url.serialize_path());

            if (auto file = Web::HTML::SelectedFile::from_file_path(file_path); file.is_error())
                warnln("Unable to open file {} for drag-and-drop: {}", file_path, file.error());
            else
                files.append(file.release_value());
        }
    } else if (type == Web::DragEvent::Type::Drop) {
        chrome_data = make<DragData>(event);
    } else {
        VERIFY(type == Web::DragEvent::Type::DragMove);
    }

    enqueue_input_event(Web::DragEvent { type, position, screen_position, button, buttons, modifiers, move(files), move(chrome_data) });
}

void OutOfProcessWebView::finish_handling_drag_event(Web::DragEvent const& event)
{
    if (event.type != Web::DragEvent::Type::Drop)
        return;

    // FIXME: Implement opening the event URLs in the Browser.
    [[maybe_unused]] auto const& chrome_data = verify_cast<DragData>(*event.chrome_data);
}

struct KeyData : Web::ChromeInputData {
    explicit KeyData(GUI::KeyEvent const& event)
        : event(make<GUI::KeyEvent>(event))
    {
    }

    NonnullOwnPtr<GUI::KeyEvent> event;
};

static Web::UIEvents::KeyCode web_key_code_from_gui_key_code(KeyCode key_code)
{
#define MAP_KEY(key, web_key) \
    case KeyCode::key:        \
        return Web::UIEvents::KeyCode::web_key

    switch (key_code) {
    default:
        return Web::UIEvents::KeyCode::Key_Invalid;

        MAP_KEY(Key_Escape, Key_Escape);
        MAP_KEY(Key_Tab, Key_Tab);
        MAP_KEY(Key_Backspace, Key_Backspace);
        MAP_KEY(Key_Return, Key_Return);
        MAP_KEY(Key_Insert, Key_Insert);
        MAP_KEY(Key_Delete, Key_Delete);
        MAP_KEY(Key_PrintScreen, Key_PrintScreen);
        MAP_KEY(Key_PauseBreak, Key_PauseBreak);
        MAP_KEY(Key_SysRq, Key_SysRq);
        MAP_KEY(Key_Home, Key_Home);
        MAP_KEY(Key_End, Key_End);
        MAP_KEY(Key_Left, Key_Left);
        MAP_KEY(Key_Up, Key_Up);
        MAP_KEY(Key_Right, Key_Right);
        MAP_KEY(Key_Down, Key_Down);
        MAP_KEY(Key_PageUp, Key_PageUp);
        MAP_KEY(Key_PageDown, Key_PageDown);
        MAP_KEY(Key_LeftShift, Key_LeftShift);
        MAP_KEY(Key_RightShift, Key_RightShift);
        MAP_KEY(Key_LeftControl, Key_LeftControl);
        MAP_KEY(Key_RightControl, Key_RightControl);
        MAP_KEY(Key_LeftAlt, Key_LeftAlt);
        MAP_KEY(Key_RightAlt, Key_RightAlt);
        MAP_KEY(Key_AltGr, Key_AltGr);
        MAP_KEY(Key_CapsLock, Key_CapsLock);
        MAP_KEY(Key_NumLock, Key_NumLock);
        MAP_KEY(Key_ScrollLock, Key_ScrollLock);
        MAP_KEY(Key_F1, Key_F1);
        MAP_KEY(Key_F2, Key_F2);
        MAP_KEY(Key_F3, Key_F3);
        MAP_KEY(Key_F4, Key_F4);
        MAP_KEY(Key_F5, Key_F5);
        MAP_KEY(Key_F6, Key_F6);
        MAP_KEY(Key_F7, Key_F7);
        MAP_KEY(Key_F8, Key_F8);
        MAP_KEY(Key_F9, Key_F9);
        MAP_KEY(Key_F10, Key_F10);
        MAP_KEY(Key_F11, Key_F11);
        MAP_KEY(Key_F12, Key_F12);
        MAP_KEY(Key_Space, Key_Space);
        MAP_KEY(Key_ExclamationPoint, Key_ExclamationPoint);
        MAP_KEY(Key_DoubleQuote, Key_DoubleQuote);
        MAP_KEY(Key_Hashtag, Key_Hashtag);
        MAP_KEY(Key_Dollar, Key_Dollar);
        MAP_KEY(Key_Percent, Key_Percent);
        MAP_KEY(Key_Ampersand, Key_Ampersand);
        MAP_KEY(Key_Apostrophe, Key_Apostrophe);
        MAP_KEY(Key_LeftParen, Key_LeftParen);
        MAP_KEY(Key_RightParen, Key_RightParen);
        MAP_KEY(Key_Asterisk, Key_Asterisk);
        MAP_KEY(Key_Plus, Key_Plus);
        MAP_KEY(Key_Comma, Key_Comma);
        MAP_KEY(Key_Minus, Key_Minus);
        MAP_KEY(Key_Period, Key_Period);
        MAP_KEY(Key_Slash, Key_Slash);
        MAP_KEY(Key_0, Key_0);
        MAP_KEY(Key_1, Key_1);
        MAP_KEY(Key_2, Key_2);
        MAP_KEY(Key_3, Key_3);
        MAP_KEY(Key_4, Key_4);
        MAP_KEY(Key_5, Key_5);
        MAP_KEY(Key_6, Key_6);
        MAP_KEY(Key_7, Key_7);
        MAP_KEY(Key_8, Key_8);
        MAP_KEY(Key_9, Key_9);
        MAP_KEY(Key_Colon, Key_Colon);
        MAP_KEY(Key_Semicolon, Key_Semicolon);
        MAP_KEY(Key_LessThan, Key_LessThan);
        MAP_KEY(Key_Equal, Key_Equal);
        MAP_KEY(Key_GreaterThan, Key_GreaterThan);
        MAP_KEY(Key_QuestionMark, Key_QuestionMark);
        MAP_KEY(Key_AtSign, Key_AtSign);
        MAP_KEY(Key_A, Key_A);
        MAP_KEY(Key_B, Key_B);
        MAP_KEY(Key_C, Key_C);
        MAP_KEY(Key_D, Key_D);
        MAP_KEY(Key_E, Key_E);
        MAP_KEY(Key_F, Key_F);
        MAP_KEY(Key_G, Key_G);
        MAP_KEY(Key_H, Key_H);
        MAP_KEY(Key_I, Key_I);
        MAP_KEY(Key_J, Key_J);
        MAP_KEY(Key_K, Key_K);
        MAP_KEY(Key_L, Key_L);
        MAP_KEY(Key_M, Key_M);
        MAP_KEY(Key_N, Key_N);
        MAP_KEY(Key_O, Key_O);
        MAP_KEY(Key_P, Key_P);
        MAP_KEY(Key_Q, Key_Q);
        MAP_KEY(Key_R, Key_R);
        MAP_KEY(Key_S, Key_S);
        MAP_KEY(Key_T, Key_T);
        MAP_KEY(Key_U, Key_U);
        MAP_KEY(Key_V, Key_V);
        MAP_KEY(Key_W, Key_W);
        MAP_KEY(Key_X, Key_X);
        MAP_KEY(Key_Y, Key_Y);
        MAP_KEY(Key_Z, Key_Z);
        MAP_KEY(Key_LeftBracket, Key_LeftBracket);
        MAP_KEY(Key_RightBracket, Key_RightBracket);
        MAP_KEY(Key_Backslash, Key_Backslash);
        MAP_KEY(Key_Circumflex, Key_Circumflex);
        MAP_KEY(Key_Underscore, Key_Underscore);
        MAP_KEY(Key_LeftBrace, Key_LeftBrace);
        MAP_KEY(Key_RightBrace, Key_RightBrace);
        MAP_KEY(Key_Pipe, Key_Pipe);
        MAP_KEY(Key_Tilde, Key_Tilde);
        MAP_KEY(Key_Backtick, Key_Backtick);
        MAP_KEY(Key_LeftSuper, Key_LeftSuper);
        MAP_KEY(Key_RightSuper, Key_RightSuper);
        MAP_KEY(Key_BrowserSearch, Key_BrowserSearch);
        MAP_KEY(Key_BrowserFavorites, Key_BrowserFavorites);
        MAP_KEY(Key_BrowserHome, Key_BrowserHome);
        MAP_KEY(Key_PreviousTrack, Key_PreviousTrack);
        MAP_KEY(Key_BrowserBack, Key_BrowserBack);
        MAP_KEY(Key_BrowserForward, Key_BrowserForward);
        MAP_KEY(Key_BrowserRefresh, Key_BrowserRefresh);
        MAP_KEY(Key_BrowserStop, Key_BrowserStop);
        MAP_KEY(Key_VolumeDown, Key_VolumeDown);
        MAP_KEY(Key_VolumeUp, Key_VolumeUp);
        MAP_KEY(Key_Wake, Key_Wake);
        MAP_KEY(Key_Sleep, Key_Sleep);
        MAP_KEY(Key_NextTrack, Key_NextTrack);
        MAP_KEY(Key_MediaSelect, Key_MediaSelect);
        MAP_KEY(Key_Email, Key_Email);
        MAP_KEY(Key_MyComputer, Key_MyComputer);
        MAP_KEY(Key_Power, Key_Power);
        MAP_KEY(Key_Stop, Key_Stop);
        MAP_KEY(Key_LeftGUI, Key_LeftGUI);
        MAP_KEY(Key_Mute, Key_Mute);
        MAP_KEY(Key_RightGUI, Key_RightGUI);
        MAP_KEY(Key_Calculator, Key_Calculator);
        MAP_KEY(Key_Apps, Key_Apps);
        MAP_KEY(Key_PlayPause, Key_PlayPause);
        MAP_KEY(Key_Menu, Key_Menu);
    }
}

void OutOfProcessWebView::enqueue_native_event(Web::KeyEvent::Type type, GUI::KeyEvent const& event)
{
    auto key_code = web_key_code_from_gui_key_code(event.key());
    auto modifiers = web_modifiers_from_gui_modifiers(event.modifiers());

    enqueue_input_event(Web::KeyEvent { type, key_code, modifiers, event.code_point(), make<KeyData>(event) });
}

void OutOfProcessWebView::finish_handling_key_event(Web::KeyEvent const& key_event)
{
    // First, we give our superclass a chance to handle the event.
    //
    // If it does not, we dispatch the event to our parent widget, but limited such that it will never bubble up to the
    // Window. (Otherwise, it would then dispatch the event to us since we are the focused widget, and it would go around
    // indefinitely.)
    //
    // Finally, any unhandled KeyDown events are propagated to trigger any shortcut Actions.
    auto& chrome_data = verify_cast<KeyData>(*key_event.chrome_data);
    auto& event = *chrome_data.event;

    switch (key_event.type) {
    case Web::KeyEvent::Type::KeyDown:
        Super::keydown_event(event);
        break;
    case Web::KeyEvent::Type::KeyUp:
        Super::keyup_event(event);
        break;
    }

    if (!event.is_accepted()) {
        parent_widget()->dispatch_event(event, window());

        // NOTE: If other events can ever trigger shortcuts, propagate those here.
        if (!event.is_accepted() && event.type() == GUI::Event::Type::KeyDown)
            window()->propagate_shortcuts(static_cast<GUI::KeyEvent&>(event), this);
    }
}

void OutOfProcessWebView::set_content_scales_to_viewport(bool b)
{
    m_content_scales_to_viewport = b;
}

}
