#include "EnemyManager.h"

void EnemyManager::Init()
{
	//•	ゲーム開始・ステージ切替時に内部配列を「空」にして、追加に備えるため。
	enemies.clear();//既存のデータをクリア
	enemies.reserve(32); //あらかじめ16体分のメモリを確保);

	playerTouched = false;
	touchedEnemyFromSide = false;
	touchedEnemyIndex = -1;
}

void EnemyManager::Spawn(EnemyType Type, Vector2 pos){
	//•	ステージ上に敵を出現させる（プレイヤーが近づいたときなどに呼ぶ想定）
	Enemy enemy;
	EnemyInit(enemy, Type, pos);
	enemies.push_back(enemy);
}
//敵とプレイヤーの当たり判定
void EnemyManager::EnemyCollisionAll(const Rectangle& player, float dt, Vector2& velocity) {
	playerTouched = false;
	touchedEnemyFromSide = false;
	touchedEnemyIndex = -1;
	touchedEnemyDialogKey = "";

	for (auto& e : enemies) {
		if (e.isActive)EnemyCollision(e, player, dt, velocity);

		if (e.PlayerTouch) {
			playerTouched = true;
			touchedEnemyDialogKey = e.dialogKey; //当たった敵のキーを保存
			e.PlayerTouch = false;
		}
	}
}

void EnemyManager::UpdateAll(float dt, const Rectangle& player) {
	for (auto& enemy : enemies) {
		if (enemy.isActive)EnemyUpdate(enemy, dt, player);
	}
	enemies.erase(std::remove_if(enemies.begin(),enemies.end(),
		[](const Enemy& enemy) {return !enemy.isActive; }),
		enemies.end());//非アクティブな敵を削除
	/*enemies.erase( it, enemies.end() )
	eraseでは(1つ目の引数,2つ目の引数)の範囲が削除される。
	指定したイテレータ it から enemies.end() までの範囲にある要素をベクタから削除して破棄し、サイズを縮める。
	std::remove_if は、指定された条件に基づいて要素を移動させ、削除すべき要素をベクタの末尾に集める。
	[](const Enemy& enemy) {return !enemy.isActive; }//ラムダ式：非アクティブな敵を削除する条件
	ラムダ式を使うことで、簡単に関数オブジェクトを作成できる。
	定義： [](引数リスト) { 関数本体 } 引数は省略可能
	呼び出し: [](const Enemy& enemy) {return !enemy.isActive; }(enemy)
	*/
}

void EnemyManager::DrawAll() {
	for (const auto& enemy : enemies) {
		EnemyDraw(enemy);
	}
}

void EnemyManager::saveEnemiesInit() {
	enemiesInit = enemies;
}


void EnemyManager::RestorInitialEnemies() {
	enemies = enemiesInit;
	for (auto& enemy : enemies) {
		enemy.isActive = true;
		enemy.isHit = false;
		enemy.touchedFromSide = false;
		enemy.PlayerTouch = false;
	}
}
void EnemyManager::Reset() {
	for (auto& enemy : enemies) EnemyReset(enemy);
	enemies.clear();
	touchedEnemyFromSide = false;
	touchedEnemyIndex = -1;
}