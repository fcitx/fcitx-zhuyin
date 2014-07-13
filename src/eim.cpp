/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
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

#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <zhuyin.h>
#include <fcitx/ime.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-config/xdg.h>
#include <fcitx-utils/log.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-utils/utils.h>
#include <fcitx/instance.h>
#include <fcitx/keys.h>
#include <fcitx/module.h>
#include <fcitx/context.h>
#include <fcitx/module/punc/fcitx-punc.h>
#include <string>
#include <libintl.h>

#include "config.h"
#include "eim.h"
#include "enummap.h"
#include "common.h"
#include "utils.h"

#define FCITX_LIBZHUYIN_MAX(x, y) ((x) > (y)? (x) : (y))
#define FCITX_LIBZHUYIN_MIN(x, y) ((x) < (y)? (x) : (y))
extern "C" {
    FCITX_DEFINE_PLUGIN(fcitx_zhuyin, ime2, FcitxIMClass2) = {
        FcitxZhuyinCreate,
        FcitxZhuyinDestroy,
        FcitxZhuyinReloadConfig,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };
}

typedef struct _FcitxZhuyinCandWord {
    boolean ispunc;
    guint idx;
} FcitxZhuyinCandWord;

typedef struct _FcitxZhuyinFixed {
    guint len;
} FcitxZhuyinFixed;

CONFIG_DEFINE_LOAD_AND_SAVE(FcitxZhuyinConfig, FcitxZhuyinConfig, "fcitx-zhuyin")

static void FcitxZhuyinReconfigure(FcitxZhuyinAddonInstance* zhuyin);
static guint LibPinyinGetOffset(FcitxZhuyin* zhuyin);
static void FcitxZhuyinSave(void* arg);
static bool LibPinyinCheckZhuyinKey(FcitxKeySym sym, FCITX_ZHUYIN_LAYOUT layout, boolean useTone) ;
char* LibPinyinGetSentence(FcitxZhuyin* zhuyin);

// check order in FCITX_ZHUYIN_LAYOUT
static const char* input_keys [] = {
    "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-",       /* standard kb */
    "1234567890-qwertyuiopasdfghjkl;zxcvbn",       /* IBM */
    "2wsx3edcrfvtgb6yhnujm8ik,9ol.0p;/-['=",       /* Gin-yieh */
    "bpmfdtnlvkhg7c,./j;'sexuaorwiqzy890-=",       /* ET  */
    "bpmfdtnlvkhgvcgycjqwsexuaorwiqzpmntlh",       /* ET26 */
    "bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkll",       /* HSU */
    "\\,.125;abdefijkopquxyghm089[clnrstvwz",       /* dvorak */
    "abccdfghjjklmnprstvvzeuxaeghikllmnowy",       /* hsu dvorak */
};
static_assert(sizeof(input_keys) / sizeof(input_keys[0]) == FCITX_ZHUYIN_LAYOUT_LAST, "error");

static const char* tone_keys [] = {
    "7634 ",       /* standard kb */
    "/m,. ",       /* IBM */
    "1qaz ",       /* Gin-yieh */
    "1234 ",       /* ET  */
    "dfjk ",       /* ET26 */
    "sdfj ",       /* HSU */
    "3467 ",       /* dvorak */
    "dfjs "
};

static_assert(sizeof(tone_keys) / sizeof(tone_keys[0]) == FCITX_ZHUYIN_LAYOUT_LAST, "error");

static const FcitxKeyState candidateModifierMap[] = {
    FcitxKeyState_Ctrl,
    FcitxKeyState_Alt,
    FcitxKeyState_Shift,
};

static const FcitxHotkey FCITX_LIBZHUYIN_SHIFT_ENTER[2] = {
    {NULL, FcitxKey_Return, FcitxKeyState_Shift},
    {NULL, FcitxKey_None, FcitxKeyState_None}
};

