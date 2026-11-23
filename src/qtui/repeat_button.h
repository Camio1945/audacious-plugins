/*
 * repeat_button.h
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

#ifndef REPEAT_BUTTON_H
#define REPEAT_BUTTON_H

#include <QToolButton>

/* Cycles through the repeat modes: off -> repeat playlist -> repeat one song */
class RepeatButton : public QToolButton
{
public:
    RepeatButton(QWidget * parent = nullptr);
    ~RepeatButton() override;

private:
    void cycle();
    void update_state();

    static void on_config_changed(void *, void * me)
    {
        ((RepeatButton *)me)->update_state();
    }
};

#endif
