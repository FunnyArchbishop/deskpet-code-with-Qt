/**
 * DeskPet Qt - koishi窗口实现
 *
 * 透明置顶窗口 + 精灵动画 + 拖拽 + 右键触发菜单
 * 基于 QPainter 渲染，QTimer 驱动动画循环
 */

#include "petwindow.h"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QScreen>
#include <QApplication>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDir>
#include <QtMath>

// ============================================================
// 构造 / 析构
// ============================================================

PetWindow::PetWindow(QWidget* parent)
    : QWidget(parent)
{
    // ---- 无边框透明置顶窗口 ----
    setWindowFlags(Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint
                   | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_petSize = PET_SIZE;
    setFixedSize(m_petSize, m_petSize);

    // ---- 居中到屏幕 ----
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect avail = screen->availableGeometry();
        m_pos.setX((avail.width()  - m_petSize) / 2);
        m_pos.setY((avail.height() - m_petSize) / 2);
    }
    move(m_pos);

    // ---- 加载精灵 ----
    m_sfIdle  = loadSpriteFromFile(SPRITE_IDLE);
    m_sfBlink = loadSpriteFromFile(SPRITE_BLINK);
    m_sfWalk1 = loadSpriteFromFile(SPRITE_WALK1);
    m_sfWalk2 = loadSpriteFromFile(SPRITE_WALK2);
    m_sfSit   = loadSpriteFromFile(SPRITE_SIT);
    m_triggers = loadTriggerFrames(TRIGGER_DIR);

    // ---- 初始化计时 ----
    m_lastAnim        = QDateTime::currentMSecsSinceEpoch();
    m_lastStateChange = m_lastAnim;

    // ---- 启动定时器 (50ms ≈ 20fps) ----
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PetWindow::onTimerTick);
    m_timer->start(50);

    show();
}

PetWindow::~PetWindow() = default;

// ============================================================
// 精灵获取
// ============================================================

SpriteFrame& PetWindow::currentSprite() {
    switch (m_state) {
    case PetState::SITTING:
        return m_sfSit;
    case PetState::WALKING:
        return (m_animFrame % 2 == 0) ? m_sfWalk1 : m_sfWalk2;
    case PetState::TRIGGER: {
        auto it = m_triggers.find(m_activeTrigger);
        if (it != m_triggers.end()) return it.value();
        return m_sfIdle;
    }
    default: // IDLE
        return (m_animFrame == 1) ? m_sfBlink : m_sfIdle;
    }
}

const QPixmap& PetWindow::currentPixmap() {
    return currentSprite().pixmap;
}

// ============================================================
// 阴影绘制
// ============================================================

void PetWindow::drawShadow(QPainter& painter) {
    painter.save();
    painter.setPen(Qt::NoPen);

    // 阴影参数随窗口大小等比缩放
    double scale = m_petSize / (double)PET_SIZE;
    int sLeft   = (int)(SHADOW_LEFT   * scale);
    int sBottom = (int)(SHADOW_BOTTOM * scale);
    int sRight  = (int)(SHADOW_RIGHT  * scale);
    int sHeight = (int)(SHADOW_HEIGHT * scale);

    QColor shadow(60, 60, 60);
    for (int i = 0; i < 3; i++) {
        int alpha = 80 - i * 15;
        shadow.setAlpha(alpha);
        painter.setBrush(shadow);
        painter.drawEllipse(
            sLeft,
            m_petSize - sBottom + i * 3,
            m_petSize - sLeft - sRight,
            sHeight
        );
    }

    painter.restore();
}

// ============================================================
// 绘制事件
// ============================================================

void PetWindow::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QPixmap& pm = currentPixmap();
    if (pm.isNull()) return;

    // ---- 缩放到窗口尺寸 ----
    QPixmap scaled = pm.scaled(m_petSize, m_petSize,
                               Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);

    // ---- 水平翻转 (朝向) ----
    if (m_dirX < 0) {
        scaled = scaled.transformed(QTransform().scale(-1, 1));
    }

    // ---- 绘制精灵 (透明背景自动处理) ----
    painter.drawPixmap(0, 0, scaled);

    // ---- 绘制阴影 ----
    drawShadow(painter);
}

// ============================================================
// 定时器回调
// ============================================================

void PetWindow::onTimerTick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // ---- 触发动画持续计时 ----
    if (m_state == PetState::TRIGGER
        && (now - m_triggerStartTime) > TRIGGER_DURATION) {
        m_state = m_prevState;
        m_activeTrigger.clear();
    }

    // ---- 状态切换 ----
    if (!m_dragging && m_state != PetState::TRIGGER
        && (now - m_lastStateChange) > STATE_CHANGE_MS) {
        m_lastStateChange = now;
        updateState();
    }

    // ---- 动画帧推进 ----
    if ((now - m_lastAnim) > ANIM_INTERVAL) {
        m_lastAnim = now;
        m_animFrame = (m_animFrame + 1) % 4;
    }

    // ---- 移动 ----
    if (!m_dragging && m_state == PetState::WALKING) {
        updateMovement();
    }

    update();  // 触发重绘
}

// ============================================================
// 状态机
// ============================================================

