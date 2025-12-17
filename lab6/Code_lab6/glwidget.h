#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QPoint>
#include <QQuaternion>
#include <vector>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    void setScale(float scale);
    void setTranslation(float x, float y, float z);
    void resetTransform();
    void setProjectionType(int type);

    QMatrix4x4 getTransformationMatrix() const;
    QMatrix4x4 getScaleMatrix() const;
    QMatrix4x4 getTranslationMatrix() const;

    float getScale() const { return currentScale; }
    QVector3D getTranslation() const { return currentTranslation; }
    QVector3D getRotationAngles() const {
        return QVector3D(accumulatedRotationX, accumulatedRotationY, accumulatedRotationZ);
    }
    QQuaternion getRotationQuaternion() const {
        return currentRotation;
    }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    float accumulatedRotationX;
    float accumulatedRotationY;
    float accumulatedRotationZ;
    struct Vertex {
        QVector3D original;
        QVector3D transformed;
    };

    struct Cuboid {
        std::vector<Vertex> vertices;
        QVector3D center;
    };

    std::vector<Cuboid> originalCuboids;
    std::vector<Cuboid> currentCuboids;

    float currentScale;
    QVector3D currentTranslation;
    QQuaternion currentRotation;
    QMatrix4x4 rotationMatrix;

    QVector3D modelCenter;

    void applyAllTransformations();

    void initLetterHGeometry();
    void calculateCuboidVertices(Cuboid& cuboid, const QVector3D& center,
                                 float width, float height, float depth);
    void updateModelCenter();

    void drawCoordinateSystem();
    void drawLetterH();
    void drawCuboid(const Cuboid& cuboid);
    void draw3DGrid();
    void draw2DGrid();
    void draw3DAxes();
    void draw2DAxes();

    QPoint lastMousePos;
    bool isLeftMousePressed;
    bool isRightMousePressed;
    bool isMiddleMousePressed;
    float cameraYaw;
    float cameraPitch;
    float cameraDistance;
    QVector3D cameraPosition;
    QVector3D cameraTarget;
    QVector3D cameraUp;
    float cameraZoomSpeed;
    float cameraRotationSpeed;

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
    int projectionType;

    void updateCamera();

signals:
    void transformationChanged();
    void rotationAnglesChanged(float x, float y, float z);
    void cameraChanged();
};

#endif