bool LibPinyinCheckZhuyinKey(FcitxKeySym sym, FCITX_ZHUYIN_LAYOUT layout, boolean useTone)
{
    char key = sym & 0xff;
    const char* keys = input_keys[layout];
    const char* tones = tone_keys[layout];
    if (strchr(keys, key))
        return true;

    if (useTone && strchr(tones, key))
        return true;

    return false;
}

guint LibPinyinGetOffset(FcitxZhuyin* zhuyin)
{
    GArray* array = zhuyin->fixed_string;
    guint sum = 0;
    for (guint i = 0; i < array->len; i ++) {
        FcitxZhuyinFixed* f = &g_array_index(array, FcitxZhuyinFixed, i);
        sum += f->len;
    }
    return sum;
}

guint LibPinyinGetPinyinOffset(FcitxZhuyin* zhuyin)
{
    auto offset = LibPinyinGetOffset(zhuyin);
    guint16 pyoffset = 0;

    guint len;

    zhuyin_get_n_zhuyin(zhuyin->inst, &len);

    auto i = FCITX_LIBZHUYIN_MIN(offset, len);
    if (i >= 1) {
        PinyinKeyPos* pykey = NULL;
        zhuyin_get_zhuyin_key_rest(zhuyin->inst, i - 1, &pykey);
        zhuyin_get_zhuyin_key_rest_positions(zhuyin->inst, pykey, NULL, &pyoffset);
    }

    return pyoffset;
}

/**
 * @brief Reset the status.
 *
 **/
void FcitxZhuyinReset(void* arg)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*) arg;
    zhuyin->buf[0] = '\0';
    zhuyin->cursor_pos = 0;

    if (zhuyin->fixed_string->len > 0)
        g_array_remove_range(zhuyin->fixed_string, 0, zhuyin->fixed_string->len);
    if (zhuyin->inst)
        zhuyin_reset(zhuyin->inst);
}

size_t FcitxZhuyinParse(FcitxZhuyin* zhuyin, const char* str)
{
    return zhuyin_parse_more_chewings(zhuyin->inst, str);
}

/**
 * @brief Process Key Input and return the status
 *
 * @param keycode keycode from XKeyEvent
 * @param state state from XKeyEvent
 * @param count count from XKeyEvent
 * @return INPUT_RETURN_VALUE
 **/
