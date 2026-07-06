# Copilot Instructions

## General Guidelines
- First general instruction
- Second general instruction
- 既存コメントはできるだけ残し、修正時はコメントを削らず最小限の追記にとどめる。

## Project Overview
- 2Dgame_1プロジェクトはC++/raylibで書かれた2Dゲーム。ステージエディタ機能があり、EditorObjectType（当たり判定・ギミック種別）とPlacedObject（配置オブジェクト）という設計パターンを使用。
- コードコメントは日本語で、初心者にもわかりやすいよう詳細なコメントを書くスタイルを好む。

## Game Mechanics
- Implement player death upon enemy side collision (respawn); player should survive when landing on an enemy from above.
- Implement damage to the player when landing on an enemy from above, causing the player to bounce.
- Use the relationship between the player's bottom edge and the enemy's top edge, along with downward velocity, to determine landing collision.
- Ensure stable landing detection without fall motion during walking, as the user prioritizes this aspect.
- Anchor Ojisan to the screen's bottom-left as a fixed UI element, unaffected by camera movement.
- Implement a separate texture mode, activated by the B key, in addition to the stage editing mode (V).
- 敵の死亡テクスチャは側面衝突時のみ反映し、踏みつけ時は通常の挙動のままにする。

## Audio Management
- Manage BGM and sound effects in separate files, with only calls made in main.cpp.
- Ensure all sound effect files are in WAV format.

## Font and Dialog Management
- Ensure the dialog file is included in font codepoints and LoadFromFile timing to prevent garbled Japanese text when jumping high.
- Move font-loading code out of the main function and combine it with dialog content for better organization and performance.

## Texture Management
- SpriteDatabase.cpp の Load() 関数内で読み込み失敗時にダミーテクスチャを生成する。既存コメントはできるだけ残し、修正時は最小限の追記にとどめる。