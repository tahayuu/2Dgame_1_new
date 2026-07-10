#include"PlayerVisual.h"
#include <cmath>

// PlayerVisual.cpp の役割: プレイヤー/背景の見た目更新と描画を実装する。
void PlayerVisualLoad(PlayerVisual& pv) {
	pv.texIdle = LoadTexture("assets/images/player/male_hero-idle.png");
	pv.texRun = LoadTexture("assets/images/player/male_hero-run.png");
	pv.texJump = LoadTexture("assets/images/player/male_hero-jump.png");
	pv.texFall = LoadTexture("assets/images/player/male_hero-fall.png");
	pv.texidleturn = LoadTexture("assets/images/player/male_hero-idle_turn.png");
	pv.texDeath = LoadTexture("assets/images/player/male_hero-death.png");
}

void StageVisualLoad(StageVisual& sv) {
	sv.title = LoadTexture("assets/images/stage/background/title_1.png");
	sv.background = LoadTexture("assets/images/stage/background/stage1.png");
	sv.stage2 = LoadTexture("assets/images/stage/background/stage2.png");
	sv.selectBackground = LoadTexture("assets/images/stage/background/selectStage.png");
	sv.texdeath = LoadTexture("assets/images/stage/background/death.png");
}


void PlayerVisualUnload(PlayerVisual& pv) {
	if (pv.texIdle.id != 0) UnloadTexture(pv.texIdle);
	if (pv.texRun.id != 0) UnloadTexture(pv.texRun);
	if (pv.texJump.id != 0) UnloadTexture(pv.texJump);
	if (pv.texFall.id != 0) UnloadTexture(pv.texFall);
	if (pv.texidleturn.id != 0) UnloadTexture(pv.texidleturn);
	if (pv.texDeath.id != 0) UnloadTexture(pv.texDeath);
}

void StageVisualUnload(StageVisual& sv) {
	if (sv.title.id != 0) UnloadTexture(sv.title);
	if (sv.background.id != 0) UnloadTexture(sv.background);
	if (sv.stage2.id != 0)           UnloadTexture(sv.stage2);
	UnloadTexture(sv.selectBackground);
	if (sv.texdeath.id != 0) UnloadTexture(sv.texdeath);

	sv = StageVisual{};

}


 
//ステージ背景描画（画面全体に描画）
void StageVisualDrawScreen(const StageVisual& sv, int screenW, int screenH) {
	//背景
	if (sv.background.id != 0) {
		Rectangle src = { 0.0f,0.0f,sv.background.width,sv.background.height };
		Rectangle dst = { 0.0f,0.0f,(float)screenW,(float)screenH };

		DrawTexturePro(sv.background, src, dst, { 0.0 }, 0.0f, WHITE);
	}
}

void TitleVisualDrawScreen(const StageVisual& sv, int screenW, int screenH) {
	//背景
	if (sv.background.id != 0) {
		Rectangle src = { 0.0f,0.0f,sv.title.width,sv.title.height };
		Rectangle dst = { 0.0f,0.0f,(float)screenW,(float)screenH };

		DrawTexturePro(sv.title, src, dst, { 0.0 }, 0.0f, WHITE);
	}
}

	void StageVisualDrawWorld(const StageVisual & sv, float worldX, float worldY, float worldW, float worldH)
	{
		if (sv.background.id == 0) return;

		Rectangle src = { 0, 0, (float)sv.background.width, (float)sv.background.height };
		Rectangle dst = { worldX, worldY, worldW, worldH };

		DrawTexturePro(sv.background, src, dst, { 0.0f,0.0f }, 0.0f, WHITE);
	}