INPUT_RETURN_VALUE FcitxZhuyinDoInput(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*) arg;
    FcitxZhuyinConfig* config = &zhuyin->owner->config;
    FcitxInstance* instance = zhuyin->owner->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(zhuyin->owner->owner);

    if (FcitxHotkeyIsHotKeySimple(sym, state)) {
        /* there is some special case that ';' is used */
        if (FcitxHotkeyIsHotKeyLAZ(sym, state)
                || sym == '\''
                || (LibPinyinCheckZhuyinKey(sym, config->zhuyinLayout, config->useTone))
           ) {
            if (strlen(zhuyin->buf) == 0 && (sym == '\'' || sym == ';'))
                return IRV_TO_PROCESS;

            if (strlen(zhuyin->buf) < MAX_PINYIN_INPUT) {
                size_t len = strlen(zhuyin->buf);
                if (zhuyin->buf[zhuyin->cursor_pos] != 0) {
                    memmove(zhuyin->buf + zhuyin->cursor_pos + 1, zhuyin->buf + zhuyin->cursor_pos, len - zhuyin->cursor_pos);
                }
                zhuyin->buf[len + 1] = 0;
                zhuyin->buf[zhuyin->cursor_pos] = (char)(sym & 0xff);
                zhuyin->cursor_pos ++;

                size_t parselen = FcitxZhuyinParse(zhuyin, zhuyin->buf);

                if (parselen == 0 && strlen(zhuyin->buf) == 1
                        && zhuyin->owner->config.chewingIncomplete) {
                    FcitxZhuyinReset(zhuyin);
                    return IRV_TO_PROCESS;
                }
                return IRV_DISPLAY_CANDWORDS;
            } else
                return IRV_DO_NOTHING;
        }
    }

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE)
            || (FcitxHotkeyIsHotKey(sym, state, FCITX_ENTER))) {
        size_t len = strlen(zhuyin->buf);
        if (len == 0)
            return IRV_TO_PROCESS;

        return FcitxCandidateWordChooseByIndex(FcitxInputStateGetCandidateList(input), 0);
    }

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_LIBZHUYIN_SHIFT_ENTER)) {
        if (zhuyin->buf[0] == 0)
            return IRV_TO_PROCESS;

        char* sentence = LibPinyinGetSentence(zhuyin);
        if (sentence) {
            auto offset = LibPinyinGetOffset(zhuyin);
            int hzlen = 0;
            if (fcitx_utf8_strlen(sentence) > offset) {
                hzlen = fcitx_utf8_get_nth_char(sentence, offset) - sentence;
            } else {
                hzlen = strlen(sentence);
            }

            auto start = LibPinyinGetPinyinOffset(zhuyin);
            int pylen = strlen(zhuyin->buf) - start;

            if (pylen < 0) {
                pylen = 0;
            }

            char* buf = (char*) fcitx_utils_malloc0((hzlen + pylen + 1) * sizeof(char));
            strncpy(buf, sentence, hzlen);
            if (pylen) {
                strcpy(&buf[hzlen], &zhuyin->buf[start]);
            }
            buf[hzlen + pylen] = '\0';
            FcitxInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), buf);
            g_free(sentence);
            free(buf);
        } else {
            FcitxInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance), zhuyin->buf);
        }

        return IRV_CLEAN;
    }

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE) || FcitxHotkeyIsHotKey(sym, state, FCITX_DELETE)) {
        if (strlen(zhuyin->buf) > 0) {
            int offset = LibPinyinGetOffset(zhuyin);
            if (offset != 0 && FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE)) {
                g_array_remove_index_fast(zhuyin->fixed_string, zhuyin->fixed_string->len - 1);
                zhuyin_clear_constraint(zhuyin->inst, LibPinyinGetOffset(zhuyin));
            } else {
                if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE)) {
                    if (zhuyin->cursor_pos > 0) {
                        zhuyin->cursor_pos -- ;
                    } else {
                        return IRV_DO_NOTHING;
                    }
                }
                size_t len = strlen(zhuyin->buf);
                if (zhuyin->cursor_pos == len)
                    return IRV_DO_NOTHING;
                memmove(zhuyin->buf + zhuyin->cursor_pos, zhuyin->buf + zhuyin->cursor_pos + 1, len - zhuyin->cursor_pos - 1);
                zhuyin->buf[strlen(zhuyin->buf) - 1] = 0;
                if (zhuyin->buf[0] == '\0')
                    return IRV_CLEAN;
                else
                    FcitxZhuyinParse(zhuyin, zhuyin->buf);
            }
            return IRV_DISPLAY_CANDWORDS;
        } else
            return IRV_TO_PROCESS;
    } else {
        if (strlen(zhuyin->buf) > 0) {
            if (FcitxHotkeyIsHotKey(sym, state, FCITX_LEFT)) {
                if (zhuyin->cursor_pos > 0) {
                    if (zhuyin->cursor_pos == LibPinyinGetPinyinOffset(zhuyin)) {
                        g_array_remove_index_fast(zhuyin->fixed_string, zhuyin->fixed_string->len - 1);
                        zhuyin_clear_constraint(zhuyin->inst, LibPinyinGetOffset(zhuyin));
                        return IRV_DISPLAY_CANDWORDS;
                    } else {
                        zhuyin->cursor_pos--;
                        return IRV_DISPLAY_CANDWORDS;
                    }
                }

                return IRV_DO_NOTHING;
            } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_RIGHT)) {
                size_t len = strlen(zhuyin->buf);
                if (zhuyin->cursor_pos < len) {
                    zhuyin->cursor_pos ++ ;
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_HOME)) {
                auto offset = LibPinyinGetPinyinOffset(zhuyin);
                if (zhuyin->cursor_pos != offset) {
                    zhuyin->cursor_pos = offset;
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            } else if (FcitxHotkeyIsHotKey(sym, state, FCITX_END)) {
                size_t len = strlen(zhuyin->buf);
                if (zhuyin->cursor_pos != len) {
                    zhuyin->cursor_pos = len ;
                    return IRV_DISPLAY_CANDWORDS;
                }
                return IRV_DO_NOTHING;
            }
        } else {
            return IRV_TO_PROCESS;
        }
    }
    return IRV_TO_PROCESS;
}

