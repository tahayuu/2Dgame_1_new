#include "StageGimmickSignal.h"
// 指定された信号IDが有効かどうかを判定する
static bool IsValidSignalId(int signalId) {
	return signalId > 0 && signalId < Stage::MAX_GIMMICK_SIGNALS;
}
// すべての信号をOFFにする
void ClearGimmickSignals(Stage& stage) {
	for (int i = 0; i < Stage::MAX_GIMMICK_SIGNALS; i++) {
		stage.gimmickSignals[i] = false;
	}
}


/*1. すべての信号をOFFにする

2. ステージ内のDistanceTriggerPieceを1個ずつ確認する

3. パーツが外れていなければ無視する

4. 出力先の信号IDが無効なら無視する

5. パーツが外れていて、信号IDも正しければ、
   対応する信号をONにする*/

// 指定された信号がONか
void RebuildGimmickSignals(Stage& stage) {
	ClearGimmickSignals(stage);

	for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
		const DistanceTriggerPiece& trigger = stage.distanceTriggerPieces[i];
		// 離れた距離で発動するピースが初期位置から外れているかどうかを判定する
		if (!trigger.detached) continue;
		// 離れた距離で発動するピースが初期位置から外れている場合、対応する信号をONにする
		if (!IsValidSignalId(trigger.outputSignalId)) continue;

		stage.gimmickSignals[trigger.outputSignalId] = true;
	}
}

bool IsGimmickSignalActive(const Stage& stage, int signalId) {
	if (!IsValidSignalId(signalId)) return false;
	return stage.gimmickSignals[signalId];
}