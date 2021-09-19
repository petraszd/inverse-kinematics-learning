#include <QtWidgets>
#include <QtMath>
#include <math.h>
#include "ikwidget.h"

IKWidget::IKWidget(QWidget *parent)
    : QWidget(parent)
{
    m_ikState = IKState::Editing;

    m_selectedBoneIndex = NO_SELECTION;

    m_brushBackground = new QBrush(Qt::white);
    m_brushBoneDefault = new QBrush(Qt::red);
    m_brushBoneSelected = new QBrush(Qt::blue);
    m_brushController = new QBrush(Qt::green);

    m_controllerSelected = false;
    m_controller = new QVector2D();
}

IKWidget::~IKWidget()
{
    delete m_brushBackground;
    delete m_brushBoneDefault;
    delete m_brushBoneSelected;
    delete m_brushController;

    delete m_controller;
}

void IKWidget::editModeToggled(bool)
{
    m_ikState = IKState::Editing;
    repaint();
}

void IKWidget::ikModeToggled(bool)
{
    m_ikState = IKState::Simulating;

    if (m_bones.length() > 0) {
        updateControllerPosition(m_bones.last());
    } else {
        updateControllerPosition(QVector2D(width() / 2.0f, height() / 2.0f));
    }
    repaint();
}

void IKWidget::mousePressEvent(QMouseEvent *event)
{
    QVector2D mousePos(event->pos());

    if (m_ikState == IKState::Editing) {
        mousePressWhenEditing(mousePos);
    } else if (m_ikState == IKState::Simulating) {
        mousePressWhenSimulating(mousePos);
    }
}

void IKWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QVector2D mousePos(event->pos());
    if (m_ikState == IKState::Editing) {
        mouseReleaseWhenEditing(mousePos, event->modifiers() & Qt::Modifier::SHIFT);
    } else if (m_ikState == IKState::Simulating) {
        mouseReleaseWhenSimulating(mousePos);
    }
}

void IKWidget::mouseMoveEvent(QMouseEvent *event)
{
    QVector2D mousePos(event->pos());
    if (m_ikState == IKState::Editing) {
        mouseMoveWhenEditing(mousePos);
    } else if (m_ikState == IKState::Simulating) {
        mouseMoveWhenSimulating(mousePos);
    }

}

void IKWidget::mousePressWhenEditing(QVector2D& mousePos)
{
    for(int i = 0; i < m_bones.length(); ++i) {
        if (m_bones[i].distanceToPoint(mousePos) <= float(BONE_RADIUS)) {
            m_selectedBoneIndex = i;
            repaint();
            break;
        }
    }
}

void IKWidget::mousePressWhenSimulating(QVector2D& mousePos)
{
    if (m_controller->distanceToPoint(mousePos) <= float(CONTROLLER_RADIUS)) {
        m_controllerSelected = true;
    }
}

void IKWidget::mouseReleaseWhenEditing(QVector2D& mousePos, bool isShift)
{
    if (m_selectedBoneIndex == NO_SELECTION) {
        m_bones.append(mousePos);
    } else {
        if (isShift) {
            m_bones.remove(m_selectedBoneIndex);
        }
        m_selectedBoneIndex = NO_SELECTION;
    }

    repaint();
}

void IKWidget::mouseReleaseWhenSimulating(QVector2D& mousePos)
{
    if (m_controllerSelected) {
        updateControllerPosition(mousePos);

        repaint();
    }
    m_controllerSelected = false;
}

void IKWidget::mouseMoveWhenEditing(QVector2D& mousePos)
{
    if (m_selectedBoneIndex != NO_SELECTION) {
        m_bones[m_selectedBoneIndex] = QVector2D(mousePos);
        repaint();
    }
}

void IKWidget::mouseMoveWhenSimulating(QVector2D& mousePos)
{
    if (m_controllerSelected) {
        updateControllerPosition(mousePos);
        repaint();
    }
}

void IKWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::HighQualityAntialiasing, true);

    paintBackground(painter);

    for (int i = 0; i < m_bones.length() - 1; ++i) {
        paintConnection(painter, m_bones[i], m_bones[i + 1]);
    }

    for (int i = 0; i < m_bones.length(); ++i) {
        if (i == m_selectedBoneIndex) {
            paintBone(painter, m_bones[i], *m_brushBoneSelected);
        } else {
            paintBone(painter, m_bones[i], *m_brushBoneDefault);
        }
    }

    if (m_ikState == IKState::Simulating) {
        paintController(painter);
    }
}

