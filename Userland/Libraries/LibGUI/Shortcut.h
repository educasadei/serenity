/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2022, Geordie Hall <me@geordiehall.com>
 * Copyright (c) 2026, Eduardo Casadei <educasadei@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Traits.h>
#include <Kernel/API/KeyCode.h>
#include <LibGUI/Event.h>

namespace GUI {

class Shortcut {
public:
    Shortcut() = default;

    Shortcut(u8 modifiers, u32 code_point);
    Shortcut(u32 code_point);

    Shortcut(u8 modifiers, KeyCode key)
        : m_type(Type::Keyboard)
        , m_modifiers(modifiers)
        , m_keyboard_key(key)
    {
    }
    Shortcut(KeyCode key)
        : m_type(Type::Keyboard)
        , m_modifiers(0)
        , m_keyboard_key(key)
    {
    }
    Shortcut(u8 modifiers, MouseButton button)
        : m_type(Type::Mouse)
        , m_modifiers(modifiers)
        , m_mouse_button(button)
    {
    }
    Shortcut(MouseButton button)
        : m_type(Type::Mouse)
        , m_modifiers(0)
        , m_mouse_button(button)
    {
    }

    enum class Type : u8 {
        Keyboard,
        Mouse,
    };

    ByteString to_byte_string() const;
    Type type() const { return m_type; }
    bool is_valid() const { return m_type == Type::Keyboard ? ((m_keyboard_key != Key_Invalid) != (m_code_point != 0)) : (m_mouse_button != MouseButton::None); }
    u8 modifiers() const { return m_modifiers; }

    KeyCode key() const
    {
        VERIFY(m_type == Type::Keyboard);
        return m_keyboard_key;
    }

    u32 code_point() const
    {
        VERIFY(m_type == Type::Keyboard);
        return m_code_point;
    }

    MouseButton mouse_button() const
    {
        VERIFY(m_type == Type::Mouse);
        return m_mouse_button;
    }

    bool matches(KeyEvent const& event) const;
    bool matches(MouseEvent const& event) const;

    bool operator==(Shortcut const& other) const
    {
        return m_modifiers == other.m_modifiers && m_type == other.m_type && m_keyboard_key == other.m_keyboard_key && m_code_point == other.m_code_point && m_mouse_button == other.m_mouse_button;
    }

private:
    Type m_type { Type::Keyboard };
    u8 m_modifiers { 0 };
    KeyCode m_keyboard_key { KeyCode::Key_Invalid };
    u32 m_code_point { 0 };
    MouseButton m_mouse_button { MouseButton::None };
};

}

namespace AK {

template<>
struct Traits<GUI::Shortcut> : public DefaultTraits<GUI::Shortcut> {
    static unsigned hash(const GUI::Shortcut& shortcut)
    {
        auto hash = pair_int_hash(shortcut.modifiers(), (u32)shortcut.type());
        if (shortcut.type() == GUI::Shortcut::Type::Keyboard) {
            hash = pair_int_hash(hash, shortcut.code_point());
            return pair_int_hash(hash, shortcut.key());
        } else {
            return pair_int_hash(hash, shortcut.mouse_button());
        }
    }
};

}
