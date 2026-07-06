#include <stdio.h>
#include <string.h>

// Windows GDI+ を使用する場合のヘッダは複雑なため、
// raylib の内部 PNG 処理を使用
// または外部ツール ImageMagick の convert コマンドを使用

// ImageMagick install check and run
int main() {
	// convert コマンドでアルファを50%に設定
	system("cd C:\\Users\\tiany\\OneDrive\\vscode_game\\2Dgame_1\\2Dgame_1\\assets\\images\\stage\\stage_1\\ && convert douka.png -alpha set -channel A -evaluate multiply 0.5 douka.png");
	printf("Done!\\n");
	return 0;
}
