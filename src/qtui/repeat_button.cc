/*
 * repeat_button.cc
 * Copyright 2026 Audacious contributors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions, and the following disclaimer in the documentation
 *    provided with the distribution.
 *
 * This software is provided "as is" and without any warranty, express or
 * implied. In no event shall the authors be liable for any damages arising from
 * the use of this software.
 */

#include "repeat_button.h"

#include <libaudcore/hook.h>
#include <libaudcore/i18n.h>
#include <libaudcore/runtime.h>

#include <QIcon>

enum class RepeatMode
{
    Off,
    Playlist,
    One
};

static RepeatMode get_repeat_mode()
{
    if (!aud_get_bool("repeat"))
        return RepeatMode::Off;

    return aud_get_bool("no_playlist_advance") ? RepeatMode::One
                                               : RepeatMode::Playlist;
}

static void set_repeat_mode(RepeatMode mode)
{
    aud_set_bool("repeat", mode != RepeatMode::Off);
    aud_set_bool("no_playlist_advance", mode == RepeatMode::One);
}

RepeatButton::RepeatButton(QWidget * parent) : QToolButton(parent)
{
    setAutoRaise(true);
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);

    connect(this, &QToolButton::clicked, this, &RepeatButton::cycle);

    hook_associate("set repeat", on_config_changed, this);
    hook_associate("set no_playlist_advance", on_config_changed, this);

    update_state();
}

RepeatButton::~RepeatButton()
{
    hook_dissociate("set repeat", on_config_changed);
    hook_dissociate("set no_playlist_advance", on_config_changed);
}

void RepeatButton::cycle()
{
    switch (get_repeat_mode())
    {
    case RepeatMode::Off:
        set_repeat_mode(RepeatMode::Playlist);
        break;
    case RepeatMode::Playlist:
        set_repeat_mode(RepeatMode::One);
        break;
    case RepeatMode::One:
        set_repeat_mode(RepeatMode::Off);
        break;
    }

    update_state();
}

void RepeatButton::update_state()
{
    RepeatMode mode = get_repeat_mode();
    setChecked(mode != RepeatMode::Off);

    switch (mode)
    {
    case RepeatMode::Off:
        setIcon(QIcon::fromTheme("media-playlist-repeat"));
        setToolTip(_("Repeat: off (click to repeat the playlist)"));
        break;
    case RepeatMode::Playlist:
        setIcon(QIcon::fromTheme("media-playlist-repeat"));
        setToolTip(_("Repeat: playlist (click to repeat one song)"));
        break;
    case RepeatMode::One:
        setIcon(QIcon::fromTheme("media-playlist-repeat-song"));
        setToolTip(_("Repeat: one song (click to turn repeat off)"));
        break;
    }
}