void FcitxZhuyinLoad(FcitxZhuyin* zhuyin)
{
    if (zhuyin->inst != NULL)
        return;

    FcitxZhuyinAddonInstance* zhuyinaddon = zhuyin->owner;

    if (zhuyin->owner->zhuyin_context == NULL) {
        char* user_path = FcitxZhuyinGetUserPath();
        char* syspath = FcitxZhuyinGetSysPath();
        zhuyinaddon->zhuyin_context = zhuyin_init(syspath, user_path);
        zhuyin_load_phrase_library(zhuyinaddon->zhuyin_context, TSI_DICTIONARY);
        zhuyin_load_phrase_library(zhuyinaddon->zhuyin_context, USER_DICTIONARY);
        free(user_path);
        free(syspath);
    }

    zhuyin->inst = zhuyin_alloc_instance(zhuyinaddon->zhuyin_context);

    FcitxZhuyinReconfigure(zhuyinaddon);
}

boolean FcitxZhuyinInit(void* arg)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*) arg;
    FcitxInstanceSetContext(zhuyin->owner->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    FcitxInstanceSetContext(zhuyin->owner->owner, CONTEXT_ALTERNATIVE_PREVPAGE_KEY, zhuyin->owner->config.hkPrevPage);
    FcitxInstanceSetContext(zhuyin->owner->owner, CONTEXT_ALTERNATIVE_NEXTPAGE_KEY, zhuyin->owner->config.hkNextPage);

    FcitxZhuyinLoad(zhuyin);

    return true;
}

void FcitxZhuyinUpdatePreedit(FcitxZhuyin* zhuyin, char* sentence)
{
    FcitxInstance* instance = zhuyin->owner->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);
    auto offset = LibPinyinGetOffset(zhuyin);

    auto pyoffset = LibPinyinGetPinyinOffset(zhuyin);
    if (pyoffset > zhuyin->cursor_pos)
        zhuyin->cursor_pos = pyoffset;

    size_t hzlen = 0;
    if (fcitx_utf8_strlen(sentence) > offset) {
        hzlen = fcitx_utf8_get_nth_char(sentence, offset) - sentence;
    } else {
        hzlen = strlen(sentence);
    }

    if (hzlen > 0) {
        char* buf = (char*) fcitx_utils_malloc0((hzlen + 1) * sizeof(char));
        strncpy(buf, sentence, hzlen);
        buf[hzlen] = 0;
        FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, "%s", buf);
        free(buf);
    }

    int charcurpos = hzlen;

    auto lastpos = pyoffset;
    auto curoffset = pyoffset;
    guint pinyinLen = 0;
    zhuyin_get_n_zhuyin(zhuyin->inst, &pinyinLen);
    for (auto i = offset; i < pinyinLen; i ++) {
        PinyinKey* pykey = NULL;
        PinyinKeyPos* pykeypos = NULL;
        zhuyin_get_zhuyin_key(zhuyin->inst, i, &pykey);
        zhuyin_get_zhuyin_key_rest(zhuyin->inst, i, &pykeypos);

        guint16 rawBegin = 0, rawEnd = 0;
        zhuyin_get_zhuyin_key_rest_positions(zhuyin->inst, pykeypos, &rawBegin, &rawEnd);
        if (lastpos > 0) {
            FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), " ");
            if (curoffset < zhuyin->cursor_pos)
                charcurpos ++;
            for (auto j = lastpos; j < rawBegin; j ++) {
                char temp[2] = {'\0', '\0'};
                temp[0] = zhuyin->buf[j];
                FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), temp);
                if (curoffset < zhuyin->cursor_pos) {
                    curoffset ++;
                    charcurpos ++;
                }
            }
        }
        lastpos = rawEnd;

        {
            guint16 pykeyposLen = 0;
            zhuyin_get_zhuyin_key_rest_length(zhuyin->inst, pykeypos, &pykeyposLen);
            gchar* pystring;
            zhuyin_get_bopomofo_string(zhuyin->inst, pykey, &pystring);
            FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_CODE, "%s", pystring);

            if (curoffset + pykeyposLen <= zhuyin->cursor_pos) {
                curoffset += pykeyposLen;
                charcurpos += strlen(pystring);
            } else {
                curoffset = zhuyin->cursor_pos;
                size_t len = fcitx_utf8_strlen(pystring);

                if (zhuyin->cursor_pos > len + curoffset)
                    charcurpos += strlen(pystring);
                else {
                    charcurpos += fcitx_utf8_get_nth_char(pystring, zhuyin->cursor_pos - curoffset) - pystring;
                }
            }
            g_free(pystring);
        }
    }

    auto buflen = strlen(zhuyin->buf);

    if (lastpos < buflen) {
        FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), " ");
        if (lastpos < zhuyin->cursor_pos)
            charcurpos ++;

        for (auto i = lastpos; i < buflen; i ++) {
            char temp[2] = {'\0', '\0'};
            temp[0] = zhuyin->buf[i];
            FcitxMessagesMessageConcatLast(FcitxInputStateGetPreedit(input), temp);
            if (lastpos < zhuyin->cursor_pos) {
                charcurpos ++;
                lastpos++;
            }
        }
    }
    FcitxInputStateSetCursorPos(input, charcurpos);
}

