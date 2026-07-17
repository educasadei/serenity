/*
 * Copyright (c) 2026,
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/API/KeyCode.h>
#include <ctype.h>

#define GUI_ENUMERATE_KEYS                                 \
    __ENUMERATE_KEY(Invalid, "Invalid")                    \
    __ENUMERATE_KEY(Escape, "Escape")                      \
    __ENUMERATE_KEY(Tab, "Tab")                            \
    __ENUMERATE_KEY(Backspace, "Backspace")                \
    __ENUMERATE_KEY(Return, "Return")                      \
    __ENUMERATE_KEY(Insert, "Insert")                      \
    __ENUMERATE_KEY(Delete, "Delete")                      \
    __ENUMERATE_KEY(PrintScreen, "PrintScreen")            \
    __ENUMERATE_KEY(PauseBreak, "PauseBreak")              \
    __ENUMERATE_KEY(SysRq, "SysRq")                        \
    __ENUMERATE_KEY(Home, "Home")                          \
    __ENUMERATE_KEY(End, "End")                            \
    __ENUMERATE_KEY(Left, "Left")                          \
    __ENUMERATE_KEY(Up, "Up")                              \
    __ENUMERATE_KEY(Right, "Right")                        \
    __ENUMERATE_KEY(Down, "Down")                          \
    __ENUMERATE_KEY(PageUp, "PageUp")                      \
    __ENUMERATE_KEY(PageDown, "PageDown")                  \
    __ENUMERATE_KEY(LeftShift, "LeftShift")                \
    __ENUMERATE_KEY(RightShift, "RightShift")              \
    __ENUMERATE_KEY(LeftControl, "LeftControl")            \
    __ENUMERATE_KEY(RightControl, "RightControl")          \
    __ENUMERATE_KEY(LeftAlt, "LeftAlt")                    \
    __ENUMERATE_KEY(RightAlt, "RightAlt")                  \
    __ENUMERATE_KEY(AltGr, "AltGr")                        \
    __ENUMERATE_KEY(CapsLock, "CapsLock")                  \
    __ENUMERATE_KEY(NumLock, "NumLock")                    \
    __ENUMERATE_KEY(ScrollLock, "ScrollLock")              \
    __ENUMERATE_KEY(F1, "F1")                              \
    __ENUMERATE_KEY(F2, "F2")                              \
    __ENUMERATE_KEY(F3, "F3")                              \
    __ENUMERATE_KEY(F4, "F4")                              \
    __ENUMERATE_KEY(F5, "F5")                              \
    __ENUMERATE_KEY(F6, "F6")                              \
    __ENUMERATE_KEY(F7, "F7")                              \
    __ENUMERATE_KEY(F8, "F8")                              \
    __ENUMERATE_KEY(F9, "F9")                              \
    __ENUMERATE_KEY(F10, "F10")                            \
    __ENUMERATE_KEY(F11, "F11")                            \
    __ENUMERATE_KEY(F12, "F12")                            \
    __ENUMERATE_KEY(Space, "Space")                        \
    __ENUMERATE_KEY_CODE_POINT(ExclamationPoint, "!", '!') \
    __ENUMERATE_KEY_CODE_POINT(DoubleQuote, "\"", '"')     \
    __ENUMERATE_KEY_CODE_POINT(Hashtag, "#", '#')          \
    __ENUMERATE_KEY_CODE_POINT(Dollar, "$", '$')           \
    __ENUMERATE_KEY_CODE_POINT(Percent, "%", '%')          \
    __ENUMERATE_KEY_CODE_POINT(Ampersand, "&", '&')        \
    __ENUMERATE_KEY_CODE_POINT(Apostrophe, "'", '\'')      \
    __ENUMERATE_KEY_CODE_POINT(LeftParen, "(", '(')        \
    __ENUMERATE_KEY_CODE_POINT(RightParen, ")", ')')       \
    __ENUMERATE_KEY_CODE_POINT(Asterisk, "*", '*')         \
    __ENUMERATE_KEY_CODE_POINT(Plus, "+", '+')             \
    __ENUMERATE_KEY_CODE_POINT(Comma, ",", ',')            \
    __ENUMERATE_KEY_CODE_POINT(Minus, "-", '-')            \
    __ENUMERATE_KEY_CODE_POINT(Period, ".", '.')           \
    __ENUMERATE_KEY_CODE_POINT(Slash, "/", '/')            \
    __ENUMERATE_KEY_CODE_POINT(0, "0", '0')                \
    __ENUMERATE_KEY_CODE_POINT(1, "1", '1')                \
    __ENUMERATE_KEY_CODE_POINT(2, "2", '2')                \
    __ENUMERATE_KEY_CODE_POINT(3, "3", '3')                \
    __ENUMERATE_KEY_CODE_POINT(4, "4", '4')                \
    __ENUMERATE_KEY_CODE_POINT(5, "5", '5')                \
    __ENUMERATE_KEY_CODE_POINT(6, "6", '6')                \
    __ENUMERATE_KEY_CODE_POINT(7, "7", '7')                \
    __ENUMERATE_KEY_CODE_POINT(8, "8", '8')                \
    __ENUMERATE_KEY_CODE_POINT(9, "9", '9')                \
    __ENUMERATE_KEY_CODE_POINT(Colon, ":", ':')            \
    __ENUMERATE_KEY_CODE_POINT(Semicolon, ";", ';')        \
    __ENUMERATE_KEY_CODE_POINT(LessThan, "<", '<')         \
    __ENUMERATE_KEY_CODE_POINT(Equal, "=", '=')            \
    __ENUMERATE_KEY_CODE_POINT(GreaterThan, ">", '>')      \
    __ENUMERATE_KEY_CODE_POINT(QuestionMark, "?", '?')     \
    __ENUMERATE_KEY_CODE_POINT(AtSign, "@", '@')           \
    __ENUMERATE_KEY_CODE_POINT(A, "A", 'A')                \
    __ENUMERATE_KEY_CODE_POINT(B, "B", 'B')                \
    __ENUMERATE_KEY_CODE_POINT(C, "C", 'C')                \
    __ENUMERATE_KEY_CODE_POINT(D, "D", 'D')                \
    __ENUMERATE_KEY_CODE_POINT(E, "E", 'E')                \
    __ENUMERATE_KEY_CODE_POINT(F, "F", 'F')                \
    __ENUMERATE_KEY_CODE_POINT(G, "G", 'G')                \
    __ENUMERATE_KEY_CODE_POINT(H, "H", 'H')                \
    __ENUMERATE_KEY_CODE_POINT(I, "I", 'I')                \
    __ENUMERATE_KEY_CODE_POINT(J, "J", 'J')                \
    __ENUMERATE_KEY_CODE_POINT(K, "K", 'K')                \
    __ENUMERATE_KEY_CODE_POINT(L, "L", 'L')                \
    __ENUMERATE_KEY_CODE_POINT(M, "M", 'M')                \
    __ENUMERATE_KEY_CODE_POINT(N, "N", 'N')                \
    __ENUMERATE_KEY_CODE_POINT(O, "O", 'O')                \
    __ENUMERATE_KEY_CODE_POINT(P, "P", 'P')                \
    __ENUMERATE_KEY_CODE_POINT(Q, "Q", 'Q')                \
    __ENUMERATE_KEY_CODE_POINT(R, "R", 'R')                \
    __ENUMERATE_KEY_CODE_POINT(S, "S", 'S')                \
    __ENUMERATE_KEY_CODE_POINT(T, "T", 'T')                \
    __ENUMERATE_KEY_CODE_POINT(U, "U", 'U')                \
    __ENUMERATE_KEY_CODE_POINT(V, "V", 'V')                \
    __ENUMERATE_KEY_CODE_POINT(W, "W", 'W')                \
    __ENUMERATE_KEY_CODE_POINT(X, "X", 'X')                \
    __ENUMERATE_KEY_CODE_POINT(Y, "Y", 'Y')                \
    __ENUMERATE_KEY_CODE_POINT(Z, "Z", 'Z')                \
    __ENUMERATE_KEY_CODE_POINT(LeftBracket, "[", '[')      \
    __ENUMERATE_KEY_CODE_POINT(RightBracket, "]", ']')     \
    __ENUMERATE_KEY_CODE_POINT(Backslash, "\\", '\\')      \
    __ENUMERATE_KEY_CODE_POINT(Circumflex, "^", '^')       \
    __ENUMERATE_KEY_CODE_POINT(Underscore, "_", '_')       \
    __ENUMERATE_KEY_CODE_POINT(LeftBrace, "{", '{')        \
    __ENUMERATE_KEY_CODE_POINT(RightBrace, "}", '}')       \
    __ENUMERATE_KEY_CODE_POINT(Pipe, "|", '|')             \
    __ENUMERATE_KEY_CODE_POINT(Tilde, "~", '~')            \
    __ENUMERATE_KEY_CODE_POINT(Backtick, "`", '`')         \
    __ENUMERATE_KEY(LeftSuper, "LeftSuper")                \
    __ENUMERATE_KEY(RightSuper, "RightSuper")              \
    __ENUMERATE_KEY(BrowserSearch, "BrowserSearch")        \
    __ENUMERATE_KEY(BrowserFavorites, "BrowserFavorites")  \
    __ENUMERATE_KEY(BrowserHome, "BrowserHome")            \
    __ENUMERATE_KEY(PreviousTrack, "PreviousTrack")        \
    __ENUMERATE_KEY(BrowserBack, "BrowserBack")            \
    __ENUMERATE_KEY(BrowserForward, "BrowserForward")      \
    __ENUMERATE_KEY(BrowserRefresh, "BrowserRefresh")      \
    __ENUMERATE_KEY(BrowserStop, "BrowserStop")            \
    __ENUMERATE_KEY(VolumeDown, "VolumeDown")              \
    __ENUMERATE_KEY(VolumeUp, "VolumeUp")                  \
    __ENUMERATE_KEY(Wake, "Wake")                          \
    __ENUMERATE_KEY(Sleep, "Sleep")                        \
    __ENUMERATE_KEY(NextTrack, "NextTrack")                \
    __ENUMERATE_KEY(MediaSelect, "MediaSelect")            \
    __ENUMERATE_KEY(Email, "Email")                        \
    __ENUMERATE_KEY(MyComputer, "MyComputer")              \
    __ENUMERATE_KEY(Power, "Power")                        \
    __ENUMERATE_KEY(Stop, "Stop")                          \
    __ENUMERATE_KEY(LeftGUI, "LeftGUI")                    \
    __ENUMERATE_KEY(Mute, "Mute")                          \
    __ENUMERATE_KEY(RightGUI, "RightGUI")                  \
    __ENUMERATE_KEY(Calculator, "Calculator")              \
    __ENUMERATE_KEY(Apps, "Apps")                          \
    __ENUMERATE_KEY(PlayPause, "PlayPause")                \
    __ENUMERATE_KEY(Menu, "Menu")

namespace GUI {
enum Key {
#define __ENUMERATE_KEY(name, ui_name) Key_##name,
#define __ENUMERATE_KEY_CODE_POINT(name, ui_name, code_point) Key_##name,
    GUI_ENUMERATE_KEYS
#undef __ENUMERATE_KEY
#undef __ENUMERATE_KEY_CODE_POINT
};

inline Key key_from_key_event(KeyCode key, u32 key_code_point)
{
    switch (toupper(key_code_point)) {
#define __ENUMERATE_KEY(name, ui_name)
#define __ENUMERATE_KEY_CODE_POINT(name, ui_name, code_point) \
    case code_point:                                          \
        return Key::Key_##name;
        GUI_ENUMERATE_KEYS
#undef __ENUMERATE_KEY
#undef __ENUMERATE_KEY_CODE_POINT
    }

    switch (key) {
#define __ENUMERATE_KEY_CODE_POINT(name, ui_name, code_point)
#define __ENUMERATE_KEY(name, ui_name) \
    case KeyCode::Key_##name:          \
        return Key::Key_##name;
        GUI_ENUMERATE_KEYS
#undef __ENUMERATE_KEY
#undef __ENUMERATE_KEY_CODE_POINT
    default:
        return Key::Key_Invalid;
    }
}

inline u32 key_to_code_point(Key key)
{
    switch (key) {
#define __ENUMERATE_KEY(name, ui_name) \
    case Key::Key_##name:              \
        return 0;
#define __ENUMERATE_KEY_CODE_POINT(name, ui_name, code_point) \
    case Key::Key_##name:                                     \
        return code_point;
        GUI_ENUMERATE_KEYS
#undef __ENUMERATE_KEY
#undef __ENUMERATE_KEY_CODE_POINT
    default:
        return 0;
    }
}

inline char const* key_to_string(Key key)
{
    switch (key) {
#define __ENUMERATE_KEY(name, ui_name) \
    case Key::Key_##name:              \
        return ui_name;
#define __ENUMERATE_KEY_CODE_POINT(name, ui_name, code_point) \
    case Key::Key_##name:                                     \
        return ui_name;
        GUI_ENUMERATE_KEYS
#undef __ENUMERATE_KEY
#undef __ENUMERATE_KEY_CODE_POINT
    default:
        return nullptr;
    }
}
}
