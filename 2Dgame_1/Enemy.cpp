#include"Enemy.h"
#include"raylib.h"
#include"StageCollision.h"
#include<cmath>
//==========================
// static
//==========================
static bool CheckOverlapXEnemy(const Rectangle& player, const Enemy& enemy) {
	//X方向が重なっているか
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float objectLeft = enemy.rect.x;
	float objectRight = enemy.rect.x + enemy.rect.width;

	bool overlapX = (playerRight > objectLeft) && (playerLeft < objectRight);

	return overlapX;
}

static bool CheckOverlapYEnemy(const Rectangle& player, const Enemy& enemy) {
	//Y方向が重なっているか
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = enemy.rect.y;
	float hazardBottom = enemy.rect.y + enemy.rect.height;
	bool overlapY = (playerBottom > hazardTop ) && (playerTop < hazardBottom);
	return overlapY;
}

//許容値付きY重なり判定
static bool CheckNealapYEnemy(const Rectangle& player, const Enemy& enemy, float tolerance) {
	//Y方向が重なっているか（踏み判定用）
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = enemy.rect.y;
	float hazardBottom = enemy.rect.y + enemy.rect.height;
	// プレイヤーの下端が敵の上端より下（＝重なり始めている）かつ
	// プレイヤーの上端が敵の下端より上（完全に下に居ない）なら踏み扱い
	bool overlapY = (playerBottom > hazardTop - tolerance) && (playerTop < hazardBottom);
	return overlapY;
}

static void EnemyCollisionErase(Enemy& enemy, const Rectangle& player, float dt, Vector2& velocity, float Y, float X) {
	// X方向の重なりが無ければ当たりなし
	if (!CheckOverlapXEnemy(player, enemy)) return;

	// Y方向の実際の重なりを確認（矩形が重なっているか）
	bool overlapY = CheckOverlapYEnemy(player, enemy);

	// 中心を使って主要方向を判定（横衝突か縦衝突か）
	float playerCenterX = player.x + player.width * 0.5f;
	float playerCenterY = player.y + player.height * 0.5f;
	float enemyCenterX = enemy.rect.x + enemy.rect.width * 0.5f;
	float enemyCenterY = enemy.rect.y + enemy.rect.height * 0.5f;
	float dx = playerCenterX - enemyCenterX;
	float dy = playerCenterY - enemyCenterY;
	bool isMostlyHorizontal = (fabsf(dx) > fabsf(dy)); // true = 横衝突優勢

	// 踏み／頭当たり判定用閾値（必要に応じて調整）
	const float stompTolerance = 15.0f;          // px
	const float stompVelocityThreshold = 50.0f; // px/s

	float enemyTop = enemy.rect.y;
	float enemyBottom = enemy.rect.y + enemy.rect.height;
	float playerBottom = player.y + player.height;
	float playerTop = player.y;

	// 上から踏む条件（矩形が重なっていて、下端が敵上端に近く、かつ下向き速度が十分）
	bool isCloseAbove = (playerBottom <= enemyTop + stompTolerance);
	bool isFallingEnough = (velocity.y > stompVelocityThreshold);

	// 下から（頭当たり）条件（矩形が重なっていて、上端が敵下端に近く、かつ上向き速度が十分）
	bool isCloseBelow = (playerTop >= enemyBottom - stompTolerance);
	bool isRisingEnough = (velocity.y < -stompVelocityThreshold);

	if (!isMostlyHorizontal) {
		// 縦衝突（上または下から）
		if (overlapY && isCloseAbove && isFallingEnough) {
			// 踏み（上から）: 敵にダメージ、プレイヤーを跳ね返す
			enemy.hp -= 1;
			if (velocity.x < 0.0f) velocity.x = -X;
			else if (velocity.x > 0.0f) velocity.x = X;
			velocity.y = -Y;
		
		}
		else if (overlapY && isCloseBelow && isRisingEnough) {
			// 下からの当たり（頭当たり）→ 死亡扱い（フラグ）
			enemy.PlayerTouch = true;

		}
		else {
			// 縦衝突だが踏み/頭条件を満たさない場合（軽い接触など）：何もしない
		}
	}
	else {
		// 横衝突（side）: プレイヤー死亡扱い
		enemy.PlayerTouch = true;

	}
}
//プレイヤーを感知する
static bool playerSence(Rectangle player,Enemy& enemy,int add) {
	if (enemy.rect.x <= player.x + player.width + add) {
		return true;//プレイヤーの右側に敵がいる場合
	}
	else if (enemy.rect.x >= player.x - add) {
		return true;//プレイヤーの左側に敵がいる場合
	}
	else return false;

}

