/**
 * DeskPet Qt - koishi加载器实现
 *
 * 从图片文件 (PNG, JPG, BMP 等) 加载koishi帧。
 * PNG 格式推荐使用，原生支持透明通道。
 * 用户可以替换 assets/ 目录下的图片来自定义角色。
 */

#include "sprite_loader.h"

#include <QDir>
#include <QFileInfoList>

// ============================================================
// 从图片文件加载koishi帧
// ============================================================
SpriteFrame loadSpriteFromFile(const QString& filePath) {
    SpriteFrame result;
    result.pixmap = QPixmap(filePath);
    return result;
}

// ============================================================
// 从目录加载所有触发帧 (*.png)
// ============================================================
TriggerMap loadTriggerFrames(const QString& dirPath) {
    TriggerMap triggers;

    QDir dir(dirPath);
    if (!dir.exists()) return triggers;

    QFileInfoList files = dir.entryInfoList({"*.png"}, QDir::Files);
    for (const QFileInfo& info : files) {
        QString name = info.completeBaseName();  // 文件名去掉扩展名
        SpriteFrame frame = loadSpriteFromFile(info.absoluteFilePath());

        if (frame.isValid()) {
            triggers.insert(name, frame);
        }
    }

    return triggers;
}
