/***************************************************************************
 *   Copyright (C) 2015~2015 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "common.h"
#include "config.h"
#include <fcitx-utils/utils.h>
#include <fcitx-config/xdg.h>

char* FcitxZhuyinGetSysPath()
{
    char* syspath = NULL;
    /* portable detect here */
    if (getenv("FCITXDIR")) {
        syspath = fcitx_utils_get_fcitx_path_with_filename("datadir", "libzhuyin/data");
    } else {
        syspath = strdup(LIBZHUYIN_PKGDATADIR "/data");
    }
    return syspath;
}


char* FcitxZhuyinGetUserPath()
{
    char* user_path = NULL;
    FILE* fp = FcitxXDGGetFileUserWithPrefix("zhuyin", "zhuyin_data/.place_holder", "w", NULL);
    if (fp)
        fclose(fp);
    FcitxXDGGetFileUserWithPrefix("zhuyin", "zhuyin_data", NULL, &user_path);
    return user_path;
}