char* LibPinyinGetSentence(FcitxZhuyin* zhuyin)
{
    char* sentence = NULL;
    zhuyin_get_sentence(zhuyin->inst, &sentence);

    return sentence;
}

/**
 * @brief function DoInput has done everything for us.
 *
 * @param searchMode
 * @return INPUT_RETURN_VALUE
 **/
INPUT_RETURN_VALUE FcitxZhuyinGetCandWords(void* arg)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*)arg;
    FcitxInstance* instance = zhuyin->owner->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);
    FcitxGlobalConfig* config = FcitxInstanceGetGlobalConfig(zhuyin->owner->owner);
    FcitxZhuyinConfig* pyConfig = &zhuyin->owner->config;
    struct _FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    FcitxCandidateWordSetPageSize(candList, config->iMaxCandWord);
    FcitxUICloseInputWindow(instance);
    strcpy(FcitxInputStateGetRawInputBuffer(input), zhuyin->buf);
    FcitxInputStateSetRawInputBufferSize(input, strlen(zhuyin->buf));
    FcitxInputStateSetShowCursor(input, true);
    FcitxInputStateSetClientCursorPos(input, 0);

    FcitxKeyState state = zhuyin->owner->config.zhuyinLayout == FCITX_ZHUYIN_ETEN26 ? FcitxKeyState_None : candidateModifierMap[pyConfig->candidateModifiers];
    FcitxCandidateWordSetChooseAndModifier(candList, "1234567890", state);

    /* add punc */
    if (strlen(zhuyin->buf) == 1
        && LibPinyinCheckZhuyinKey((FcitxKeySym) zhuyin->buf[0], pyConfig->zhuyinLayout, pyConfig->useTone)
        && (zhuyin->buf[0] >= ' ' && zhuyin->buf[0] <= '\x7e') /* simple */
        && !(zhuyin->buf[0] >= 'a' && zhuyin->buf[0] <= 'z') /* not a-z */
        && !(zhuyin->buf[0] >= 'A' && zhuyin->buf[0] <= 'Z') /* not A-Z /*/
        && !(zhuyin->buf[0] >= '0' && zhuyin->buf[0] <= '9') /* not digit */
       ) {
        int c = zhuyin->buf[0];
        char* result = FcitxPuncGetPunc(instance, &c);
        if (result) {
            FcitxCandidateWord candWord;
            FcitxZhuyinCandWord* pyCand = (FcitxZhuyinCandWord*) fcitx_utils_malloc0(sizeof(FcitxZhuyinCandWord));
            pyCand->ispunc = true;
            candWord.callback = FcitxZhuyinGetCandWord;
            candWord.extraType = MSG_OTHER;
            candWord.owner = zhuyin;
            candWord.priv = pyCand;
            candWord.strExtra = NULL;
            candWord.strWord = strdup(result);
            candWord.wordType = MSG_OTHER;

            FcitxCandidateWordAppend(FcitxInputStateGetCandidateList(input), &candWord);
        }
    }
    char* sentence = NULL;

    zhuyin_guess_sentence(zhuyin->inst);
    sentence = LibPinyinGetSentence(zhuyin);
    if (sentence) {
        FcitxZhuyinUpdatePreedit(zhuyin, sentence);

        FcitxMessagesAddMessageAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT, "%s", sentence);

        g_free(sentence);
    } else {
        FcitxInputStateSetCursorPos(input, zhuyin->cursor_pos);
        FcitxMessagesAddMessageAtLast(FcitxInputStateGetClientPreedit(input), MSG_INPUT, "%s", zhuyin->buf);
        FcitxMessagesAddMessageAtLast(FcitxInputStateGetPreedit(input), MSG_INPUT, "%s", zhuyin->buf);
    }

    zhuyin_guess_candidates(zhuyin->inst, LibPinyinGetOffset(zhuyin));
    guint candidateLen = 0;
    zhuyin_get_n_candidate(zhuyin->inst, &candidateLen);
    for (guint i = 0 ; i < candidateLen; i ++) {
        lookup_candidate_t* token = NULL;
        zhuyin_get_candidate(zhuyin->inst, i, &token);
        FcitxCandidateWord candWord;
        FcitxZhuyinCandWord* pyCand = (FcitxZhuyinCandWord*) fcitx_utils_malloc0(sizeof(FcitxZhuyinCandWord));
        pyCand->ispunc = false;
        pyCand->idx = i;
        candWord.callback = FcitxZhuyinGetCandWord;
        candWord.extraType = MSG_OTHER;
        candWord.owner = zhuyin;
        candWord.priv = pyCand;
        candWord.strExtra = NULL;

        const gchar* phrase_string = NULL;
        zhuyin_get_candidate_string(zhuyin->inst, token, &phrase_string);
        candWord.strWord = strdup(phrase_string);
        candWord.wordType = MSG_OTHER;

        FcitxCandidateWordAppend(FcitxInputStateGetCandidateList(input), &candWord);
    }

    return IRV_DISPLAY_CANDWORDS;
}

