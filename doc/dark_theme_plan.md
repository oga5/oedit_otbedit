# ダークテーマ対応 実装計画

## 概要

oedit / otbedit にアプリ側で制御できるダークテーマ機能を追加する。  
OS（Windows）のテーマ設定には依存せず、アプリの設定画面からダーク/ライトを切り替えられるようにする。

フレームワークは MFC であるため、ダイアログ・フレームとも対応可能。ただし対象ごとに対応難度が異なる（[制約](#技術的な制約と注意点) を参照）。

---

## 現状の確認

| 項目 | 状況 |
|------|------|
| テキストエディタ部分の色 | `EDIT_CTRL_COLOR_CNT` 種類すべてカスタマイズ済み |
| カラープリセット | `PresetColor.h` に `default` / `solarized-light` / `solarized-dark` が定義済み |
| プリセット選択UI | `EditorOptionPage` に実装済み |
| ダークテーマ用フラグ | **未実装** |
| ダイアログ・フレームのダーク化 | **未実装** |

---

## 実装方針

### 1. ダークテーマフラグの追加

- `src/tools/oedit/oedit/global.h` の `_st_option` に `BOOL dark_theme` フィールドを追加
- `src/tools/common_editor/oeditapp2.cpp` の `LoadOption` / `SaveOption` に INI 読み書きを追加
  - INI キー例：`[APPLICATION]` セクション `DARK_THEME`

### 2. 設定 UI への「ダークテーマ」チェックボックス追加

- `SetupPage2`（oedit）または `SetupPage`（共通）にチェックボックスを追加
- 変更時に `UpdateAllDocViews` でアプリ全体に再描画通知を送る

### 3. タイトルバーのダーク化（フレーム）

- `MainFrm.cpp` の `OnCreate` / `OnActivateApp` で以下の Windows API を呼び出す：

  ```cpp
  BOOL useDark = TRUE;
  DwmSetWindowAttribute(
      hwnd,
      DWMWA_USE_IMMERSIVE_DARK_MODE,  // = 20
      &useDark,
      sizeof(useDark)
  );
  ```

- OS のダークモード設定に依存せずアプリ単位で指定可能（Windows 10 Build 18985 / 20H1 以降）
- ダークテーマフラグが切り替わった際にも再呼び出しして即時反映

### 4. ダイアログ・プロパティページの背景ダーク化

- `octrllib` に共通ユーティリティ `CDarkModeUtil` クラスを作成し、各ダイアログから利用
  - ダーク背景ブラシ（例：`RGB(0x1e, 0x1e, 0x1e)`）の管理
  - テキスト色（例：`RGB(0xd4, 0xd4, 0xd4)`）の管理
- MFC の `WM_CTLCOLOR` 系メッセージをオーバーライドしてブラシ・文字色を返す：

  | メッセージ | 対象コントロール |
  |-----------|----------------|
  | `OnCtlColor` (CTLCOLOR_DLG)    | ダイアログ背景 |
  | `OnCtlColor` (CTLCOLOR_STATIC) | スタティックテキスト |
  | `OnCtlColor` (CTLCOLOR_EDIT)   | エディットボックス |
  | `OnCtlColor` (CTLCOLOR_LISTBOX)| リストボックス |

- 対象ダイアログ：
  - `COptionSheet` / 各プロパティページ
  - `CSearchDlg` / `CReplaceDlg` / `CFilterDlg`
  - `CNameInputDlg` / `CLineJumpDlg`
  - `CFileBookMarkDlg`
  - `CDataEditDlg` / `CShowCLobDlg` 等

### 5. ツールバー・ステータスバーのダーク化

- `SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL)` を適用
- ステータスバー・ツールバーの背景色を `WM_ERASEBKGND` でカスタム描画

### 6. エディタ色プリセットとの連携

- ダークテーマ ON 時のデフォルト色として `solarized-dark`（または新規 `dark` プリセット）を自動適用するオプションを追加
- エディタ色は手動設定を維持できるよう「テーマ切替時のみ自動適用」フラグを別途設ける

---

## 技術的な制約と注意点

| 対象 | 対応可否 | 備考 |
|------|----------|------|
| タイトルバー | ✅ 可能 | `DwmSetWindowAttribute` 使用、Windows 10 Build 18985 以降 |
| ダイアログ背景 | ✅ 可能 | `WM_CTLCOLOR` (OnCtlColor) で制御 |
| スタティックテキスト | ✅ 可能 | `OnCtlColor` で文字色・背景色を指定 |
| エディットコントロール | ✅ 可能 | `CTLCOLOR_EDIT` で制御 |
| リストボックス | ✅ 可能 | `CTLCOLOR_LISTBOX` で制御 |
| ツールバー / ステータスバー | ✅ 可能 | `SetWindowTheme` またはカスタムドロー |
| ボタン（プッシュボタン） | ⚠️ 部分対応 | オーナードロー化が必要（標準ボタンは OS 依存） |
| メニュー | ⚠️ 制限あり | MFC でのメニュー完全ダーク化は非常に複雑、要検討 |
| コモンダイアログ（ファイルを開く等） | ❌ 困難 | OS 管轄のため OS のテーマ設定が必要 |

---

## 実装ステップ（推奨順序）

1. `g_option` にダークテーマフラグ追加 → INI 読み書き対応
2. `CDarkModeUtil` ユーティリティを `octrllib` に作成（ブラシ管理・色定義）
3. `MainFrm` でタイトルバーダーク化（`DwmSetWindowAttribute`）
4. `COptionSheet` / プロパティページに `OnCtlColor` を追加（背景・テキスト色制御）
5. 各ダイアログに `OnCtlColor` を追加（SearchDlg / ReplaceDlg / NameInputDlg 等）
6. SetupPage に設定 UI（チェックボックス）を追加
7. ツールバー・ステータスバーに `SetWindowTheme` を適用
8. エディタ色プリセットとの連携（ダークテーマ ON 時のデフォルト色自動適用）

---

## OS テーマ設定が不要な理由

- タイトルバーの `DwmSetWindowAttribute(DWMWA_USE_IMMERSIVE_DARK_MODE)` は、Windows 10 Build 18985 以降においてアプリ単位で指定でき、OS 全体のダークモード設定に依存しない。
- ダイアログ背景・コントロール色はすべてアプリ側の `WM_CTLCOLOR` で制御するため、OS のテーマ設定は不要。
- 結果として、OS が「ライトテーマ」でもアプリのみをダーク表示にすることが可能。