void IKWidget::paintBackground(QPainter& painter)
{
    painter.setBrush(*m_brushBackground);
    painter.setPen(Qt::PenStyle::NoPen);
    painter.drawRect(0, 0, width(), height());
}

void IKWidget::paintBone(QPainter& painter, QVector2D& bone, QBrush& brush)
{
    painter.setBrush(brush);
    painter.setPen(Qt::PenStyle::SolidLine);

    painter.drawEllipse(bone.toPointF(), BONE_RADIUS, BONE_RADIUS);
}

void IKWidget::paintConnection(QPainter& painter, QVector2D& a, QVector2D& b)
{
    painter.setPen(Qt::PenStyle::SolidLine);
    painter.setBrush(Qt::BrushStyle::NoBrush);

    QVector2D delta = b - a;
    delta.normalize();

    float angle = atan2f(delta.y(), delta.x()) + qDegreesToRadians(90.0f);
    QVector2D ortho = QVector2D(cos(angle), sin(angle));

    QVector2D aright = a + ortho * CONNECTION_RADIUS + delta * CONNECTION_RADIUS;
    QVector2D aleft = a - ortho * CONNECTION_RADIUS + delta * CONNECTION_RADIUS;

    QPointF points[4] = {
        a.toPointF(),
        aleft.toPointF(),
        b.toPointF(),
        aright.toPointF(),
    };
    painter.drawPolygon(points, 4);
}

void IKWidget::paintController(QPainter& painter) {
    if (m_bones.length() > 1) {
        QVector2D firstBone = m_bones.first();

        painter.setPen(Qt::PenStyle::DotLine);
        painter.drawLine(firstBone.toPointF(), m_controller->toPointF());
    }

    painter.setPen(Qt::PenStyle::NoPen);
    painter.setBrush(*m_brushController);

    painter.drawEllipse(m_controller->toPointF(), CONTROLLER_RADIUS, CONTROLLER_RADIUS);
}

void IKWidget::updateControllerPosition(const QVector2D& newPos)
{
    m_controller->setX(newPos.x());
    m_controller->setY(newPos.y());
    calcInverseKinematics();
}

void IKWidget::calcInverseKinematics()
{
    int nConnections = m_bones.length() - 1;
    if (nConnections == 0) {
        return;
    }

    // Init temporary data
    float *angles = new float[nConnections];
    float *lengths = new float[nConnections];
    for (int i = 0; i < nConnections; ++i) {
        QVector2D delta = m_bones[i + 1] - m_bones[i];

        lengths[i] = delta.length();

        delta.normalize();
        float angle = atan2f(delta.y(), delta.x());
        angles[i] = angle;
    }

    // Calculate
    for (int step = 0; step < STEPS_TO_DESCENT; ++step) {
        if (getDistanceToSimulator(angles, lengths, nConnections) < MAXIMUM_DELTA_TO_STOP) {
            break;
        }

        for (int i = 0; i < nConnections; ++i) {
            float angle = angles[i];

            float initDistance = getDistanceToSimulator(angles, lengths, nConnections);
            angles[i] += SAMPLING_SIZE;
            float newDistance = getDistanceToSimulator(angles, lengths, nConnections);

            float gradient = (newDistance - initDistance) / SAMPLING_SIZE;
            angles[i] = angle - LEARNING_RATE * gradient;
        }
    }

    QVector2D current = m_bones.first();
    for (int i = 0; i < m_bones.length() - 1; ++i) {
        QVector2D position = current + QVector2D(cos(angles[i]), sin(angles[i])) * lengths[i];
        m_bones[i + 1] = position;
        current = position;
    }

    delete[] angles;
    delete[] lengths;
}

float IKWidget::getDistanceToSimulator(float* angles, float* lengths, int nConnections)
{
    QVector2D current = m_bones.first();
    for (int i = 0; i < nConnections; ++i) {
        QVector2D position = current + QVector2D(cos(angles[i]), sin(angles[i])) * lengths[i];
        current = position;
    }
    return current.distanceToPoint(*m_controller);
}