// 目的: 速度と接地状態からアニメーション状態を更新する。
// 注意: isDying 中は他アニメ更新を止める。
void PlayerVisualUpdate(PlayerVisual& pv, float dt, const Vector2& velocity, bool onGround) {

	if(pv.isDying) {
		pv.deathTimer += dt;
		float deathFrameTime = 1.0f / pv.deathFPS;//死亡アニメの1コマの時間
		if (pv.deathTimer >= deathFrameTime) {//もしdeathTimerが1/deathFPSを超えたらコマを1つ進める
			pv.deathTimer -= deathFrameTime;//死亡アニメの1コマ分の時間を引く
			if (pv.deathFrame < pv.deathFrameCount - 1) {//死亡アニメの最後のコマまで行っていないなら
				pv.deathFrame++;//死亡アニメのコマ番号を1つ進める
			}
		}
		return;//死亡中はそれ以外のアニメを更新しない
	}

	if (velocity.x > 1.0f) pv.facingRight = true;
	else if (velocity.x < -1.0f) pv.facingRight = false;
	/*絶対値 もし velocity.x が 0 より小さいなら
	→ マイナスをつけてプラスにする
		そうでなければ
		→ そのまま使う */
	float absVx = (velocity.x < 0.0f) ? -velocity.x : velocity.x;//条件 ? 真のとき : 偽のとき
	pv.isRunning = onGround && (absVx > 1.0f);
	pv.isJumping = !onGround && (velocity.y < 0.0f);
	pv.isFalling = !onGround && (velocity.y > 0.0f);
	const int idleFrameCount = pv.frameCount;
	const int runFrameCount = pv.frameCount;
	
	if (pv.isRunning) {//run： 1 / 12 ≈ 0.0833秒ごと
		pv.runTimer += dt;
		float runFrameTime = 1.0f / pv.runFPS;
		if (pv.runTimer >= runFrameTime) {//もしrunTimerが1/runFPSを超えたらコマを1つ進める
			pv.runTimer -= runFrameTime;//そしてrunTimerからその分の時間を引く(誤差をなくすため)
			pv.runFrame = (pv.runFrame + 1) % runFrameCount;//走りアニメのコマ番号を1つ進めて、最後まで行ったら最初に戻す
		}                                      //もし最大を超えたら 0 に戻す(10%10==0番目)コマは0-9までだから
	}
	else if(pv.isJumping){
		pv.jumpTimer += dt;
		float jumpFrameTime = 1.0f / pv.jumpFPS;
		if (pv.jumpTimer >= jumpFrameTime) {
			pv.jumpTimer -= jumpFrameTime;
			pv.jumpFrame = (pv.jumpFrame + 1) % pv.jumpFrameCount;
		}
	}

	else if (pv.isFalling) {
		pv.fallTimer+= dt;
		float fallFrameTime = 1.0f / pv.fallFPS;
		if (pv.fallTimer >= fallFrameTime) {
			pv.fallTimer -= fallFrameTime;
			pv.fallFrame = (pv.fallFrame + 1) % pv.fallFrameCount;
		}
	}

	else {	//idle：1 / 10 = 0.1秒ごと
		pv.idleTimer += dt;
		float idleFrameTime = 1.0f / pv.idleFPS;
		if (pv.idleTimer >= idleFrameTime) {
			pv.idleTimer -= idleFrameTime;
			pv.idleFrame = (pv.idleFrame + 1) % idleFrameCount;
		}
	}
}