void PetWindow::updateState() {
    int r = QRandomGenerator::global()->bounded(100);

    if (r < 40) {
        m_state = PetState::WALKING;
        int speed = QRandomGenerator::global()->bounded(
            MOVE_SPEED_MIN, MOVE_SPEED_MAX + 1);
        m_velocity.setX(speed * m_dirX);
        m_velocity.setY(QRandomGenerator::global()->bounded(-1, 2));
    } else if (r < 80) {
        m_state = PetState::SITTING;
        m_velocity = QPoint(0, 0);
    } else {
        m_state = PetState::IDLE;
        m_velocity = QPoint(0, 0);
    }
}

// ============================================================
// 移动 & 屏幕边界检测
// ============================================================

void PetWindow::updateMovement() {
    m_pos += m_velocity;

    // 更新朝向
    if (m_velocity.x() > 0)       m_dirX = 1;
    else if (m_velocity.x() < 0)  m_dirX = -1;

    clampToScreen();
    move(m_pos);
}

void PetWindow::clampToScreen() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect avail = screen->availableGeometry();

    if (m_pos.x() < avail.left()) {
        m_pos.setX(avail.left());
        m_velocity.setX(-m_velocity.x());
        m_dirX = 1;
    }
    if (m_pos.x() + m_petSize > avail.right()) {
        m_pos.setX(avail.right() - m_petSize);
        m_velocity.setX(-m_velocity.x());
        m_dirX = -1;
    }
    if (m_pos.y() < avail.top()) {
        m_pos.setY(avail.top());
        m_velocity.setY(-m_velocity.y());
    }
    if (m_pos.y() + m_petSize > avail.bottom()) {
        m_pos.setY(avail.bottom() - m_petSize);
        m_velocity.setY(-m_velocity.y());
    }
}

// ============================================================
// 鼠标交互 — 拖拽
// ============================================================

void PetWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging   = true;
        m_dragOffset = event->pos();
        if (m_state != PetState::TRIGGER) {
            m_state = PetState::IDLE;
        }
        m_velocity = QPoint(0, 0);
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void PetWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging) {
        m_pos = event->globalPosition().toPoint() - m_dragOffset;
        move(m_pos);
    }
    QWidget::mouseMoveEvent(event);
}

void PetWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

// ============================================================
// 右键菜单 — 触发动画 & 退出
// ============================================================

void PetWindow::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    menu.setStyleSheet(R"(
        QMenu {
            background: #2b2b2b;
            color: #e0e0e0;
            border: 1px solid #555;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 24px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background: #4a6fa5;
        }
        QMenu::separator {
            height: 1px;
            background: #555;
            margin: 4px 8px;
        }
    )");

    // ---- 大小调整子菜单 ----
    QMenu* sizeMenu = menu.addMenu(QString::fromUtf8("\u8c03\u6574\u5927\u5c0f"));
    struct SizeOption { int size; QString label; };
    const SizeOption sizes[] = {
        { 100, QString::fromUtf8("\u2606 \u5c0f (100)") },
        { 150, QString::fromUtf8("\u2606 \u8f83\u5c0f (150)") },
        { 200, QString::fromUtf8("\u2605 \u4e2d (200)") },
        { 300, QString::fromUtf8("\u2606 \u8f83\u5927 (300)") },
        { 400, QString::fromUtf8("\u2606 \u5927 (400)") },
    };
    for (const auto& opt : sizes) {
        QAction* act = sizeMenu->addAction(opt.label);
        act->setData(opt.size);
        act->setCheckable(true);
        if (opt.size == m_petSize) act->setChecked(true);
        connect(act, &QAction::triggered, this, &PetWindow::onResizeAction);
    }

    menu.addSeparator();

    if (!m_triggers.isEmpty()) {
        for (auto it = m_triggers.begin(); it != m_triggers.end(); ++it) {
            QString label = QString::fromUtf8("\u2728 ") + it.key();
            QAction* act = menu.addAction(label);
            act->setData(it.key());
            connect(act, &QAction::triggered, this, &PetWindow::onTriggerAction);
        }
        menu.addSeparator();
    }

    QAction* exitAction = menu.addAction(QString::fromUtf8("\u9000\u51FA DeskPet"));
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    menu.exec(event->globalPos());
}

void PetWindow::onTriggerAction() {
    QAction* act = qobject_cast<QAction*>(sender());
    if (!act) return;

    QString name = act->data().toString();
    if (!m_triggers.contains(name)) return;

    m_prevState         = m_state;
    m_state             = PetState::TRIGGER;
    m_activeTrigger     = name;
    m_triggerStartTime  = QDateTime::currentMSecsSinceEpoch();
    m_velocity          = QPoint(0, 0);
}

void PetWindow::onResizeAction() {
    QAction* act = qobject_cast<QAction*>(sender());
    if (!act) return;

    int newSize = act->data().toInt();
    if (newSize == m_petSize) return;

    // 保持窗口中心不变
    QPoint center = m_pos + QPoint(m_petSize / 2, m_petSize / 2);
    m_petSize = newSize;
    setFixedSize(m_petSize, m_petSize);
    m_pos = center - QPoint(m_petSize / 2, m_petSize / 2);
    move(m_pos);

    // 确保始终置顶
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    show();

    update();
}
