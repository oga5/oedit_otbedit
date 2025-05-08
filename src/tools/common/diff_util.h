#ifndef __DIFF_UTIL_H_INCLUDED__
#define __DIFF_UTIL_H_INCLUDED__

#include "GridData.h"

typedef std::vector<int> int_v;

enum diff_result_status {
	DIFF_INSERT,
	DIFF_DELETE,
	DIFF_COMMON,
};

struct _diff_result_st {
	enum diff_result_status cmd;
	int		row1;
	int		row2;
};
typedef std::vector<struct _diff_result_st> diff_result_v;

diff_result_v *grid_diff(CGridData *a, CGridData *b, int *diff_cnt);

#endif /* __DIFF_UTIL_H_INCLUDED__ */
