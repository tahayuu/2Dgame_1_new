#include "CameraController.h"

// CameraController.cpp の役割: カメラ追従ロジック本体。
void UpdateCamera(
    Camera2D& camera,
    const Rectangle& player,
    const CameraConfig& config,
    float stageWidth,
    float stageHeight,
    int   screenWidth,
    int   screenHeight,
    float dt)
{
    // X axis clamp
    float cameraX = player.x + player.width / 2.0f;
    if (cameraX < screenWidth / 2.0f)
        cameraX = screenWidth / 2.0f;
    if (cameraX > stageWidth - screenWidth / 2.0f)
        cameraX = stageWidth - screenWidth / 2.0f;

    // Y axis zone detection
    const float groundFixedY = screenHeight / 2.0f;
    const float undergroundFixedY = stageHeight - screenHeight / 2.0f;
    const float shaftEnterY = (config.shaftEnterY > 0.0f)
        ? config.shaftEnterY
        : (float)screenHeight;
    const float shaftExitY = (config.shaftExitY > 0.0f)
        ? config.shaftExitY
        : stageHeight - (float)screenHeight;

    float playerCenterY = player.y + player.height / 2.0f;
    float targetCameraY;

    if (config.twoLayered && stageHeight > (float)screenHeight) {
        if (playerCenterY <= shaftEnterY) {
            targetCameraY = groundFixedY;
        }
        else if (playerCenterY >= shaftExitY) {
            targetCameraY = undergroundFixedY;
        }
        else {
            targetCameraY = playerCenterY;
        }
    }
    else {
        targetCameraY = groundFixedY;
    }

    // Smooth lerp
    float speed = config.lerpSpeed > 0.0f ? config.lerpSpeed : 10.0f;
    float lerpFactor = speed * dt;
    if (lerpFactor > 1.0f) lerpFactor = 1.0f;
    float cameraY = camera.target.y + (targetCameraY - camera.target.y) * lerpFactor;

    camera.target = { cameraX, cameraY };
}