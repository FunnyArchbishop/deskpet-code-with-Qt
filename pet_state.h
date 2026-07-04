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
// 精灵文件基础路径 (不带扩展名, 自动 .gif 优先 → .png 回退)
// 用户可替换 assets/ 下的 .gif 或 .png 文件自定义角色
// ============================================================
inline const QString SPRITE_IDLE   = QStringLiteral("assets/sprite_idle");
inline const QString SPRITE_BLINK  = QStringLiteral("assets/sprite_blink");
inline const QString SPRITE_WALK1  = QStringLiteral("assets/sprite_walk1");
inline const QString SPRITE_WALK2  = QStringLiteral("assets/sprite_walk2");
inline const QString SPRITE_SIT    = QStringLiteral("assets/sprite_sit");
inline const QString TRIGGER_DIR   = QStringLiteral("assets/triggers");

// ============================================================
// GIF 模式精灵路径
// ============================================================
inline const QString SPRITE_STAND  = QStringLiteral("assets/stand");
inline const QString SPRITE_CREEP  = QStringLiteral("assets/creep");

// ============================================================
// Trigger 显示名称映射
// ============================================================
inline QString triggerDisplayName(const QString& internalName) {
    if (internalName == "koishi") return QStringLiteral("koishi_cute");
    return internalName;
}

// ============================================================
// 好感度阈值对话 (20/40/60/80/100)
// ============================================================
constexpr int AFFECTION_PER_MINUTE = 5;   // 每分钟增加好感度
constexpr int AFFECTION_MAX        = 100;  // 好感度上限
constexpr int BUBBLE_DURATION_MS   = 3000; // 气泡显示时间 (ms)

inline QString affectionDialogue(int affection) {
    if (affection >= 100) return QStringLiteral("\u5982\u679c\u662f\u4f60\u7684\u8bdd\u2026\u4e00\u5b9a\u4e0d\u4f1a\u5fd8\u8bb0\u6211\u5427\u2026\u2665 \u59d0\u59d0\u2026\u6211\u597d\u50cf\u2026\u6709\u4e86\u91cd\u8981\u7684\u4eba\u4e86\u2026");
    if (affection >= 80)  return QStringLiteral("\u660e\u660e\u6211\u53ea\u662f\u8def\u8fb9\u7684\u5c0f\u77f3\u5b50\u2026\u4e3a\u4ec0\u4e48\u4f60\u603b\u80fd\u627e\u5230\u6211\u5462\uff1f\u4f60\u2026\u771f\u662f\u4e2a\u5947\u602a\u7684\u4eba\u5462\u2026");
    if (affection >= 60)  return QStringLiteral("\u6211\u628a\u7b2c\u4e09\u53ea\u773c\u95ed\u4e0a\u4e86\u2026\u56e0\u4e3a\u4e0d\u60f3\u88ab\u8ba8\u538c\u3002\u4f46\u662f\u2026\u59d0\u59d0\u7ed9\u4e86\u6211\u5ba0\u7269\uff0c\u8bf4\u8fd9\u6837\u4f1a\u6162\u6162\u597d\u8d77\u6765\u7684\u2026");
    if (affection >= 40)  return QStringLiteral("\u4f60\u2026\u7279\u610f\u6765\u627e\u6211\u7684\u5417\uff1f\u771f\u662f\u4e2a\u6709\u8da3\u7684\u4eba\u5462\u3002\u6211\u53ea\u662f\u6f2b\u65e0\u76ee\u7684\u5730\u5230\u5904\u95f2\u901b\u800c\u5df2\u2026");
    if (affection >= 20)  return QStringLiteral("\u554a\u2026\uff1f\u4f60\u2026\u80fd\u770b\u89c1\u6211\u5417\uff1f\u660e\u660e\u6211\u5e94\u8be5\u50cf\u8def\u8fb9\u7684\u5c0f\u77f3\u5b50\u4e00\u6837\u2026\u4e0d\u4f1a\u88ab\u4eba\u6ce8\u610f\u5230\u7684\u2026");
    return QStringLiteral("\u2026\u2026\uff1f\u2026\u2026\u2026\u2026");
}

// ============================================================
// 显示模式枚举
// ============================================================
enum class PetMode {
    SPRITE_MODE,  // 多帧 PNG 精灵模式
    GIF_MODE      // stand.gif + creep.gif 模式
};

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
