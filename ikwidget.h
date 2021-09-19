#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtMath>

enum class IKState {
    Editing,
    Simulating,
};

class IKWidget : public QWidget
{
    Q_OBJECT

private:
    constexpr static const qreal CONTROLLER_RADIUS = 25.0;
    constexpr static const qreal BONE_RADIUS = 20.0;
    constexpr static const qreal CONNECTION_RADIUS = 20.0;
    static const int NO_SELECTION = -1;
    static const int STEPS_TO_DESCENT = 1000;
    constexpr static const float LEARNING_RATE = 0.000025f;
    constexpr static const float SAMPLING_SIZE = qDegreesToRadians(0.125f);
    constexpr static const float MAXIMUM_DELTA_TO_STOP = 1.0f;

    IKState m_ikState;

    QVector2D *m_controller;
    bool m_controllerSelected;

    QVector<QVector2D> m_bones;
    int m_selectedBoneIndex;

    QBrush* m_brushBackground;
    QBrush* m_brushBoneDefault;
    QBrush* m_brushBoneSelected;
    QBrush* m_brushController;

public:
    IKWidget(QWidget *parent = nullptr);
    ~IKWidget();

public slots:
    void editModeToggled(bool state);
    void ikModeToggled(bool state);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    void paintBackground(QPainter& painter);
    void paintBone(QPainter& painter, QVector2D& bone, QBrush& brush);
    void paintConnection(QPainter& painter, QVector2D& a, QVector2D& b);
    void paintController(QPainter& painter);

    void mousePressWhenEditing(QVector2D& mousePos);
    void mousePressWhenSimulating(QVector2D& mousePos);

    void mouseReleaseWhenEditing(QVector2D& mousePos, bool isShift);
    void mouseReleaseWhenSimulating(QVector2D& mousePos);

    void mouseMoveWhenEditing(QVector2D& mousePos);
    void mouseMoveWhenSimulating(QVector2D& mousePos);

    void updateControllerPosition(const QVector2D& newPos);
    void calcInverseKinematics();

    float getDistanceToSimulator(float* angles, float* lengths, int nConnections);
};

#endif // WIDGET_H