/**
 * @brief get the candidate word by index
 *
 * @param iIndex index of candidate word
 * @return the string of canidate word
 **/
INPUT_RETURN_VALUE FcitxZhuyinGetCandWord(void* arg, FcitxCandidateWord* candWord)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*)arg;
    FcitxZhuyinCandWord* pyCand = (FcitxZhuyinCandWord*) candWord->priv;
    FcitxInstance* instance = zhuyin->owner->owner;
    FcitxInputState* input = FcitxInstanceGetInputState(instance);

    if (pyCand->ispunc) {
        strcpy(FcitxInputStateGetOutputString(input), candWord->strWord);
        return IRV_COMMIT_STRING;
    } else {
        guint candidateLen = 0;
        zhuyin_get_n_candidate(zhuyin->inst, &candidateLen);
        if (candidateLen <= pyCand->idx)
            return IRV_TO_PROCESS;
        lookup_candidate_t* cand = NULL;
        zhuyin_get_candidate(zhuyin->inst, pyCand->idx, &cand);
        zhuyin_choose_candidate(zhuyin->inst, LibPinyinGetOffset(zhuyin), cand);

        FcitxZhuyinFixed f;
        const gchar* phrase_string = NULL;
        zhuyin_get_candidate_string(zhuyin->inst, cand, &phrase_string);
        f.len = fcitx_utf8_strlen(phrase_string);
        g_array_append_val(zhuyin->fixed_string, f);

        auto offset = LibPinyinGetOffset(zhuyin);

        guint pykeysLen = 0;
        zhuyin_get_n_zhuyin(zhuyin->inst, &pykeysLen);
        if (offset >= pykeysLen) {
            char* sentence = NULL;
            zhuyin_guess_sentence(zhuyin->inst);
            sentence = LibPinyinGetSentence(zhuyin);
            if (sentence) {
                strcpy(FcitxInputStateGetOutputString(input), sentence);
                g_free(sentence);
                zhuyin_train(zhuyin->inst);
            } else
                strcpy(FcitxInputStateGetOutputString(input), "");

            return IRV_COMMIT_STRING;
        }

        auto pyoffset = LibPinyinGetPinyinOffset(zhuyin);
        if (pyoffset > zhuyin->cursor_pos)
            zhuyin->cursor_pos = pyoffset;

        return IRV_DISPLAY_CANDWORDS;
    }
    return IRV_TO_PROCESS;
}

