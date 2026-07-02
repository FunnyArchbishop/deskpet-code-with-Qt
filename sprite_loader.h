/**
 * DeskPet Qt - koishi加载器接口
 *
 * 从图片文件 (PNG 等) 加载koishi帧为 QPixmap，供 QPainter 高效渲染。
 * 支持透明通道，用户可替换任意图片作为角色素材。
 */

#ifndef DESKPET_SPRITE_LOADER_H
#define DESKPET_SPRITE_LOADER_H

#include <QPixmap>
#include <QString>
#include <QMap>

// ============================================================
// koishi帧结构 (单一 QPixmap)
// ============================================================
struct SpriteFrame {
    QPixmap pixmap;

    bool isValid() const { return !pixmap.isNull(); }
};

// ============================================================
// 触发帧映射: 触发名 → koishi帧
// ============================================================
using TriggerMap = QMap<QString, SpriteFrame>;

// ============================================================
// 加载函数
// ============================================================

/** 从图片文件加载koishi帧 (PNG / JPG / BMP 等) */
SpriteFrame loadSpriteFromFile(const QString& filePath);

/** 从目录加载所有触发帧 (*.png) */
TriggerMap loadTriggerFrames(const QString& dirPath);

#endif // DESKPET_SPRITE_LOADER_H
