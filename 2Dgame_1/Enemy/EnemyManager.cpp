#include "EnemyManager.h"

// ================================================================
// EnemyManager.cpp の役割
// ---------------------------------------------------------------
// ・Enemy単体APIをまとめて呼び出し、敵群のライフサイクルを管理する。
// ・プレイヤーとの接触結果を1フレーム単位で取りまとめる。
// ================================================================

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
	auto enemy = CreateEnemy(Type, pos);
	if (enemy) enemies.push_back(std::move(enemy));
}
//敵とプレイヤーの当たり判定
// 目的: 各敵の接触判定を回し、プレイヤー接触情報を1つに集約する。
void EnemyManager::EnemyCollisionAll(const Rectangle& player, float dt, Vector2& velocity, AudioManager& audio) {
	playerTouched = false;
	touchedEnemyFromSide = false;
	touchedEnemyIndex = -1;
	touchedEnemyDialogKey = "";

	for (int i = 0; i < (int)enemies.size(); ++i) {
		auto& e = enemies[i];

		if (e->isActive) e->OnCollision(player, dt, velocity, audio);

		if (e->PlayerTouch) {
			AudioPlaySfx(audio, SfxId::childsLaugh);
			playerTouched = true;
			touchedEnemyDialogKey = e->dialogKey;
			touchedEnemyIndex = i;
			touchedEnemyFromSide = e->touchedFromSide;
			e->PlayerTouch = false;
		}
	}
}

void EnemyManager::UpdateAll(float dt, const Rectangle& player) {
	for (auto& enemy : enemies) {
		if (enemy->isActive) enemy->Update(dt, player);
	}
	enemies.erase(std::remove_if(enemies.begin(),enemies.end(),
		[](const std::unique_ptr<Enemy>& enemy) {return !enemy->isActive; }),
		enemies.end());//非アクティブな敵を削除
	/*enemies.erase( it, enemies.end() )
	eraseでは(1つ目の引数,2つ目の引数)の範囲が削除される。
	指定したイテレータ it から enemies.end() までの範囲にある要素をベクタから削除して破棄し、サイズを縮める。
	std::remove_if は、指定された条件に基づいて要素を移動させ、削除すべき要素をベクタの末尾に集める。
	[](const std::unique_ptr<Enemy>& enemy) {return !enemy->isActive; }//ラムダ式：非アクティブな敵を削除する条件
	ラムダ式を使うことで、簡単に関数オブジェクトを作成できる。
	定義： [](引数リスト) { 関数本体 } 引数は省略可能
	呼び出し: [](const std::unique_ptr<Enemy>& enemy) {return !enemy->isActive; }(enemy)
	*/
}

void EnemyManager::DrawAll() {
	for (const auto& enemy : enemies) {
		enemy->Draw();
	}
}

void EnemyManager::saveEnemiesInit() {
	enemiesInit.clear();
	enemiesInit.reserve(enemies.size());
	for (const auto& enemy : enemies) {
		enemiesInit.push_back({ enemy->type, enemy->pos, enemy->vel, enemy->speed, enemy->hp,
			enemy->patrolMinX, enemy->patrolMaxX, enemy->patrolMinY, enemy->patrolMaxY,
			enemy->dialogKey });
}
}


void EnemyManager::RestorInitialEnemies() {
	enemies.clear();
	enemies.reserve(enemiesInit.size());
	for (const auto& initial : enemiesInit) {
		auto enemy = CreateEnemy(initial.type, initial.pos);
		enemy->vel = initial.vel;
		enemy->speed = initial.speed;
		enemy->hp = initial.hp;
		enemy->patrolMinX = initial.patrolMinX;
		enemy->patrolMaxX = initial.patrolMaxX;
		enemy->patrolMinY = initial.patrolMinY;
		enemy->patrolMaxY = initial.patrolMaxY;
		enemy->dialogKey = initial.dialogKey;
		enemies.push_back(std::move(enemy));
	}
}
void EnemyManager::Reset() {
	for (auto& enemy : enemies) enemy->Reset();
	enemies.clear();
	touchedEnemyFromSide = false;
	touchedEnemyIndex = -1;
}