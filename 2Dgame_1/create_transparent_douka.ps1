Add-Type -AssemblyName System.Drawing

$inputPath = "C:\Users\tiany\OneDrive\vscode_game\2Dgame_1\2Dgame_1\assets\images\stage\stage_1\douka.png"
$outputPath = $inputPath  # 上書き

# 画像を読み込む
$bitmap = New-Object System.Drawing.Bitmap($inputPath)
$width = $bitmap.Width
$height = $bitmap.Height

Write-Host "Processing: $inputPath"
Write-Host "Size: ${width}x${height}"

# 全ピクセルのアルファ値を50%に設定
for ($y = 0; $y -lt $height; $y++) {
	for ($x = 0; $x -lt $width; $x++) {
		$pixel = $bitmap.GetPixel($x, $y)

		# アルファ値を50%に（半透明化）
		$newAlpha = [Math]::Min(255, [int]($pixel.A * 0.5))
		$newColor = [System.Drawing.Color]::FromArgb($newAlpha, $pixel.R, $pixel.G, $pixel.B)

		$bitmap.SetPixel($x, $y, $newColor)
	}

	# 進捗表示
	if ($y % 50 -eq 0) {
		Write-Host "Processing: $y / $height"
	}
}

# 保存
$bitmap.Save($outputPath)
$bitmap.Dispose()

Write-Host "✓ Complete! Saved: $outputPath"
Write-Host "Result: douka.png (50% transparent)"
