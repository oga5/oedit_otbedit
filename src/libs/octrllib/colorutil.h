/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef COLOR_UTIL_H
#define COLOR_UTIL_H

#define INVALID_COLOR	0xffffffff

static void GetRGB(COLORREF color, int *R, int *G, int *B)
{
	*R = (color & 0x000000FF);
	*G = (color & 0x0000FF00) >> 8;
	*B = (color & 0x00FF0000) >> 16;
}

static COLORREF make_yellow_color(COLORREF color, double ratio)
{
	int R, G, B;
	GetRGB(color, &R, &G, &B);
	return RGB(min(R, 0xff), min(G, 0xff), min(B * ratio, 0xff));
}

static COLORREF make_dark_color(COLORREF color, double ratio)
{
	int R, G, B;
	GetRGB(color, &R, &G, &B);
	return RGB(min(R * ratio, 0xff), min(G * ratio, 0xff), min(B * ratio, 0xff));
}

static COLORREF make_highlight_color(COLORREF color, double ratio)
{
	int R, G, B;
	GetRGB(color, &R, &G, &B);

	if(R == G && R == B) {
		R = (int)min(R * ratio, 0xff);
		G = (int)min(G * ratio, 0xff);
		B = (int)min(B * ratio, 0xff);
	} else {
		if(R > G && R > B) { // max is R
			G = (int)min(G * ratio, 0xff);
			B = (int)min(B * ratio, 0xff);
		} else if(G > R && G > B) { // max is G
			R = (int)min(R * ratio, 0xff);
			B = (int)min(B * ratio, 0xff);
		} else { // max is B
			R = (int)min(R * ratio, 0xff);
			G = (int)min(G * ratio, 0xff);
		}
	}

	return RGB(R, G, B);
}

#define MIX_COLOR_CALC(C1, C2, R) ((double)C1 + (double)((double)C2 - (double)C1) * R)

static COLORREF mix_color(COLORREF color1, COLORREF color2, double ratio)
{
	int R1, G1, B1, R2, G2, B2;
	GetRGB(color1, &R1, &G1, &B1);
	GetRGB(color2, &R2, &G2, &B2);

	double SR = MIX_COLOR_CALC(R1, R2, ratio);
	double SG = MIX_COLOR_CALC(G1, G2, ratio);
	double SB = MIX_COLOR_CALC(B1, B2, ratio);
	
	return RGB(min(max(SR, 0), 0xff), 
			   min(max(SG, 0), 0xff),
			   min(max(SB, 0), 0xff));
}

static COLORREF make_xor_color(COLORREF color, COLORREF bk_color)
{
	int R, G, B;
	int RBK, GBK, BBK;
	GetRGB(color, &R, &G, &B);
	GetRGB(bk_color, &RBK, &GBK, &BBK);

	return RGB((R ^ RBK),  (G ^ GBK), (B ^ BBK));
}

typedef struct hsv_st {
	double		h;
	double		s;
	double		v;
} HSV;

COLORREF make_reverse_color(COLORREF color);
void RGBtoHSV(COLORREF color, HSV *hsv);
COLORREF HSVtoRGB(const HSV *hsv);

COLORREF make_blend_color(COLORREF color1, COLORREF color2);


void SaveColorDlgCutomColors(CColorDialog *dlg);
void LoadColorDlgCutomColors(CColorDialog *dlg);

#endif COLOR_UTIL_H