FcitxZhuyin* FcitxZhuyinNew(FcitxZhuyinAddonInstance* zhuyinaddon)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*) fcitx_utils_malloc0(sizeof(FcitxZhuyin));
    zhuyin->inst = NULL;
    zhuyin->fixed_string = g_array_new(FALSE, FALSE, sizeof(FcitxZhuyinFixed));
    zhuyin->owner = zhuyinaddon;
    return zhuyin;
}

void FcitxZhuyinDelete(FcitxZhuyin* zhuyin)
{
    if (zhuyin->inst)
        zhuyin_free_instance(zhuyin->inst);
    g_array_free(zhuyin->fixed_string, TRUE);
}

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
void* FcitxZhuyinCreate(FcitxInstance* instance)
{
    FcitxZhuyinAddonInstance* zhuyinaddon = (FcitxZhuyinAddonInstance*) fcitx_utils_malloc0(sizeof(FcitxZhuyinAddonInstance));
    bindtextdomain("fcitx-zhuyin", LOCALEDIR);
    bind_textdomain_codeset("fcitx-zhuyin", "UTF-8");
    zhuyinaddon->owner = instance;

    if (!FcitxZhuyinConfigLoadConfig(&zhuyinaddon->config)) {
        free(zhuyinaddon);
        return NULL;
    }

    zhuyinaddon->zhuyin = FcitxZhuyinNew(zhuyinaddon);

    FcitxZhuyinReconfigure(zhuyinaddon);

    FcitxInstanceRegisterIM(instance,
                            zhuyinaddon->zhuyin,
                            "zhuyin",
                            _("Bopomofo"),
                            "zhuyin",
                            FcitxZhuyinInit,
                            FcitxZhuyinReset,
                            FcitxZhuyinDoInput,
                            FcitxZhuyinGetCandWords,
                            NULL,
                            FcitxZhuyinSave,
                            NULL,
                            NULL,
                            5,
                            "zh_TW"
                           );

    return zhuyinaddon;
}

/**
 * @brief Destroy the input method while unload it.
 *
 * @return int
 **/
void FcitxZhuyinDestroy(void* arg)
{
    FcitxZhuyinAddonInstance* zhuyin = (FcitxZhuyinAddonInstance*) arg;
    FcitxZhuyinDelete(zhuyin->zhuyin);
    if (zhuyin->zhuyin_context) {
        zhuyin_fini(zhuyin->zhuyin_context);
    }

    free(zhuyin);
}

void FcitxZhuyinReconfigure(FcitxZhuyinAddonInstance* zhuyinaddon)
{
    FcitxZhuyinConfig* config = &zhuyinaddon->config;

    zhuyin_option_t settings = 0;
    settings |= DYNAMIC_ADJUST;
    for (int i = 0; i <= FCITX_AMB_LAST; i ++) {
        if (config->amb[i])
            settings |= FcitxZhuyinTransAmbiguity((FCITX_AMBIGUITY) i);
    }

    if (config->chewingIncomplete) {
        settings |= CHEWING_INCOMPLETE;
    }

    if (config->useTone) {
        settings |= USE_TONE;
    }
    settings |= IS_PINYIN;
    settings |= IS_BOPOMOFO;
    if (zhuyinaddon->zhuyin_context) {
        zhuyin_set_options(zhuyinaddon->zhuyin_context, settings);
    }
}

