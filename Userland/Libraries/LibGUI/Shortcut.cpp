/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2022, Geordie Hall <me@geordiehall.com>
 * Copyright (c) 2026, Eduardo Casadei <educasadei@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/ByteString.h>
#include <AK/StringBuilder.h>
#include <AK/Vector.h>
#include <LibGUI/Shortcut.h>
#include <LibUnicode/CharacterTypes.h>
#include <string.h>

namespace GUI {

Shortcut::Shortcut(u8 modifiers, u32 code_point)
    : m_type(Type::Keyboard)
    , m_modifiers(modifiers)
    , m_code_point(Unicode::to_unicode_uppercase(code_point))
{
}

Shortcut::Shortcut(u32 code_point)
    : m_type(Type::Keyboard)
    , m_code_point(Unicode::to_unicode_uppercase(code_point))
{
}

bool Shortcut::matches(KeyEvent const& event) const
{
    if (m_type != Type::Keyboard)
        return false;
    if (m_modifiers != event.modifiers())
        return false;

    if (m_keyboard_key != KeyCode::Key_Invalid && m_keyboard_key == event.key())
        return true;
    if (m_code_point != 0 && m_code_point == Unicode::to_unicode_uppercase(event.code_point()))
        return true;

    return false;
}

bool Shortcut::matches(MouseEvent const& event) const
{
    if (m_type != Type::Mouse)
        return false;
    if (m_modifiers != event.modifiers())
        return false;

    return m_mouse_button == event.button();
}

ByteString Shortcut::to_byte_string() const
{
    Vector<StringView, 6> parts;

    if (m_modifiers & Mod_Ctrl)
        parts.append("Ctrl"sv);
    if (m_modifiers & Mod_Shift)
        parts.append("Shift"sv);
    if (m_modifiers & Mod_Alt)
        parts.append("Alt"sv);
    if (m_modifiers & Mod_AltGr)
        parts.append("AltGr"sv);
    if (m_modifiers & Mod_Super)
        parts.append("Super"sv);

    StringBuilder builder;

    if (m_type == Type::Keyboard) {
        if (m_code_point != 0)
            builder.append_code_point(m_code_point);
        else if (auto* key_name = key_code_to_string(m_keyboard_key))
            builder.append(key_name, strlen(key_name));
        else
            builder.append("(Invalid)"sv);
    } else {
        if (m_mouse_button != MouseButton::None)
            builder.appendff("Mouse {}", mouse_button_to_string(m_mouse_button));
        else
            builder.append("(Invalid)"sv);
    }

    parts.append(builder.string_view());

    StringBuilder joiner;
    joiner.join('+', parts);
    return joiner.to_byte_string();
}

}
