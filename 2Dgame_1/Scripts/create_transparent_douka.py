#!/usr/bin/env python3
"""
douka.png を透過処理したバージョンを作成
全ピクセルのアルファ値を 50% に設定
"""

from PIL import Image
import os

# パスの設定
stage_dir = r"C:\Users\tiany\OneDrive\vscode_game\2Dgame_1\2Dgame_1\assets\images\stage\stage_1"
input_file = os.path.join(stage_dir, "douka.png")
output_file = os.path.join(stage_dir, "douka.png")  # 上書き

print(f"Processing: {input_file}")

# 画像を読み込む
try:
	img = Image.open(input_file)
	print(f"Original image: {img.size}, Mode: {img.mode}")

	# RGBA に変換
	if img.mode != 'RGBA':
		img = img.convert('RGBA')

	# 全ピクセルを処理
	pixels = img.load()
	width, height = img.size

	for y in range(height):
		for x in range(width):
			r, g, b, a = pixels[x, y]
			# アルファ値を 50% に設定（透過度 50%）
			pixels[x, y] = (r, g, b, int(a * 0.5))

	# 保存
	img.save(output_file, "PNG")
	print(f"✓ Saved: {output_file}")
	print(f"Result: Transparent douka.png (50% opacity)")

except Exception as e:
	print(f"Error: {e}")
