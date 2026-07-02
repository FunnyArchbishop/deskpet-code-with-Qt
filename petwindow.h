/**
 * DeskPet Qt - 宠物窗口
 *
 * 透明置顶无边框窗口，显示koishi并处理交互动画
 */

#ifndef DESKPET_PETWINDOW_H
#define DESKPET_PETWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QMap>
#include <QString>

#include "pet_state.h"
#include "sprite_loader.h"

class PetWindow : public QWidget {
    Q_OBJECT

public:
    explicit PetWindow(QWidget* parent = nullptr);
    ~PetWindow() override;

protected:
    // ---- 事件处理 ----
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onTimerTick();         // 每 50ms 更新逻辑 + 重绘
    void onTriggerAction();     // 右键菜单触发动画
    void onResizeAction();      // 右键菜单调整大小

private:
    // ---- 精灵获取 ----
    SpriteFrame& currentSprite();
    const QPixmap& currentPixmap();

    // ---- 状态更新 ----
    void updateState();
    void updateMovement();
    void clampToScreen();

    // ---- 阴影绘制 ----
    void drawShadow(QPainter& painter);

    // ============================================================
    // 宠物状态
    // ============================================================
    QPoint   m_pos        = {100, 100};  // 窗口位置
    QPoint   m_velocity   = {0, 0};       // 移动速度
    int      m_dirX       = 1;            // 朝向: 1=右, -1=左
    PetState m_state      = PetState::IDLE;
    PetState m_prevState  = PetState::IDLE;
    int      m_animFrame  = 0;

    // ---- 拖拽 ----
    bool     m_dragging   = false;
    QPoint   m_dragOffset;

    // ---- 触发动画 ----
    QString  m_activeTrigger;
    qint64   m_triggerStartTime = 0;

    // ---- 精灵帧 ----
    SpriteFrame m_sfIdle;
    SpriteFrame m_sfBlink;
    SpriteFrame m_sfWalk1;
    SpriteFrame m_sfWalk2;
    SpriteFrame m_sfSit;
    TriggerMap  m_triggers;

    // ---- 计时 ----
    qint64 m_lastAnim        = 0;
    qint64 m_lastStateChange = 0;

    // ---- 定时器 ----
    QTimer* m_timer = nullptr;

    // ---- 可调大小 ----
    int m_petSize = PET_SIZE;
};

#endif // DESKPET_PETWINDOW_H
