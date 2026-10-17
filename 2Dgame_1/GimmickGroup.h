#pragma once
#include<array>
#include<cstddef>
#include<stdexcept>

template<typename T, int N>//typename Tによって型を固定せずクラスを定義できるようになる
//templateを使うことですべて同じGimmickGroupクラスを使えるようになる

class GimmickGroup {
private:
	//データを自分で所有
	std::array<T, N> objects{};
	std::array<T, N> initialObjects{};
	size_t count = 0;
	//size_t は負の整数を扱わない型で、配列のサイズやインデックスに適している

public:

	void Reset() {
		for (std::size_t i = 0; i < count; i++) {
			objects[i] = initialObjects[i];
		}
	}

	void SaveInitialState() {
		for (std::size_t i = 0; i < count; i++) {
			initialObjects[i] = objects[i];
		}
	}

	//ギミックを追加する関数
	bool Add(const T& object) {
		if (count >= N) {
			return false;
		}

		objects[count] = object;
		initialObjects[count] = object;
		count++;
		return true;
	}

	//配列をprivateにしているため、外部から直接アクセスできないようにする
	//Get関数によって、外部からギミックを取得することができるようにする
	T& Get(std::size_t index) {
		if (index < 0 || index >= count) {
			throw std::out_of_range("Index out of range");
		}
		return objects[index];
	}
	/*取得したギミックを変更できる
	auto& block = blocks.Get(0);
	  block.isBroken = true;  が可能*/


	  //const版のGet関数を追加することで、constなGimmickGroupオブジェクトからもギミックを取得できるようにする
	const T& Get(std::size_t index) const {
		if (index >= count) {
			throw std::out_of_range("GimmickGroup index");
		}

		return objects[index];
	}
	/*const auto& group = stage.touchBreakBlocks;
	 const auto& block = group.Get(0);とすると 取得したブロックを読み取ることはできても変更することはできない*/

	 /*なぜ2種類必要なのか？
	 ゲームでは、処理の目的によってデータを変更する必要が異なる
	処理	必要な操作
	StageUpdate	ギミックの状態を変更する
	StageCollision	当たり判定に応じて状態を変更する
	StageDraw	ギミックの状態を読み取る
	StageEditorIO	ギミックの状態を読み取ってエディタへ反映する
	例えば描画処理では、基本的にギミックの状態を変更する必要がない
	そこで const な参照を利用することで、意図しない状態変更を防ぐ*/

	int Size() const {//管理しているギミックの個数を返す関数
		return count;
	}

	void Clear() {//管理しているギミックをすべて削除する関数
		count = 0;
	}
};