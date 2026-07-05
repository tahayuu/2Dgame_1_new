#pragma once

// ゲームの状態を全ファイルで共有するために独立させる
enum class GameState {
    START,
    SELECT,
    PLAYING,
    DEAD_SCREEN,
    DEADING_SCREEN,
    Deback,
    EDITOR,
    PAUSE
};