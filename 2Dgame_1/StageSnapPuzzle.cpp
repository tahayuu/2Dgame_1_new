#include "StageSnapPuzzle.h"
#include "SpriteDatabase.h"
#include <cmath>
#include <cfloat>

static Vector2 GetRectCenter(const Rectangle& rect) {
    return { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
}

// ピースの移動を開始する（スナップ中や元の位置に戻る場合に使用）
static void StartPieceMove(DragPiece& piece, Vector2 target, DragPieceState state) {
    piece.moveStart = { piece.rect.x, piece.rect.y };
    piece.moveTarget = target;
    piece.moveTimer = 0.0f;
    piece.moveDuration = 0.15f;
    piece.state = state;
}
// ピースの移動アニメーションを更新する
static void UpdateDragPieceAnimation(DragPiece& piece, float dt) {
    if (piece.effectTimer > 0.0f) {
        piece.effectTimer -= dt;
    }

    if (piece.state != DragPieceState::SNAPPING &&
        piece.state != DragPieceState::RETURNING) {
        return;
    }

    piece.moveTimer += dt;

    float t = piece.moveDuration > 0.0f
        ? piece.moveTimer / piece.moveDuration
        : 1.0f;

    if (t > 1.0f) {
        t = 1.0f;
    }

    const float eased = 1.0f - powf(1.0f - t, 3.0f);

    piece.rect.x =
        piece.moveStart.x +
        (piece.moveTarget.x - piece.moveStart.x) * eased;

    piece.rect.y =
        piece.moveStart.y +
        (piece.moveTarget.y - piece.moveStart.y) * eased;

    if (t < 1.0f) {
        return;
    }

    piece.rect.x = piece.moveTarget.x;
    piece.rect.y = piece.moveTarget.y;

    if (piece.state == DragPieceState::SNAPPING) {
        piece.state = DragPieceState::SNAPPED;
        piece.effectTimer = 0.18f;
    }
    else {
        piece.state = piece.currentSlotId >= 0
            ? DragPieceState::SNAPPED
            : DragPieceState::FREE;
    }
}

static float GetDistanceFromInitialRect(const DragPiece& piece) {
    const Vector2 currentCenter = GetRectCenter(piece.rect);
    const Vector2 initialCenter = GetRectCenter(piece.initialRect);

    const float dx = currentCenter.x - initialCenter.x;
    const float dy = currentCenter.y - initialCenter.y;

    return sqrtf(dx * dx + dy * dy);
}
static void UpdateDistanceTriggerState(DistanceTriggerPiece& trigger) {
    const float distance = GetDistanceFromInitialRect(trigger.drag);

    if (!trigger.detached &&
        distance >= trigger.detachDistance) {

        trigger.detached = true;
    }
    else if (trigger.detached &&
        distance <= trigger.reattachDistance) {

        trigger.detached = false;
    }
}

static void StartDistanceTriggerReturn(DistanceTriggerPiece& trigger) {
    DragPiece& piece = trigger.drag;

    StartPieceMove(
        piece,
        {
            piece.initialRect.x,
            piece.initialRect.y
        },
        DragPieceState::RETURNING
    );
}

static void BeginDragPiece(DragPiece& piece, Vector2 mouseWorld) {
    piece.state = DragPieceState::DRAGGING;

    piece.dragOffset = {
        mouseWorld.x - piece.rect.x,
        mouseWorld.y - piece.rect.y
    };
}
static void UpdateDraggedPiecePosition(DragPiece& piece, Vector2 mouseWorld) {
    piece.rect.x = mouseWorld.x - piece.dragOffset.x;
    piece.rect.y = mouseWorld.y - piece.dragOffset.y;
}

// ピースをスロットの中央に配置するための座標を計算する
static Vector2 GetPiecePositionInSlot(const DragPiece& piece, const SnapSlot& slot) {
    return {
        slot.rect.x + (slot.rect.width - piece.rect.width) * 0.5f,
        slot.rect.y + (slot.rect.height - piece.rect.height) * 0.5f
    };
}
// グループIDとスロットIDに一致するスロットのインデックスを検索する
static int FindSlotIndex(const Stage& stage, int groupId, int slotId) {
    for (int i = 0; i < stage.snapSlotCount; i++) {
        const SnapSlot& slot = stage.snapSlots[i];
        if (slot.groupId == groupId && slot.slotId == slotId) return i;
    }
    return -1;
}
// ピースIDとグループIDに一致するピースのインデックスを検索する
static int FindPieceIndex(const Stage& stage, int groupId, int pieceId) {
    for (int i = 0; i < stage.dragPieceCount; i++) {
        const DragPiece& piece = stage.dragPieces[i];
        if (piece.groupId == groupId && piece.pieceId == pieceId) return i;
    }
    return -1;
}

// ピースをスロットの中央に移動させる（スナップ中に使用）
static void StartPieceMoveToSlot(DragPiece& piece, const SnapSlot& slot) {
    StartPieceMove(piece, GetPiecePositionInSlot(piece, slot), DragPieceState::SNAPPING);
}

static void StartPieceReturn(Stage& stage, DragPiece& piece) {
    const int originSlotIndex = FindSlotIndex(stage, piece.groupId, piece.dragOriginSlotId);
    Vector2 target = { piece.initialRect.x, piece.initialRect.y };

    if (originSlotIndex >= 0) target = GetPiecePositionInSlot(piece, stage.snapSlots[originSlotIndex]);

    piece.dragOriginSlotId = -1;
    StartPieceMove(piece, target, DragPieceState::RETURNING);
}

static bool CanUseDestinationSlot(const Stage& stage, const DragPiece& piece, const SnapSlot& slot) {
    if (slot.groupId != piece.groupId) return false;
    if (slot.slotId == piece.dragOriginSlotId) return false;
    if (slot.occupiedPieceId < 0) return true;
    if (!piece.allowSwap || piece.dragOriginSlotId < 0) return false;

    const int otherPieceIndex = FindPieceIndex(stage, piece.groupId, slot.occupiedPieceId);
    if (otherPieceIndex < 0) return false;
    return !stage.dragPieces[otherPieceIndex].locked;
}

static int FindNearestSnapSlot(const Stage& stage, const DragPiece& piece) {
    const Vector2 pieceCenter = GetRectCenter(piece.rect);
    float nearestDistance = FLT_MAX;
    int nearestIndex = -1;

    for (int i = 0; i < stage.snapSlotCount; i++) {
        const SnapSlot& slot = stage.snapSlots[i];
        if (!CanUseDestinationSlot(stage, piece, slot)) continue;

        const Vector2 slotCenter = GetRectCenter(slot.rect);
        const float dx = pieceCenter.x - slotCenter.x;
        const float dy = pieceCenter.y - slotCenter.y;
        const float distance = sqrtf(dx * dx + dy * dy);

        if (distance <= slot.snapRadius && distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

static bool PlacePieceInSlot(Stage& stage, int pieceIndex, int destinationSlotIndex) {
    if (pieceIndex < 0 || pieceIndex >= stage.dragPieceCount) return false;
    if (destinationSlotIndex < 0 || destinationSlotIndex >= stage.snapSlotCount) return false;

    DragPiece& piece = stage.dragPieces[pieceIndex];
    SnapSlot& destination = stage.snapSlots[destinationSlotIndex];
    const int originSlotIndex = FindSlotIndex(stage, piece.groupId, piece.dragOriginSlotId);
    const int otherPieceId = destination.occupiedPieceId;

    if (otherPieceId >= 0) {
        if (!piece.allowSwap || originSlotIndex < 0) return false;

        const int otherPieceIndex = FindPieceIndex(stage, piece.groupId, otherPieceId);
        if (otherPieceIndex < 0 || stage.dragPieces[otherPieceIndex].locked) return false;

        DragPiece& otherPiece = stage.dragPieces[otherPieceIndex];
        SnapSlot& origin = stage.snapSlots[originSlotIndex];

        origin.occupiedPieceId = otherPiece.pieceId;
        origin.effectTimer = 0.25f;
        otherPiece.currentSlotId = origin.slotId;
        otherPiece.dragOriginSlotId = -1;
        StartPieceMoveToSlot(otherPiece, origin);
    }
    else if (originSlotIndex >= 0) {
        stage.snapSlots[originSlotIndex].occupiedPieceId = -1;
    }

    destination.occupiedPieceId = piece.pieceId;
    destination.effectTimer = 0.25f;
    piece.currentSlotId = destination.slotId;
    piece.dragOriginSlotId = -1;
    StartPieceMoveToSlot(piece, destination);
    return true;
}

bool IsSnapGroupSolved(const Stage& stage, int groupId) {
    if (groupId <= 0) return false;

    bool hasRequiredSlot = false;
    for (int i = 0; i < stage.snapSlotCount; i++) {
        const SnapSlot& slot = stage.snapSlots[i];
        if (slot.groupId != groupId || slot.requiredPieceId <= 0) continue;

        hasRequiredSlot = true;
        if (slot.occupiedPieceId != slot.requiredPieceId) return false;
    }

    return hasRequiredSlot;
}

static void ApplySolvedLocks(Stage& stage) {
    for (int i = 0; i < stage.snapSlotCount; i++) {
        const SnapSlot& slot = stage.snapSlots[i];
        if (!slot.lockOnCorrect || slot.requiredPieceId <= 0) continue;
        if (slot.occupiedPieceId != slot.requiredPieceId) continue;

        const int pieceIndex = FindPieceIndex(stage, slot.groupId, slot.occupiedPieceId);
        if (pieceIndex >= 0) stage.dragPieces[pieceIndex].locked = true;
    }

    for (int i = 0; i < stage.dragPieceCount; i++) {
        DragPiece& piece = stage.dragPieces[i];
        if (piece.lockOnSolve && IsSnapGroupSolved(stage, piece.groupId)) piece.locked = true;
    }
}
// ピースのアニメーションを更新する
static void UpdatePieceAnimations(Stage& stage, float dt) {
    for (int i = 0; i < stage.dragPieceCount; i++) {
        UpdateDragPieceAnimation(stage.dragPieces[i], dt);
    }

    for (int i = 0; i < stage.snapSlotCount; i++) {
        SnapSlot& slot = stage.snapSlots[i];

        if (slot.effectTimer > 0.0f) {
            slot.effectTimer -= dt;
        }
    }
}

bool UpdateSnapPuzzles(Stage& stage, Camera2D camera, float dt) {
    UpdatePieceAnimations(stage, dt);

    for (int i = 0; i < stage.snapSlotCount; i++) stage.snapSlots[i].highlighted = false;

    const Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    const bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const bool mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    bool consumedThisFrame = stage.snapMouseCaptured;

    if (stage.draggingSnapPieceIndex < 0 && mousePressed) {
        for (int i = stage.dragPieceCount - 1; i >= 0; i--) {
            DragPiece& piece = stage.dragPieces[i];
            if (piece.locked) continue;
            if (piece.state != DragPieceState::FREE && piece.state != DragPieceState::SNAPPED) continue;
            if (!CheckCollisionPointRec(mouseWorld, piece.rect)) continue;

            stage.draggingSnapPieceIndex = i;
            stage.snapMouseCaptured = true;
            consumedThisFrame = true;
            piece.dragOriginSlotId = piece.currentSlotId;
            BeginDragPiece(piece, mouseWorld);
            break;
        }
    }

    if (stage.draggingSnapPieceIndex >= 0) {
        DragPiece& piece = stage.dragPieces[stage.draggingSnapPieceIndex];
		DistanceTriggerPiece& trigger = stage.distanceTriggerPieces[stage.draggingDistanceTriggerPieceIndex];
        UpdateDraggedPiecePosition(piece, mouseWorld);

        const int candidateSlotIndex = FindNearestSnapSlot(stage, piece);
        if (candidateSlotIndex >= 0) {
            stage.snapSlots[candidateSlotIndex].highlighted = true;

            if (PlacePieceInSlot(stage, stage.draggingSnapPieceIndex, candidateSlotIndex)) {
                stage.draggingSnapPieceIndex = -1;
                ApplySolvedLocks(stage);
            }
        }
        else if (piece.returnOnMiss) {
            StartDistanceTriggerReturn(trigger);
        }
        else {
            piece.state = DragPieceState::FREE;
        }
    }

    if (mouseReleased) stage.snapMouseCaptured = false;

    bool hasAnimation = false;
    for (int i = 0; i < stage.dragPieceCount; i++) {
        const DragPieceState state = stage.dragPieces[i].state;
        if (state == DragPieceState::SNAPPING || state == DragPieceState::RETURNING) {
            hasAnimation = true;
            break;
        }
    }

    return consumedThisFrame || stage.snapMouseCaptured || stage.draggingSnapPieceIndex >= 0 || hasAnimation;
}

bool UpdateDistanceTriggerPieces(
    Stage& stage,
    Camera2D camera,
    float dt,
    bool allowMouseInput
) {
    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        UpdateDragPieceAnimation(
            stage.distanceTriggerPieces[i].drag,
            dt
        );
    }

    const Vector2 mouseWorld =
        GetScreenToWorld2D(
            GetMousePosition(),
            camera
        );

    const bool mousePressed =
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    const bool mouseReleased =
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    bool consumedThisFrame =
        stage.distanceTriggerMouseCaptured;

    if (allowMouseInput &&
        stage.draggingDistanceTriggerPieceIndex < 0 &&
        mousePressed) {

        for (int i = stage.distanceTriggerPieceCount - 1;
            i >= 0;
            i--) {

            DistanceTriggerPiece& trigger =
                stage.distanceTriggerPieces[i];

            DragPiece& piece = trigger.drag;

            if (piece.locked) {
                continue;
            }

            if (piece.state != DragPieceState::FREE) {
                continue;
            }

            if (!CheckCollisionPointRec(mouseWorld, piece.rect)) {
                continue;
            }

            stage.draggingDistanceTriggerPieceIndex = i;
            stage.distanceTriggerMouseCaptured = true;
            consumedThisFrame = true;

            BeginDragPiece(piece, mouseWorld);
            break;
        }
    }

    if (stage.draggingDistanceTriggerPieceIndex >= 0) {
        const int index =
            stage.draggingDistanceTriggerPieceIndex;

        DistanceTriggerPiece& trigger =
            stage.distanceTriggerPieces[index];

        DragPiece& piece = trigger.drag;

        UpdateDraggedPiecePosition(
            piece,
            mouseWorld
        );

        if (mouseReleased) {
            const float distance =
                GetDistanceFromInitialRect(piece);

            if (distance <= trigger.reattachDistance) {
                piece.rect = piece.initialRect;
                piece.state = DragPieceState::FREE;
                trigger.detached = false;
            }
            else if (piece.returnOnMiss) {
                StartDistanceTriggerReturn(trigger);
            }
            else {
                piece.state = DragPieceState::FREE;
            }

            stage.draggingDistanceTriggerPieceIndex = -1;
        }
    }

    if (mouseReleased) {
        stage.distanceTriggerMouseCaptured = false;
    }

    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        UpdateDistanceTriggerState(
            stage.distanceTriggerPieces[i]
        );
    }

    bool hasAnimation = false;

    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        const DragPieceState state =
            stage.distanceTriggerPieces[i].drag.state;

        if (state == DragPieceState::RETURNING) {
            hasAnimation = true;
            break;
        }
    }

    return consumedThisFrame ||
        stage.distanceTriggerMouseCaptured ||
        stage.draggingDistanceTriggerPieceIndex >= 0 ||
        hasAnimation;
}

static void DrawSlot(const SnapSlot& slot) {
    if (!slot.showGuide && !slot.highlighted && slot.effectTimer <= 0.0f) return;

    Color tint = ColorAlpha(WHITE, 0.45f);
    if (slot.highlighted) tint = ColorAlpha(YELLOW, 0.9f);
    if (slot.effectTimer > 0.0f) tint = WHITE;

    if (slot.spriteId != SpriteId::None) {
        SpriteDatabase::DrawSprite(slot.spriteId, slot.rect, slot.rotation, slot.flipX, slot.flipY, tint);
    }
    else {
        const float thickness = slot.highlighted ? 4.0f : 2.0f;
        DrawRectangleLinesEx(slot.rect, thickness, tint);
    }
}

static void DrawPiece(const DragPiece& piece) {
    Rectangle drawRect = piece.rect;

    if (piece.effectTimer > 0.0f) {
        const float progress = 1.0f - piece.effectTimer / 0.18f;
        const float scale = 1.0f + sinf(progress * PI) * 0.12f;
        const Vector2 center = GetRectCenter(piece.rect);
        drawRect.width *= scale;
        drawRect.height *= scale;
        drawRect.x = center.x - drawRect.width * 0.5f;
        drawRect.y = center.y - drawRect.height * 0.5f;
    }

    const Color tint = piece.locked ? ColorAlpha(WHITE, 0.9f) : WHITE;
    SpriteDatabase::DrawSprite(piece.spriteId, drawRect, piece.rotation, piece.flipX, piece.flipY, tint);
}

void DrawSnapPuzzles(const Stage& stage) {
    for (int i = 0; i < stage.snapSlotCount; i++) DrawSlot(stage.snapSlots[i]);

    for (int i = 0; i < stage.dragPieceCount; i++) {
        if (i == stage.draggingSnapPieceIndex) continue;
        DrawPiece(stage.dragPieces[i]);
    }

    if (stage.draggingSnapPieceIndex >= 0 && stage.draggingSnapPieceIndex < stage.dragPieceCount) {
        DrawPiece(stage.dragPieces[stage.draggingSnapPieceIndex]);
    }
}

void DrawDistanceTriggerPieces(const Stage& stage) {
    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        if (i == stage.draggingDistanceTriggerPieceIndex) {
            continue;
        }

        DrawPiece(
            stage.distanceTriggerPieces[i].drag
        );
    }

    const int draggingIndex =
        stage.draggingDistanceTriggerPieceIndex;

    if (draggingIndex >= 0 &&
        draggingIndex < stage.distanceTriggerPieceCount) {

        DrawPiece(
            stage.distanceTriggerPieces[draggingIndex].drag
        );
    }
}

