#include "StageEventChanger.h"
// ================================================================
// 目的: 指定した eventId に対応する DecoSprite を検索する。
// 戻り値: 見つかればポインタ、見つからなければ nullptr
// ================================================================
static DecoSprite* FindDecoSpriteByEventId(Stage& stage,int eventId) {

    if (eventId <= 0) {
		return nullptr;// 無効なイベントIDの場合はnullptrを返す
    }

    for (int i = 0; i < stage.decoSpriteCount; i++) {
        auto& deco = stage.decoSprites[i];

        if (deco.eventId == eventId) {
			return &deco;// 見つかった場合はポインタを返す
        }
    }

    return nullptr;
}

/*この関数の目的
ApplyEventChanger は、イベントが発動したときに次の3つを行う関数です。
変更対象となる看板などの画像を探す
変更前の状態を保存する
看板画像とジャンプ操作を変更し、イベントを発動済みにする*/
static void ApplyEventChanger(Stage& stage, EventChanger& event){
    DecoSprite* target = FindDecoSpriteByEventId(stage, event.targetEventId);
	// 変更対象の DecoSprite が見つかった場合に処理を行う
	if (!event.originalStateSaved) {// まだ元の状態が保存されていない場合
        if (target != nullptr) {
			event.originalSpriteId = target->spriteId;// 元のスプライトIDを保存
		
        }
		event.originalJumpMode = stage.currentJumpMode;// 元のジャンプモードを保存

		event.originalStateSaved = true;// 元の状態を保存済みにする
    }

    if (target != nullptr && event.changedSpriteId != SpriteId::None) {
		target->spriteId = event.changedSpriteId;// DecoSprite のスプライトIDを変更
    }

	stage.currentJumpMode = event.jumpMode;// ジャンプモードを変更

	event.triggered = true;// イベントを発動済みにする
}

///この関数の目的
static void REstoreEventChanger(Stage& stage, EventChanger& event) {
    
    if (!event.originalStateSaved)return;

	DecoSprite* target = FindDecoSpriteByEventId(stage, event.targetEventId);
    if (target != nullptr) {
        target->spriteId = event.originalSpriteId;
    }
    stage.currentJumpMode = event.originalJumpMode;
}


void UpdateEventChangers(Stage& stage, const Rectangle& playerRect) {
    for (int i = 0; i < stage.eventChangerCount; i++) {
        auto& event = stage.eventChangers[i];
		const bool inside = CheckCollisionRecs(playerRect, event.rect);// プレイヤーがイベントチェンジャーの中にいるか
		const bool entered = inside && !event.playerWasInside;// プレイヤーがイベントチェンジャーに入った瞬間
		const bool exited = !inside && event.playerWasInside;// プレイヤーがイベントチェンジャーから出た瞬間
        if (entered){
            /*組み合わせ
                oneShot	triggered	canTrigger	意味
                false	false	true	繰り返しイベントなので発動可能
                false	true	true	発動済みでも繰り返しイベントなので再発動可能
                true	false	true	一度きりだが、まだ未発動
                true	true	false	一度きりで、すでに発動済み*/
			const bool canTrigger = !event.oneShot || !event.triggered;// 繰り返しイベント、または未発動の一度きりイベント
            if (canTrigger) {
                ApplyEventChanger(stage, event);
            };
        }
        if (exited && event.restoreOnExit) {// プレイヤーがイベントチェンジャーから出た瞬間で、元に戻す設定がある場合
            REstoreEventChanger(stage, event);

        };
        event.playerWasInside = inside; //現在のフレームでエリア内にいるかどうかを保存し、次のフレームで「前回の状態」として使います
    }

}