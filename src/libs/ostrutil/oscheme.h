/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef OSCHEME_H_INCLUDED
#define OSCHEME_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

struct _st_scheme;
typedef struct _st_scheme oscheme;

struct _st_cell;
typedef struct _st_cell ocell;
typedef ocell *pcell;

typedef pcell (*oscheme_callback_func)(oscheme *sc, pcell *argv, int argc);
typedef void (*oscheme_portout_func)(oscheme *sc, const TCHAR *str);

oscheme *init_oscheme();
void clear_oscheme(oscheme *sc);

int oscheme_load_file(oscheme *sc, const TCHAR *file_name);
pcell oscheme_eval_str(oscheme *sc, const TCHAR *str);
pcell oscheme_apply(oscheme *sc, pcell x);
int oscheme_bound_global(oscheme *sc, pcell x);
int oscheme_unbound_global(oscheme *sc, pcell x);
TCHAR *oscheme_tostring(oscheme *sc, pcell lst);

void oscheme_error(oscheme *sc, const TCHAR *msg);

int oscheme_regist_callback_func(oscheme *sc, TCHAR *name,
	oscheme_callback_func func, int min_argc, int max_argc, TCHAR *type_chk_str);

pcell oscheme_true(oscheme *sc);
pcell oscheme_false(oscheme *sc);
pcell oscheme_nil(oscheme *sc);

int oscheme_is_false(oscheme *sc, pcell p);
TCHAR *oscheme_get_strvalue(oscheme *sc, pcell x);
INT_PTR oscheme_get_ivalue(oscheme *sc, pcell x);

int oscheme_get_int_value(oscheme *sc, pcell x);
long oscheme_get_long_value(oscheme *sc, pcell x);

pcell oscheme_mk_number(oscheme *sc, int n);
pcell oscheme_mk_string(oscheme *sc, const TCHAR *str);
pcell oscheme_mk_char(oscheme *sc, unsigned int ch);
pcell oscheme_mk_eof(oscheme *sc);
pcell oscheme_cons_cell(oscheme *sc, pcell a, pcell b);

TCHAR *oscheme_get_err_msg(oscheme *sc);
void oscheme_set_out_func(oscheme *sc, oscheme_portout_func func);
void oscheme_set_err_func(oscheme *sc, oscheme_portout_func func);

void oscheme_set_cancel_key(oscheme *sc, int fvirt, int keycode);
int oscheme_canceled(oscheme *sc);

void oscheme_set_dont_gc_flg(oscheme *sc);
void oscheme_clear_dont_gc_flg(oscheme *sc);

void oscheme_set_user_data(oscheme *sc, void *ptr);
void *oscheme_get_user_data(oscheme *sc);

#ifdef  __cplusplus
}
#endif

#endif /* OSCHEME_H_INCLUDED */