void InitializeSnapPuzzles(Stage& stage) {
    stage.draggingSnapPieceIndex = -1;
    stage.snapMouseCaptured = false;

    for (int i = 0; i < stage.snapSlotCount; i++) {
        SnapSlot& slot = stage.snapSlots[i];
        slot.occupiedPieceId = -1;
        slot.highlighted = false;
        slot.effectTimer = 0.0f;
    }

    for (int i = 0; i < stage.dragPieceCount; i++) {
        DragPiece& piece = stage.dragPieces[i];
        piece.locked = false;
        piece.dragOriginSlotId = -1;
        piece.moveTimer = 0.0f;
        piece.effectTimer = 0.0f;

        const int slotIndex = FindSlotIndex(stage, piece.groupId, piece.currentSlotId);
        if (slotIndex >= 0 && stage.snapSlots[slotIndex].occupiedPieceId < 0) {
            SnapSlot& slot = stage.snapSlots[slotIndex];
            slot.occupiedPieceId = piece.pieceId;
            const Vector2 position = GetPiecePositionInSlot(piece, slot);
            piece.rect.x = position.x;
            piece.rect.y = position.y;
            piece.state = DragPieceState::SNAPPED;
        }
        else {
            piece.currentSlotId = -1;
            piece.rect = piece.initialRect;
            piece.state = DragPieceState::FREE;
        }
    }

    ApplySolvedLocks(stage);

    for (int i = 0; i < stage.dragPieceCount; i++) stage.dragPiecesInit[i] = stage.dragPieces[i];
    for (int i = 0; i < stage.snapSlotCount; i++) stage.snapSlotsInit[i] = stage.snapSlots[i];
}
void InitializeDistanceTriggerPieces(Stage& stage) {
    stage.draggingDistanceTriggerPieceIndex = -1;
    stage.distanceTriggerMouseCaptured = false;

    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        DistanceTriggerPiece& trigger =
            stage.distanceTriggerPieces[i];

        DragPiece& piece = trigger.drag;

        piece.rect = piece.initialRect;
        piece.state = DragPieceState::FREE;
        piece.currentSlotId = -1;
        piece.dragOriginSlotId = -1;
        piece.moveTimer = 0.0f;
        piece.effectTimer = 0.0f;
        piece.locked = false;

        trigger.detached = false;

        stage.distanceTriggerPiecesInit[i] =
            trigger;
    }
}
void ResetSnapPuzzles(Stage& stage) {
    for (int i = 0; i < stage.dragPieceCount; i++) stage.dragPieces[i] = stage.dragPiecesInit[i];
    for (int i = 0; i < stage.snapSlotCount; i++) stage.snapSlots[i] = stage.snapSlotsInit[i];
    stage.draggingSnapPieceIndex = -1;
    stage.snapMouseCaptured = false;
}
void ResetDistanceTriggerPieces(Stage& stage) {
    for (int i = 0; i < stage.distanceTriggerPieceCount; i++) {
        stage.distanceTriggerPieces[i] =
            stage.distanceTriggerPiecesInit[i];
    }

    stage.draggingDistanceTriggerPieceIndex = -1;
    stage.distanceTriggerMouseCaptured = false;
}

bool IsHazardDisabledBySnapPuzzle(const Stage& stage, int hazardIndex) {
    if (hazardIndex < 0 || hazardIndex >= stage.hazardCount) return false;
    const int groupId = stage.hazardDisableSnapGroupIds[hazardIndex];
    return groupId > 0 && IsSnapGroupSolved(stage, groupId);
}