/*
 * menu-ops-qt.cc
 * Copyright 2016 John Lindgren
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

#include "menu-ops.h"

#include <string.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QProcess>
#include <QUrl>

#include <libaudcore/audstrings.h>
#include <libaudcore/i18n.h>
#include <libaudcore/interface.h>
#include <libaudcore/playlist.h>
#include <libaudcore/vfs.h>
#include <libaudqt/libaudqt.h>

void pl_copy ()
{
    auto list = Playlist::active_playlist ();
    int entries = list.n_entries ();

    if (! list.n_selected ())
        return;

    list.cache_selected ();

    QList<QUrl> urls;
    for (int i = 0; i < entries; i ++)
    {
        if (list.entry_selected (i))
            urls.append (QString (list.entry_filename (i)));
    }

    auto data = new QMimeData;
    data->setUrls (urls);
    QApplication::clipboard ()->setMimeData (data);
}

void pl_cut ()
{
    pl_copy ();
    pl_remove_selected ();
}

static void paste_to (Playlist list, int pos)
{
    auto data = QApplication::clipboard ()->mimeData ();
    if (! data->hasUrls ())
        return;

    Index<PlaylistAddItem> items;
    for (auto & url : data->urls ())
        items.append (String (url.toEncoded ()));

    list.insert_items (pos, std::move (items), false);
}

void pl_paste ()
{
    auto list = Playlist::active_playlist ();
    paste_to (list, list.get_focus ());
}

void pl_paste_end ()
{
    paste_to (Playlist::active_playlist (), -1);
}

void pl_song_info ()
{
    audqt::infowin_show_selected (Playlist::active_playlist ());
}

/* Opens the containing folder and highlights the given file in it, so that the
 * file can still be identified when the folder holds many entries. */
static void show_in_folder (const QString & path)
{
#ifdef _WIN32
    /* explorer.exe /select opens the folder and selects the file */
    if (QProcess::startDetached ("explorer.exe",
            {QStringLiteral ("/select,") + QDir::toNativeSeparators (path)}))
        return;
#elif defined(__APPLE__)
    if (QProcess::startDetached ("open", {QStringLiteral ("-R"), path}))
        return;
#endif

    /* fall back to just opening the containing folder */
    QDesktopServices::openUrl (QUrl::fromLocalFile (QFileInfo (path).absolutePath ()));
}

void pl_open_folder ()
{
    auto list = Playlist::active_playlist ();
    int focus = list.get_focus ();

    String filename = list.entry_filename (focus);
    if (! filename)
        return;

    const char * slash = strrchr (filename, '/');
    if (! slash)
        return;

    /* don't trim trailing slash, it may be important */
    StringBuf folder = str_copy (filename, slash + 1 - filename);

    /* check that it's really a folder so as to prevent opening random
     * files from a malicious playlist */
    if (! VFSFile::test_file (folder, VFS_IS_DIR))
    {
        aud_ui_show_error (str_printf
            (_("%s does not appear to be a valid folder."), & filename[0]));
        return;
    }

    /* convert a file:// URI, if any, into a local path */
    StringBuf local = uri_to_filename (filename, false);
    show_in_folder (QString::fromUtf8 (local ? local : (const char *) filename));
}
