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