//プレイヤーの見た目を今の状態で正しく描画する
void PlayerVisualDraw(const PlayerVisual& pv, const Rectangle& player, const Vector2& velocity, bool gravityReversed, float layerScale) {
	Texture2D tex;
	int frameIndex;
	//どのテクスチャを使うか
	if (pv.isDying) {
		tex = pv.texDeath;
		frameIndex = pv.deathFrame;
	}
	else {
	if (pv.isJumping) {
		tex = pv.texJump;
	}
	else if (pv.isFalling) {
		tex = pv.texFall;
	}
	else if (pv.isRunning) {
		tex = pv.texRun;
	}
	else {
		tex = pv.texIdle;
	}
	if (pv.isJumping) {
		frameIndex = pv.jumpFrame;
	}
	else if (pv.isFalling) {
		frameIndex = pv.fallFrame;
	}
	else if (pv.isRunning) {
		frameIndex = pv.runFrame;
	}
	else {
		frameIndex = pv.idleFrame;
	}
}
	
	// テクスチャが読み込まれていない場合は青い四角を描くだけにする
	if (tex.id == 0) {
		DrawRectangleRec(player, BLUE);
		return;
	}
	
	//テクスチャの何コマ目を描くか計算
	float frameW = 0.0f;
	if (pv.isDying) {
	    frameW = (float)tex.width / pv.deathFrameCount;
	}
	else if (pv.isJumping) {
	    frameW = (float)tex.width / pv.jumpFrameCount;
	}
	else if (pv.isFalling) {
		 frameW = (float)tex.width / pv.fallFrameCount;
	}
	else {
		 frameW = (float)tex.width / pv.frameCount;
	}
	float frameH = (float)tex.height;

	Rectangle src = { frameW * frameIndex, 0.0f, frameW, frameH };

	//左右反転
	if (!pv.facingRight) {
		src.x += src.width;
		src.width = -src.width;
	}

	//重力反転時：上下反転
	if (gravityReversed) {
		src.y += src.height;
		src.height = -src.height;
	}

	const float drawScale = pv.drawSvale;

	// プレイヤーの中心座標（ワールド座標のまま）
	float centerX = player.x + player.width * 0.5f;
	float centerY = player.y + player.height * 0.5f;

	// 見た目用の描画矩形（奥レイヤーではサイズだけ縮小）
	Rectangle dst = {
		centerX,
		centerY,
		player.width * drawScale * layerScale,
		player.height * drawScale * layerScale
	};

	Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };

	// 奥レイヤー：少し暗くして奥行き感を強調
	Color tint = (layerScale < 1.0f) ? Color{ 200, 200, 220, 255 } : WHITE;
	DrawTexturePro(tex, src, dst, origin, 0.0f, tint);
}



/*flowchart TD
A["毎フレーム Update 呼び出し"] -- > B{ "hasJumpBoost?" }
B-- "true" -- > C["spawnTimer を加算"]
C-- > D{ "spawnTimer >= spawnInterval?" }
D-- "true" -- > E["空きスロットを探して\n新しいパーティクルを1粒生成"]
D-- "false" -- > F["生成スキップ"]
B-- "false" -- > G["spawnTimer リセット\n（新規生成しない）"]
E-- > H["全パーティクルの\nlife 減算・位置更新"]
F-- > H
G-- > H
H-- > I["毎フレーム Draw 呼び出し"]
I-- > J["active な粒だけ\nフェードアウトしながら円を描画"]*/