void FcitxZhuyinReloadConfig(void* arg)
{
    FcitxZhuyinAddonInstance* zhuyinaddon = (FcitxZhuyinAddonInstance*) arg;
    FcitxZhuyinConfigLoadConfig(&zhuyinaddon->config);
    FcitxZhuyinReconfigure(zhuyinaddon);
}

void FcitxZhuyinSave(void* arg)
{
    FcitxZhuyin* zhuyin = (FcitxZhuyin*) arg;
    if (zhuyin->owner->zhuyin_context) {
        zhuyin_save(zhuyin->owner->zhuyin_context);
    }
}

void FcitxZhuyinClearData(FcitxZhuyin* zhuyin, int type)
{
    FcitxZhuyinAddonInstance* zhuyinaddon = zhuyin->owner;
    FcitxZhuyinReset(zhuyin);

    zhuyin_context_t* context = zhuyinaddon->zhuyin_context;
    if (!context) {
        return;
    }

    switch (type) {
    case 0:
        zhuyin_mask_out(context, PHRASE_INDEX_LIBRARY_MASK, PHRASE_INDEX_MAKE_TOKEN(15, null_token));
        break;
    case 1:
        zhuyin_mask_out(context, PHRASE_INDEX_LIBRARY_MASK, PHRASE_INDEX_MAKE_TOKEN(14, null_token));
        break;
    case 2:
        zhuyin_mask_out(context, 0, 0);
        break;
    }

    zhuyin_train(zhuyin->inst);
    zhuyin_save(context);
}

void FcitxZhuyinImport(FcitxZhuyin* zhuyin)
{
    FcitxZhuyinAddonInstance* zhuyinaddon = zhuyin->owner;
    FcitxZhuyinReset(zhuyin);
    FcitxZhuyinLoad(zhuyin);

    zhuyin_context_t* context = zhuyinaddon->zhuyin_context;
    if (!context)
        return;

    const char* path = "zhuyin/importdict_zhuyin";

    zhuyin_mask_out(context, PHRASE_INDEX_LIBRARY_MASK, PHRASE_INDEX_MAKE_TOKEN(14, null_token));

    import_iterator_t* iter = zhuyin_begin_add_phrases(context, 14);
    if (iter == NULL) {
        return;
    }

    FcitxStringHashSet* sset = FcitxXDGGetFiles(path, NULL, ".txt");
    HASH_FOREACH(str, sset, FcitxStringHashSet) {
        /* user phrase library should be already loaded here. */
        FILE* dictfile = FcitxXDGGetFileWithPrefix(path, str->name, "r", NULL);
        if (NULL == dictfile) {
            continue;
        }

        char* linebuf = NULL;
        size_t size = 0;
        while (getline(&linebuf, &size, dictfile) != -1) {
            if (0 == strlen(linebuf))
                continue;

            if ('\n' == linebuf[strlen(linebuf) - 1]) {
                linebuf[strlen(linebuf) - 1] = '\0';
            }

            gchar** items = g_strsplit_set(linebuf, " \t", 3);
            guint len = g_strv_length(items);
            do {

                gchar* phrase = NULL, * pinyin = NULL;
                gint count = -1;
                if (2 == len || 3 == len) {
                    phrase = items[0];
                    pinyin = items[1];
                    if (3 == len)
                        count = atoi(items[2]);
                } else {
                    break;
                }

                if (!fcitx_utf8_check_string(phrase)) {
                    continue;
                }

                zhuyin_iterator_add_phrase(iter, phrase, pinyin, count);
            } while (0);

            g_strfreev(items);
        }
        free(linebuf);
        fclose(dictfile);
    }

    zhuyin_end_add_phrases(iter);

    if (zhuyin->inst) {
        zhuyin_train(zhuyin->inst);
    }
    zhuyin_save(context);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
