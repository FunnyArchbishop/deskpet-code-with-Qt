/**
 * DeskPet Qt - koishi状态 & 配置常量
 *
 * 桌面koishi Qt 版 — 状态枚举 & 可调参数
 * 使用图片文件 (PNG) 作为koishi素材，支持透明通道
 */

#ifndef DESKPET_PET_STATE_H
#define DESKPET_PET_STATE_H

#include <QString>

// ============================================================
// 显示尺寸
// ============================================================
constexpr int PET_SIZE       = 200;   // 窗口显示尺寸 (原 512 缩小约 2.5 倍)
constexpr int PET_IMG_SIZE   = 128;   // 素材图片尺寸 (建议使用方形 PNG)

// ============================================================
// 行为参数
// ============================================================
constexpr int ANIM_INTERVAL   = 300;   // 动画帧间隔 (ms)
constexpr int STATE_CHANGE_MS = 5000;  // 状态切换间隔 (ms)
constexpr int MOVE_SPEED_MIN  = 2;     // 最小移动速度
constexpr int MOVE_SPEED_MAX  = 5;     // 最大移动速度
constexpr int TRIGGER_DURATION= 3000;  // 触发动画持续时间 (ms)

// ============================================================
// 阴影参数
// ============================================================
constexpr int SHADOW_LEFT    = 3;
constexpr int SHADOW_BOTTOM  = 9;
constexpr int SHADOW_RIGHT   = 3;
constexpr int SHADOW_HEIGHT  = 3;

// ============================================================
// koishi文件路径 (PNG 图片)
// 用户可替换 assets/ 下的图片自定义角色
// ============================================================
inline const QString SPRITE_IDLE   = QStringLiteral("assets/sprite_idle.png");
inline const QString SPRITE_BLINK  = QStringLiteral("assets/sprite_blink.png");
inline const QString SPRITE_WALK1  = QStringLiteral("assets/sprite_walk1.png");
inline const QString SPRITE_WALK2  = QStringLiteral("assets/sprite_walk2.png");
inline const QString SPRITE_SIT    = QStringLiteral("assets/sprite_sit.png");
inline const QString TRIGGER_DIR   = QStringLiteral("assets/triggers");

// ============================================================
// 宠物状态枚举
// ============================================================
enum class PetState {
    IDLE,
    WALKING,
    SITTING,
    TRIGGER
};

#endif // DESKPET_PET_STATE_H
