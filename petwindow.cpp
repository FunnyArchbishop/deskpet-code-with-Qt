/**
 * DeskPet Qt - 宠物窗口实现
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

    // ---- 加载精灵 (.gif 优先, .png 回退) ----
    m_sfIdle  = loadSpriteByBaseName(SPRITE_IDLE);
    m_sfBlink = loadSpriteByBaseName(SPRITE_BLINK);
    m_sfWalk1 = loadSpriteByBaseName(SPRITE_WALK1);
    m_sfWalk2 = loadSpriteByBaseName(SPRITE_WALK2);
    m_sfSit   = loadSpriteByBaseName(SPRITE_SIT);
    m_triggers = loadTriggerFrames(TRIGGER_DIR);

    // ---- 加载 GIF 模式精灵 ----
    m_sfStand = loadSpriteByBaseName(SPRITE_STAND);
    m_sfCreep = loadSpriteByBaseName(SPRITE_CREEP);

    m_lastAnim          = QDateTime::currentMSecsSinceEpoch();
    m_lastStateChange   = m_lastAnim;
    m_lastAffectionInc  = m_lastAnim;

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
    // ---- GIF 模式: stand.gif / creep.gif ----
    if (m_mode == PetMode::GIF_MODE) {
        if (m_state == PetState::TRIGGER) {
            auto it = m_triggers.find(m_activeTrigger);
            if (it != m_triggers.end()) return it.value();
        }
        return (m_state == PetState::WALKING) ? m_sfCreep : m_sfStand;
    }

    // ---- PNG 精灵模式 ----
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

QPixmap PetWindow::currentPixmap() {
    SpriteFrame& sf = currentSprite();
    sf.startGif();  // 确保 GIF 在播放
    return sf.currentFrame();
}

void PetWindow::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPixmap pm = currentPixmap();
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

    // ---- 对话气泡 ----
    if (m_showBubble) {
        drawBubble(painter);
    }
}

// ============================================================
// 定时器回调
// ============================================================

void PetWindow::onTimerTick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // ---- affection increment (+5 per minute) ----
    if (m_affection < AFFECTION_MAX
        && (now - m_lastAffectionInc) >= (60000 / AFFECTION_PER_MINUTE)) {
        m_lastAffectionInc = now;
        m_affection = qMin(m_affection + 1, AFFECTION_MAX);
    }

    // ---- bubble timeout ----
    if (m_showBubble && (now - m_bubbleShowTime) > BUBBLE_DURATION_MS) {
        m_showBubble = false;
        setFixedSize(m_petSize, m_petSize);
        m_expandedW = 0;
    }

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
    int w = width();
    int h = height();

    if (m_pos.x() < avail.left()) {
        m_pos.setX(avail.left());
        m_velocity.setX(-m_velocity.x());
        m_dirX = 1;
    }
    if (m_pos.x() + w > avail.right()) {
        m_pos.setX(avail.right() - w);
        m_velocity.setX(-m_velocity.x());
        m_dirX = -1;
    }
    if (m_pos.y() < avail.top()) {
        m_pos.setY(avail.top());
        m_velocity.setY(-m_velocity.y());
    }
    if (m_pos.y() + h > avail.bottom()) {
        m_pos.setY(avail.bottom() - h);
        m_velocity.setY(-m_velocity.y());
    }
}

// ============================================================
// 鼠标交互 - 拖拽
// ============================================================

void PetWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging   = true;
        m_dragOffset = event->pos();
        m_clickPos   = event->pos();
        m_wasClick   = true;  // assume click until movement proves otherwise
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
        // detect drag: if moved > 5px, it's a drag not a click
        if ((event->pos() - m_clickPos).manhattanLength() > 5) {
            m_wasClick = false;
        }
        m_pos = event->globalPosition().toPoint() - m_dragOffset;
        move(m_pos);
    }
    QWidget::mouseMoveEvent(event);
}

void PetWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // ---- click (not drag): show affection ----
        if (m_wasClick && m_dragging) {
            m_bubbleText      = getAffectionText();
            m_bubbleShowTime  = QDateTime::currentMSecsSinceEpoch();
            m_showBubble      = true;
            calcBubbleSize();

            // expand downward only — pet stays in place
            int needH = m_petSize + m_bubbleH + 14;
            m_expandedW = m_petSize;
            setFixedSize(m_petSize, needH);
            // no position change — pet doesn't move
        }
        m_dragging = false;
        m_wasClick = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

// ============================================================
// 右键菜单 - 触发动画 & 退出
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

    // ---- 模式切换 ----
    QMenu* modeMenu = menu.addMenu(QString::fromUtf8("\u6a21\u5f0f\u5207\u6362"));
    QAction* spriteMode = modeMenu->addAction(QString::fromUtf8("\u56fe\u7247\u6a21\u5f0f (PNG\u591a\u5e27)"));
    QAction* gifMode    = modeMenu->addAction(QString::fromUtf8("GIF\u6a21\u5f0f (stand+creep)"));
    spriteMode->setCheckable(true);
    gifMode->setCheckable(true);
    spriteMode->setChecked(m_mode == PetMode::SPRITE_MODE);
    gifMode->setChecked(m_mode == PetMode::GIF_MODE);
    spriteMode->setData((int)PetMode::SPRITE_MODE);
    gifMode->setData((int)PetMode::GIF_MODE);
    connect(spriteMode, &QAction::triggered, this, &PetWindow::onSwitchMode);
    connect(gifMode,    &QAction::triggered, this, &PetWindow::onSwitchMode);


    menu.addSeparator();

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
            QString displayName = triggerDisplayName(it.key());
            QString label = QString::fromUtf8("\u2728 ") + displayName;
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

void PetWindow::onSwitchMode() {
    QAction* act = qobject_cast<QAction*>(sender());
    if (!act) return;

    PetMode newMode = (PetMode)act->data().toInt();
    if (newMode == m_mode) return;

    m_mode = newMode;

    if (m_mode == PetMode::GIF_MODE) {
        m_sfStand.startGif();
        m_sfCreep.startGif();
    }

    update();
}

// ============================================================
// 对话气泡 & 好感度
// ============================================================

QString PetWindow::getAffectionText() const {
    QString dialogue = affectionDialogue(m_affection);
    if (!dialogue.isEmpty()) {
        return dialogue;
    }
    return QString::fromUtf8("\u2665 \u597d\u611f\u5ea6: ") + QString::number(m_affection) + QStringLiteral(" / 100");
}

void PetWindow::calcBubbleSize() {
    QFont font("Microsoft YaHei", 11);
    QFontMetrics fm(font);

    // constrain to pet width, word-wrap for long text
    int maxW = m_petSize - 12;  // leave margin for border
    int padX = 12, padY = 10;

    QRect textRect = fm.boundingRect(QRect(0, 0, maxW - padX * 2, 2000),
                                     Qt::AlignLeft | Qt::TextWordWrap,
                                     m_bubbleText);
    m_bubbleW = qMin(textRect.width() + padX * 2, maxW);
    m_bubbleH = textRect.height() + padY * 2;
    m_bubbleW = qMax(m_bubbleW, 60);
}

void PetWindow::drawBubble(QPainter& painter) {
    if (m_bubbleText.isEmpty() || m_bubbleW <= 0) return;

    painter.save();

    int padX = 12, padY = 10;

    // bubble below pet, centered horizontally within pet width
    int bx = (m_petSize - m_bubbleW) / 2;
    int by = m_petSize + 4;

    // background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 235));
    painter.drawRoundedRect(bx, by, m_bubbleW, m_bubbleH, 10, 10);

    // border
    painter.setPen(QPen(QColor(200, 180, 220), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bx, by, m_bubbleW, m_bubbleH, 10, 10);

    // text
    QFont font("Microsoft YaHei", 11);
    painter.setFont(font);
    painter.setPen(QColor(80, 40, 80));
    painter.drawText(bx + padX, by + padY, m_bubbleW - padX * 2,
                     m_bubbleH - padY * 2,
                     Qt::AlignLeft | Qt::TextWordWrap, m_bubbleText);

    // triangle pointer (pointing up toward pet)
    QPointF pts[3] = {
        QPointF(m_petSize / 2.0, by),
        QPointF(m_petSize / 2.0 - 5, by - 6),
        QPointF(m_petSize / 2.0 + 5, by - 6)
    };
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 235));
    painter.drawPolygon(pts, 3);

    painter.restore();
}
