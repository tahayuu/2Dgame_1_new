2Dgame_GPT フォルダ整理版

このZIPはアップロードされた「2Dgame_GPT (2)(2).zip」をもとに、
Visual Studioの仮想フィルターを参考に物理フォルダへ配置し直したものです。

【配置先】
  Player        6 ファイル
  Stage        15 ファイル
  StageInit    15 ファイル
  Editor        5 ファイル
  Enemy         4 ファイル
  Item          4 ファイル
  Ojisan       11 ファイル
  Other        17 ファイル
  Texture       2 ファイル
  Archive       1 ファイル
  Scripts       2 ファイル
  Docs          4 ファイル
  (直下)       main.cpp / プロジェクト設定など

【変更した内容】
1. ソース(.cpp/.h)を機能ごとの物理フォルダへ移動しました。
2. 2Dgame_1.vcxproj と .vcxproj.filters のファイル参照パスを修正しました。
3. 全ビルド構成の追加インクルードディレクトリに上記ソースフォルダを登録しました。
4. 2Dgame_1.slnx を追加しました（既存のソリューションがなくても開きやすくするため）。
5. メモとスクリプトは Docs / Scripts にまとめました。

【変更していない内容】
- .cpp/.hの中身とゲームロジック、関数名、ファイル名
- 画像や音声に関する既存のプロジェクト参照
- 設定に含まれるコンパイラ・プラットフォームツールセット
- 実行時に参照される ojisan_lines.text の配置

【開き方】
1. 元のプロジェクトを必ずバックアップしてから別フォルダに解凍してください。
2. 2Dgame_GPT/2Dgame_1.slnx をVisual Studioで開きます。
3. x64 / Debugなど現在使用中の設定を選び、リビルドしてください。
4. 画像や音声等は元の配置から参照できるようにしてください。

【注意】
- 入力ZIPには画像・音声アセット、NuGetのpackages、ビルド生成物、もともとの.slnxは含まれていません。
  そのため、このZIP単体でのゲーム実行・ビルド成功は保証できません。
- プロジェクトでは ..\x64\Debug\assets\... にある画像/フォントを参照しています。
  解凍先を変えた場合、画像などの参照先も元プロジェクトと一致させる必要があります。
- StageInit.cpp、TiledLoader.cpp、TiledMap.cpp、main-DESKTOP-DFKNGJU.cpp など、
  元からvcxprojに登録されていないファイルは引き続きビルド対象にはしていません。
  main-DESKTOP-DFKNGJU.cpp は Archive に保管しており、誤って同時にビルドされないようにしています。
- Windowsのファイルシステムでは player.cpp と Player.h の大文字小文字が混在していても動きますが、
  Linuxなど大文字小文字を区別する環境での互換性は別途確認が必要です。
