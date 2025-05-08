/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#include "stdafx.h"
#include "colorutil.h"

void RGBtoHSV(COLORREF color, HSV *hsv)
{
	int r, g, b;
	GetRGB(color, &r, &g, &b);

	int max_c = max(r, max(g, b));
	int min_c = min(r, min(g, b));

	if(max_c - min_c > 0) {
		if(g == max_c) {
			hsv->h = ((double)((double)b - r) / ((double)max_c - min_c)) * 60 + 120;
		}
		else if(b == max_c) {
			hsv->h = ((double)((double)r - g) / ((double)max_c - min_c)) * 60 + 120;
		}
		else {
			hsv->h = ((double)((double)g - b) / ((double)max_c - min_c)) * 60;
		}
		if(hsv->h < 0) hsv->h += 360;

		hsv->s = ((double)((double)max_c - min_c) / max_c) * 0xff;
	} else {
		hsv->h = 0;
		hsv->s = 0;
	}

	hsv->v = max_c;
}

COLORREF HSVtoRGB(const HSV *hsv)
{
	double r, g, b;
	double num1 = (hsv->v * (0xff - hsv->s) / 0xff); // Å¬’l
	double num2 = (hsv->v * (1 - hsv->s / 0xff * ((int)hsv->h % 60) / 60)); // 2”Ô–Ú‚ÌF(³‚Ì•ûŒü‚Ìê‡)
	double num3 = (hsv->v * (1 - hsv->s / 0xff * ((double)1 - ((int)hsv->h % 60) / 60))); // 2”Ô–Ú‚ÌF(•‰‚Ì•ûŒü‚Ìê‡)

	switch((int)(hsv->h / 60)) {
	case 0:
		r = (int)hsv->v;
		g = (int)num3;
		b = (int)num1;
		break;
	case 1:
		r = (int)num2;
		g = (int)hsv->v;
		b = (int)num1;
		break;
	case 2:
		r = (int)num1;
		g = (int)hsv->v;
		b = (int)num3;
		break;
	case 3:
		r = (int)num1;
		g = (int)num2;
		b = (int)hsv->v;
		break;
	case 4:
		r = (int)num3;
		g = (int)num1;
		b = (int)hsv->v;
		break;
	default:
		r = (int)hsv->v;
		g = (int)num1;
		b = (int)num2;
		break;
	}
	return RGB(r, g, b);
}

COLORREF make_reverse_color(COLORREF color)
{
	HSV		hsv;
	RGBtoHSV(color, &hsv);

	hsv.v = 0xff - hsv.v;

	hsv.h -= 180;
	if(hsv.h < 0) hsv.h += 360;
	
	return HSVtoRGB(&hsv);
}

COLORREF make_blend_color(COLORREF color1, COLORREF color2)
{
	int r1, g1, b1;
	int r2, g2, b2;

	GetRGB(color1, &r1, &g1, &b1);
	GetRGB(color2, &r2, &g2, &b2);

	return RGB((r1 + r2) / 2, (g1 + g2) / 2, (b1, b2) / 2);
}

void SaveColorDlgCutomColors(CColorDialog *dlg)
{
	COLORREF* ccolor = dlg->GetSavedCustomColors();
	CString entry_name;

	for (int i = 0; i < 16; i++) {
		entry_name.Format(_T("CUSTOM_COLOR_%.2d"), i);
		AfxGetApp()->WriteProfileInt(_T("COLOR_DLG"), entry_name, ccolor[i]);
	}
}

void LoadColorDlgCutomColors(CColorDialog *dlg)
{
	COLORREF* ccolor = dlg->m_cc.lpCustColors;
	CString entry_name;

	for (int i = 0; i < 16; i++) {
		entry_name.Format(_T("CUSTOM_COLOR_%.2d"), i);
		ccolor[i] = AfxGetApp()->GetProfileInt(_T("COLOR_DLG"), entry_name, RGB(0xff, 0xff, 0xff));
	}
}