void PlayerParticleUpdate(PlayerParticleEffect& effect, float dt, const Rectangle& player, bool hasJumpBoost) {
	if (hasJumpBoost) {//ジャンプブーストがあるときだけ粒を出す
		effect.spawnTimer += dt;
		//タイマーが生成間隔を超えたら粒を生成する
		while (effect.spawnTimer >= effect.spawnInterval) {
			effect.spawnTimer -= effect.spawnInterval;
			//配列の中から使っていない粒を探す
			for (int i = 0; i < MAX_PLAYER_PARTICLES; i++) {// 32個の中から
				if(!effect.particles[i].active) {// 使っていない粒を探す
					PlayerParticle& p = effect.particles[i];
					p.active = true;// 「この粒は使用中」にする

					//プレイヤーの中心位置を基準に、ランダムな位置に粒を出す
					float cx = player.x + player.width * 0.5f;
					float cy = player.y + player.height * 0.5f;
					//中心からランダムにばらけさせる
					float offsetX = (float)GetRandomValue(-40, 40);
					float offsetY = (float)GetRandomValue(-40, 40);
					//粒の初期位置 = プレイヤーの中心 + ランダムなオフセット
					p.pos = { cx + offsetX,cy + offsetY };
					//粒の速度もランダムにする
					   // X: 左右にゆるく (-30〜+30 px/秒)
					// Y: 上方向に浮く (-80〜-30 px/秒、マイナス=上)
					p.vel = {
						(float)GetRandomValue(-30,30),
						(float)GetRandomValue(-30,30)
					};
					//粒の寿命もランダムにする
					 // 0.6〜1.0秒(長い程遠くまで飛ぶ)
					p.maxLife = 0.6f + (float)GetRandomValue(0, 40) / 100.0f;
					p.life = p.maxLife;//残り寿命 ＝初期寿命

					//粒のサイズもランダムにする
					p.size = 3.0f + (float)GetRandomValue(0, 30) / 10.0f;

					//粒の色もランダムにする（明るい水色〜白系）
					int r = GetRandomValue(100, 200);
					int g = GetRandomValue(200, 255);
					int b = GetRandomValue(220, 255);
					p.color = { (unsigned char)r,
						(unsigned char)g,
						(unsigned char)b,
						255//最初は完全に不透明 
					};
					//unsigned を付けて定義した整数型の変数は符号なし整数型となり、負の値を表現できなくなる
					break;//粒を生成したらループを抜ける
				}
			}
		}
	}
	else {
		effect.spawnTimer = 0.0f;//ジャンプブーストがないときはタイマーをリセットして粒を生成しない
	}

	for (int i = 0; i < MAX_PLAYER_PARTICLES; i++) {
		PlayerParticle& p = effect.particles[i];
		if (!p.active) continue;//使っていない粒はスキップ
		p.life -= dt;//粒の寿命を減らす
		if (p.life <= 0.0f) {
			p.active = false;//寿命が尽きたら粒を消す
			continue;
		}
		p.pos.x += p.vel.x * dt;//粒の位置を速度に応じて動かす
		p.pos.y += p.vel.y * dt;

		// sinf で横にゆらゆら揺らす
		// life * 8.0f → 寿命が減るにつれ位相が進み、揺れが変化
		// * 20.0f * dt → 1秒あたり最大 ±20px の揺れ幅   
		p.pos.x += sinf(p.life * 8.0f) * 20.0f * dt;
	}
}

void PlayerParticleDraw(const PlayerParticleEffect& effect) {
	for (int i = 0; i < MAX_PLAYER_PARTICLES; i++) {
		const PlayerParticle& p = effect.particles[i];
		if (!p.active)continue;
		float alpha = p.life / p.maxLife;
		/*
		* alpha = 残り寿命 ÷ 初期寿命
            生まれた瞬間: life=1.0, maxLife=1.0 → 1.0 ÷ 1.0 = 1.0
            半分経過:     life=0.5, maxLife=1.0 → 0.5 ÷ 1.0 = 0.5
            消える直前:   life=0.1, maxLife=1.0 → 0.1 ÷ 1.0 = 0.1
            消滅時:       life=0.0              → active=false でここには来ない
            alpha の範囲は必ず 0.0 〜 1.0 になる
		*/
		Color c = p.color;
		/*
		alpha × 255 の変化：
  alpha = 1.0 → 1.0 × 255 = 255（完全に見える）
  alpha = 0.5 → 0.5 × 255 = 127（半透明）
  alpha = 0.1 → 0.1 × 255 =  25（ほぼ透明）
  なぜ p.color を直接変えないのか？
  p は const なので書き換え不可。
  Color c にコピーしてから c.a だけ変える。
		struct Color {
    unsigned char r;  // 赤   Red
    unsigned char g;  // 緑   Green
    unsigned char b;  // 青   Blue
    unsigned char a;  // 透明度 Alpha  ← これ
};*/
		c.a = (unsigned char)(alpha * 255.0f);

		float currentSize = p.size * alpha;//サイズも徐々に小さくする
		DrawCircleV(p.pos, currentSize, c);
		/*center	Vector2	pos	円の中心座標（ワールド座標）
          radius	float	currentSize	円の半径（px）
          Color	    Color	c	塗りつぶし色（アルファ込み）
*/
	}
}


