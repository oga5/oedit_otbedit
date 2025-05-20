/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
#ifndef _DISP_COLOR_DATA_H_INCLUDE
#define _DISP_COLOR_DATA_H_INCLUDE

class CEditData;
class CEditRowData;

#define DISP_COLOR_DATA_SELECTED	(0x01 << 0)		// 選択範囲
#define DISP_COLOR_DATA_SEARCHED	(0x01 << 1)		// 検索結果
#define DISP_COLOR_DATA_CLICKABLE	(0x01 << 2)		// クリッカブル
#define DISP_COLOR_DATA_BOLD		(0x01 << 3)		// BOLD表示

#define MARK_TAB			1
#define MARK_2BYTE_SPACE	2
#define MARK_END_OF_LINE	3
#define MARK_SPACE			4

struct _st_disp_color_data {
	COLORREF	color;
	unsigned char mark;		// 記号
	unsigned char disp_flg;
};

class CDispColorData
{
public:
	CDispColorData();
	~CDispColorData();

	void InitDispColorData(int row_len);
	void SetRowData(CEditRowData *row_data, int row);
	BOOL CheckReuse(CEditRowData *row_data, int row);
	void ClearData();

	void ClearSearched();
	void ClearSelected();

	void SetColor(int pos, COLORREF color) { m_disp_color_data[pos].color = color; }
	void SetMark(int pos, int mark) { m_disp_color_data[pos].mark = mark; }
	void SetSelected(int pos) { SetFlg(pos, DISP_COLOR_DATA_SELECTED); }
	void SetSearched(int pos) { SetFlg(pos, DISP_COLOR_DATA_SEARCHED); }
	void SetClickable(int pos) { SetFlg(pos, DISP_COLOR_DATA_CLICKABLE); }
	void SetBold(int pos) { SetFlg(pos, DISP_COLOR_DATA_BOLD); }
	void UnsetBold(int pos) { UnsetFlg(pos, DISP_COLOR_DATA_BOLD); }

	COLORREF GetColor(int pos) { return m_disp_color_data[pos].color; }
	int GetMark(int pos) { return m_disp_color_data[pos].mark; }
	BOOL GetSelected(int pos) { return GetFlg(pos, DISP_COLOR_DATA_SELECTED); }
	BOOL GetSearched(int pos) { return GetFlg(pos, DISP_COLOR_DATA_SEARCHED); }
	BOOL GetClickable(int pos) { return GetFlg(pos, DISP_COLOR_DATA_CLICKABLE); }
	BOOL GetBold(int pos) { return GetFlg(pos, DISP_COLOR_DATA_BOLD); }

	int DispColorDataCmp(int pos1, int pos2);

private:
	void InitData();
	void InitMemData();
	void FreeData();

	void SetFlg(int pos, int flg) { m_disp_color_data[pos].disp_flg |= flg; }
	void UnsetFlg(int pos, int flg) { m_disp_color_data[pos].disp_flg &= (~flg); }
	BOOL GetFlg(int pos, int flg) { return (m_disp_color_data[pos].disp_flg & flg); }

	int			m_disp_color_data_cnt;

	struct _st_disp_color_data *m_disp_color_data;

	CEditRowData	*m_row_data;
	int			m_row;
	int			m_row_len;
};

#pragma intrinsic(memcmp)
__inline int CDispColorData::DispColorDataCmp(int pos1, int pos2)
{
	if(m_disp_color_data[pos1].mark != 0) return 1;
	if(memcmp(&(m_disp_color_data[pos1]), &(m_disp_color_data[pos2]),
		sizeof(struct _st_disp_color_data)) != 0) return 1;
	return 0;
}
#pragma function(memcmp)

class CDispColorDataPool
{
public:
	CDispColorDataPool();
	~CDispColorDataPool();

	void SetPoolSize(int size);

	void ClearAllData();
	void ClearAllSearched();

	void SetEditData(CEditData *edit_data) { m_edit_data = edit_data; }
	CDispColorData *GetDispColorData(int row);
	CDispColorData *GetNewDispColorData(int row);

private:
	int GetIdx(int row) { return row % m_pool_size; }

	CEditData			*m_edit_data;
	CDispColorData		*m_disp_color_data_list;
	int					m_pool_size;
};

#endif	_DISP_COLOR_DATA_H_INCLUDE
