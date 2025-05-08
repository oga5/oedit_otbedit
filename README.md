# oedit / otbedit

## 概要

このリポジトリには、2つのテキストエディタプロジェクトが含まれています：

1. **oedit** - シンプルなテキストエディタ
   - ソースコード: `src\tools\oedit`

2. **otbedit** - タブインタフェースを搭載したテキストエディタ
   - ソースコード: `src\tools\otbedit`

## ライセンス

このプロジェクトはBSDライセンスの下で提供されています。
詳細は `LICENSE_BSD.txt` をご覧ください。

注意: 一部のファイルはBSD以外のライセンスが適用されている場合があります。それらのファイルについては、各ファイルの先頭にライセンス情報が記載されています。

## ビルド方法

### 必要環境

- Visual Studio 2022

### ビルド手順

#### oeditのビルド

1. Visual Studioで `src\tools\oedit\oedit\oedit.sln` を開きます
2. ビルドを実行します

#### otbeditのビルド

1. Visual Studioで `src\tools\otbedit\otedit\otbedit.sln` を開きます
2. ビルドを実行します

#### ogrepのビルド（検索ツール）

1. Visual Studioで `src\tools\oedit\ogrep\ogrep.sln` を開きます
2. ビルドを実行します

ビルドが成功すると、リポジトリのルートに以下のフォルダが作成されます：
- `exe` - 実行ファイルが格納されます
- `lib` - ライブラリファイルが格納されます
- `obj` - 中間オブジェクトファイルが格納されます

実行ファイルは `exe` フォルダ内の対応するビルド構成とプラットフォームのサブディレクトリに出力されます：
- `exe\Debug\Win32\oedit\`
- `exe\Debug\x64\oedit\`
- `exe\Release\Win32\oedit\`
- `exe\Release\x64\oedit\`
- `exe\Debug\Win32\otbedit\`
- `exe\Debug\x64\otbedit\`
- `exe\Release\Win32\otbedit\`
- `exe\Release\x64\otbedit\`