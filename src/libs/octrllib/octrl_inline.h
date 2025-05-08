/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef __OCTRL_INLINE_H_INCLUDED__
#define __OCTRL_INLINE_H_INCLUDED__

__inline static int inline_pt_cmp(const POINT *pt1, const POINT *pt2)
{
	if(pt1->y != pt2->y) return pt1->y - pt2->y;
	return pt1->x - pt2->x;
}

#endif __OCTRL_INLINE_H_INCLUDED__