//==========================
// 公開関数の実装
//==========================
void EnemyInit(Enemy& enemy, EnemyType type, Vector2 spawnPos) {
	enemy.type = type;
	enemy.pos = spawnPos;
	enemy.vel = { 0.0f,0.0f };
	enemy.isActive = true;
	enemy.speed = 100.0f;
	enemy.hp = 1;
	enemy.PlayerTouch = false;
	enemy.timer = 0.0f;
	switch (type)
	{
	case EnemyType::WALKER:
		enemy.rect = { spawnPos.x,spawnPos.y,40.0f,40.0f };
		enemy.vel = { 100,100 };
		enemy.hp = 1;
		enemy.patrolMinX = spawnPos.x - 100.0f;
		enemy.patrolMaxX = spawnPos.x + 100.0f;
		break;

	case EnemyType::FLYER:
		enemy.rect = { spawnPos.x,spawnPos.y,40.0f,40.0f };
		enemy.vel = { 100.0f,100.0f };
		enemy.hp = 1;
		enemy.patrolMinY = spawnPos.y - 100.0f;//上
		enemy.patrolMaxY = spawnPos.y + 100.0f;//下
		break;

	case EnemyType::JUMPCOPY:
		enemy.rect = { spawnPos.x,spawnPos.y,40.0f,40.0f };
		enemy.vel = { 100.0f,100.0f };
		enemy.hp = 1;
		enemy.patrolMinX = spawnPos.x - 100.0f;
		enemy.patrolMaxX = spawnPos.x + 100.0f;
		break;
	}


	enemy.rect.x = enemy.pos.x;
	enemy.rect.y = enemy.pos.y;
}

void EnemyCollision(Enemy& enemy, const Rectangle& player,float dt,Vector2& velocity) {
	//敵とプレイヤーの当たり判定
	if (!enemy.isActive)return;
	enemy.timer += dt;
	bool overlapX = CheckOverlapXEnemy(player, enemy);
	switch (enemy.type)
	{
	case EnemyType::WALKER: {
		EnemyCollisionErase(enemy, player, dt, velocity, 200, 100);
		break;
	}
	case EnemyType::FLYER: {
		EnemyCollisionErase(enemy, player, dt, velocity, 200, 100);
		break;
	}
	case EnemyType::JUMPCOPY: {
		EnemyCollisionErase(enemy, player, dt, velocity, 300, 150);
		break;
	}
	}
}

void EnemyUpdate(Enemy& enemy, float dt,const Rectangle& player){
	if (!enemy.isActive)return;
	enemy.timer += dt;
	switch (enemy.type) {
		case EnemyType::WALKER: {
			if (enemy.rect.x <= enemy.patrolMinX) {
				enemy.vel.x = enemy.speed;//右向き
			}
			else if (enemy.rect.x >= enemy.patrolMaxX) {
				enemy.vel.x = -enemy.speed;//左向き
			}
			enemy.pos.x += enemy.vel.x * dt;//移動
			break;
		}
		case EnemyType::FLYER: {
			if (enemy.rect.y <= enemy.patrolMinY) {
				enemy.pos.y = enemy.patrolMinY;
				enemy.vel.y = fabs(enemy.speed);
			}
			else if (enemy.rect.y >= enemy.patrolMaxY) {
				enemy.pos.y = enemy.patrolMaxY;
				enemy.vel.y = -fabs(enemy.speed);
			}
			enemy.pos.y += enemy.vel.y * dt;
			break;
		}
		case EnemyType::JUMPCOPY: {
			if (enemy.rect.x <= enemy.patrolMinX) {
				enemy.vel.x = enemy.speed;
			}
			else if (enemy.rect.x >= enemy.patrolMaxX) {
				enemy.vel.x = -enemy.speed;
			}
			enemy.pos.x += enemy.vel.x * dt;
			if (playerSence(player, enemy, 50 ) && IsKeyPressed(KEY_W) ){
				enemy.vel.y = -830.0f;
				enemy.pos.y += enemy.vel.y * dt;
			}
		}
	}

	enemy.rect.x = enemy.pos.x;
	enemy.rect.y = enemy.pos.y;

	if (enemy.hp <= 0) {
		enemy.isActive = false;
	}
}


void EnemyDraw(const Enemy& enemy){
	if (!enemy.isActive)return;
	switch (enemy.type) {
		case EnemyType::WALKER: {
			DrawRectangleRec(enemy.rect, RED);
			break;
		}
		case EnemyType::FLYER: {
			DrawRectangleRec(enemy.rect, BLUE);
			break;
		}
		default: {
			DrawRectangleRec(enemy.rect, MAROON);
			break;
		}
		case EnemyType::JUMPCOPY: {
			DrawRectangleRec(enemy.rect, RED);
			break;
		}
	}

}
void EnemyReset(Enemy& enemy) {
	enemy.isActive = false;
	enemy.hp = 0;
	enemy.vel = { 0.0f,0.0f };

}
