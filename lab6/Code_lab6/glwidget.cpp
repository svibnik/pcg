#include "glwidget.h"
#include <math.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , accumulatedRotationX(0.0f)
    , accumulatedRotationY(0.0f)
    , accumulatedRotationZ(0.0f)
    , currentScale(1.0f)
    , currentTranslation(0.0f, 0.0f, 0.0f)
    , isLeftMousePressed(false)
    , isRightMousePressed(false)
    , isMiddleMousePressed(false)
    , cameraYaw(-90.0f)
    , cameraPitch(0.0f)
    , cameraDistance(8.0f)
    , cameraTarget(0.0f, 0.0f, 0.0f)
    , cameraUp(0.0f, 1.0f, 0.0f)
    , cameraZoomSpeed(0.5f)
    , cameraRotationSpeed(0.5f)
    , projectionType(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    initLetterHGeometry();
    currentCuboids = originalCuboids;
    updateModelCenter();

    currentRotation = QQuaternion();
    rotationMatrix.setToIdentity();

    applyAllTransformations();
    updateCamera();
}

GLWidget::~GLWidget()
{
    makeCurrent();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);
    glPointSize(10.0f);
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectionMatrix.setToIdentity();
    float aspect = float(width()) / float(height());

    if (projectionType == 0) {
        float fov = 45.0f;
        float zNear = 0.1f;
        float zFar = 100.0f;
        projectionMatrix.perspective(fov, aspect, zNear, zFar);
        updateCamera();
        viewMatrix.setToIdentity();
        viewMatrix.lookAt(cameraPosition, cameraTarget, cameraUp);
    } else {
        float orthoSize = 10.0f;
        float orthoDepth = 100.0f;

        switch (projectionType) {
        case 1:
            projectionMatrix.ortho(-orthoSize * aspect, orthoSize * aspect,
                                   -orthoSize, orthoSize, -orthoDepth, orthoDepth);
            viewMatrix.setToIdentity();
            viewMatrix.lookAt(QVector3D(0.0f, 0.0f, 10.0f),
                              QVector3D(0.0f, 0.0f, 0.0f),
                              QVector3D(0.0f, 1.0f, 0.0f));
            break;
        case 2:
            projectionMatrix.ortho(-orthoSize * aspect, orthoSize * aspect,
                                   -orthoSize, orthoSize, -orthoDepth, orthoDepth);
            viewMatrix.setToIdentity();
            viewMatrix.lookAt(QVector3D(0.0f, 10.0f, 0.0f),
                              QVector3D(0.0f, 0.0f, 0.0f),
                              QVector3D(0.0f, 0.0f, 1.0f));
            break;
        case 3:
            projectionMatrix.ortho(-orthoSize * aspect, orthoSize * aspect,
                                   -orthoSize, orthoSize, -orthoDepth, orthoDepth);
            viewMatrix.setToIdentity();
            viewMatrix.lookAt(QVector3D(10.0f, 0.0f, 0.0f),
                              QVector3D(0.0f, 0.0f, 0.0f),
                              QVector3D(0.0f, 1.0f, 0.0f));
            break;
        }
    }

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projectionMatrix.constData());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix.constData());

    drawCoordinateSystem();
    drawLetterH();
}

void GLWidget::rotateX(float angle)
{
    if (qAbs(angle) < 0.001f) return;

    accumulatedRotationX += angle;

    while (accumulatedRotationX > 180.0f) accumulatedRotationX -= 360.0f;
    while (accumulatedRotationX < -180.0f) accumulatedRotationX += 360.0f;

    QQuaternion localRot = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), angle);
    currentRotation = currentRotation * localRot;
    rotationMatrix = QMatrix4x4(currentRotation.toRotationMatrix());

    applyAllTransformations();
    update();

    emit transformationChanged();
    emit rotationAnglesChanged(accumulatedRotationX, accumulatedRotationY, accumulatedRotationZ);
}

void GLWidget::rotateY(float angle)
{
    if (qAbs(angle) < 0.001f) return;

    accumulatedRotationY += angle;

    while (accumulatedRotationY > 180.0f) accumulatedRotationY -= 360.0f;
    while (accumulatedRotationY < -180.0f) accumulatedRotationY += 360.0f;

    QQuaternion localRot = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), angle);
    currentRotation = currentRotation * localRot;
    rotationMatrix = QMatrix4x4(currentRotation.toRotationMatrix());

    applyAllTransformations();
    update();

    emit transformationChanged();
    emit rotationAnglesChanged(accumulatedRotationX, accumulatedRotationY, accumulatedRotationZ);
}

void GLWidget::rotateZ(float angle)
{
    if (qAbs(angle) < 0.001f) return;

    accumulatedRotationZ += angle;

    while (accumulatedRotationZ > 180.0f) accumulatedRotationZ -= 360.0f;
    while (accumulatedRotationZ < -180.0f) accumulatedRotationZ += 360.0f;

    QQuaternion localRot = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), angle);
    currentRotation = currentRotation * localRot;
    rotationMatrix = QMatrix4x4(currentRotation.toRotationMatrix());

    applyAllTransformations();
    update();

    emit transformationChanged();
    emit rotationAnglesChanged(accumulatedRotationX, accumulatedRotationY, accumulatedRotationZ);
}

void GLWidget::setScale(float scale)
{
    currentScale = scale;
    applyAllTransformations();
    update();
    emit transformationChanged();
}

void GLWidget::setTranslation(float x, float y, float z)
{
    currentTranslation = QVector3D(x, y, z);
    applyAllTransformations();
    update();
    emit transformationChanged();
}

void GLWidget::applyAllTransformations()
{
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(currentTranslation);
    transform = transform * rotationMatrix;
    transform.scale(currentScale);

    currentCuboids = originalCuboids;

    for (size_t i = 0; i < currentCuboids.size(); i++) {
        Cuboid& cuboid = currentCuboids[i];

        cuboid.center = transform * originalCuboids[i].center;

        for (size_t j = 0; j < cuboid.vertices.size(); j++) {
            cuboid.vertices[j].transformed = transform * cuboid.vertices[j].original;
        }
    }
}

void GLWidget::resetTransform()
{
    currentScale = 1.0f;
    currentTranslation = QVector3D(0.0f, 0.0f, 0.0f);
    currentRotation = QQuaternion();
    rotationMatrix.setToIdentity();

    accumulatedRotationX = 0.0f;
    accumulatedRotationY = 0.0f;
    accumulatedRotationZ = 0.0f;

    applyAllTransformations();

    cameraYaw = -90.0f;
    cameraPitch = 0.0f;
    cameraDistance = 8.0f;
    cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    cameraUp = QVector3D(0.0f, 1.0f, 0.0f);
    updateCamera();

    update();

    emit transformationChanged();
    emit rotationAnglesChanged(0.0f, 0.0f, 0.0f);
    emit cameraChanged();
}

void GLWidget::initLetterHGeometry()
{
    originalCuboids.clear();

    float columnWidth = 0.4f;
    float columnHeight = 3.0f;
    float columnDepth = 0.6f;

    float beamWidth = 2.0f;
    float beamHeight = 0.4f;
    float beamDepth = 0.6f;

    Cuboid leftColumn;
    QVector3D leftCenter(-1.2f, 0.0f, 0.0f);
    leftColumn.center = leftCenter;
    calculateCuboidVertices(leftColumn, leftCenter, columnWidth, columnHeight, columnDepth);
    originalCuboids.push_back(leftColumn);

    Cuboid rightColumn;
    QVector3D rightCenter(1.2f, 0.0f, 0.0f);
    rightColumn.center = rightCenter;
    calculateCuboidVertices(rightColumn, rightCenter, columnWidth, columnHeight, columnDepth);
    originalCuboids.push_back(rightColumn);

    Cuboid beam;
    QVector3D beamCenter(0.0f, 0.0f, 0.0f);
    beam.center = beamCenter;
    calculateCuboidVertices(beam, beamCenter, beamWidth, beamHeight, beamDepth);
    originalCuboids.push_back(beam);

    updateModelCenter();
}

void GLWidget::calculateCuboidVertices(Cuboid& cuboid, const QVector3D& center,
                                       float width, float height, float depth)
{
    cuboid.vertices.resize(8);

    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    float halfD = depth / 2.0f;

    cuboid.vertices[0].original = QVector3D(center.x() - halfW, center.y() - halfH, center.z() + halfD);
    cuboid.vertices[1].original = QVector3D(center.x() + halfW, center.y() - halfH, center.z() + halfD);
    cuboid.vertices[2].original = QVector3D(center.x() + halfW, center.y() + halfH, center.z() + halfD);
    cuboid.vertices[3].original = QVector3D(center.x() - halfW, center.y() + halfH, center.z() + halfD);
    cuboid.vertices[4].original = QVector3D(center.x() - halfW, center.y() - halfH, center.z() - halfD);
    cuboid.vertices[5].original = QVector3D(center.x() + halfW, center.y() - halfH, center.z() - halfD);
    cuboid.vertices[6].original = QVector3D(center.x() + halfW, center.y() + halfH, center.z() - halfD);
    cuboid.vertices[7].original = QVector3D(center.x() - halfW, center.y() + halfH, center.z() - halfD);

    for (auto& vertex : cuboid.vertices) {
        vertex.transformed = vertex.original;
    }
}

void GLWidget::updateModelCenter()
{
    modelCenter = QVector3D(0.0f, 0.0f, 0.0f);
}

void GLWidget::updateCamera()
{
    cameraPosition.setX(cameraTarget.x() + cameraDistance * cos(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
    cameraPosition.setY(cameraTarget.y() + cameraDistance * sin(qDegreesToRadians(cameraPitch)));
    cameraPosition.setZ(cameraTarget.z() + cameraDistance * sin(qDegreesToRadians(cameraYaw)) * cos(qDegreesToRadians(cameraPitch)));
}

QMatrix4x4 GLWidget::getTransformationMatrix() const
{
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(currentTranslation);
    transform = transform * rotationMatrix;
    transform.scale(currentScale);
    return transform;
}

QMatrix4x4 GLWidget::getScaleMatrix() const
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.scale(currentScale);
    return m;
}

QMatrix4x4 GLWidget::getTranslationMatrix() const
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.translate(currentTranslation);
    return m;
}

void GLWidget::setProjectionType(int type)
{
    projectionType = type;
    if (type == 0) {
        updateCamera();
    }
    update();
}

void GLWidget::drawLetterH()
{
    for (const auto& cuboid : currentCuboids) {
        drawCuboid(cuboid);
    }
}

void GLWidget::drawCuboid(const Cuboid& cuboid)
{
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.8f);

    glBegin(GL_LINES);
    glVertex3f(cuboid.vertices[0].transformed.x(), cuboid.vertices[0].transformed.y(), cuboid.vertices[0].transformed.z());
    glVertex3f(cuboid.vertices[1].transformed.x(), cuboid.vertices[1].transformed.y(), cuboid.vertices[1].transformed.z());

    glVertex3f(cuboid.vertices[1].transformed.x(), cuboid.vertices[1].transformed.y(), cuboid.vertices[1].transformed.z());
    glVertex3f(cuboid.vertices[2].transformed.x(), cuboid.vertices[2].transformed.y(), cuboid.vertices[2].transformed.z());

    glVertex3f(cuboid.vertices[2].transformed.x(), cuboid.vertices[2].transformed.y(), cuboid.vertices[2].transformed.z());
    glVertex3f(cuboid.vertices[3].transformed.x(), cuboid.vertices[3].transformed.y(), cuboid.vertices[3].transformed.z());

    glVertex3f(cuboid.vertices[3].transformed.x(), cuboid.vertices[3].transformed.y(), cuboid.vertices[3].transformed.z());
    glVertex3f(cuboid.vertices[0].transformed.x(), cuboid.vertices[0].transformed.y(), cuboid.vertices[0].transformed.z());

    glVertex3f(cuboid.vertices[4].transformed.x(), cuboid.vertices[4].transformed.y(), cuboid.vertices[4].transformed.z());
    glVertex3f(cuboid.vertices[5].transformed.x(), cuboid.vertices[5].transformed.y(), cuboid.vertices[5].transformed.z());

    glVertex3f(cuboid.vertices[5].transformed.x(), cuboid.vertices[5].transformed.y(), cuboid.vertices[5].transformed.z());
    glVertex3f(cuboid.vertices[6].transformed.x(), cuboid.vertices[6].transformed.y(), cuboid.vertices[6].transformed.z());

    glVertex3f(cuboid.vertices[6].transformed.x(), cuboid.vertices[6].transformed.y(), cuboid.vertices[6].transformed.z());
    glVertex3f(cuboid.vertices[7].transformed.x(), cuboid.vertices[7].transformed.y(), cuboid.vertices[7].transformed.z());

    glVertex3f(cuboid.vertices[7].transformed.x(), cuboid.vertices[7].transformed.y(), cuboid.vertices[7].transformed.z());
    glVertex3f(cuboid.vertices[4].transformed.x(), cuboid.vertices[4].transformed.y(), cuboid.vertices[4].transformed.z());

    glVertex3f(cuboid.vertices[0].transformed.x(), cuboid.vertices[0].transformed.y(), cuboid.vertices[0].transformed.z());
    glVertex3f(cuboid.vertices[4].transformed.x(), cuboid.vertices[4].transformed.y(), cuboid.vertices[4].transformed.z());

    glVertex3f(cuboid.vertices[1].transformed.x(), cuboid.vertices[1].transformed.y(), cuboid.vertices[1].transformed.z());
    glVertex3f(cuboid.vertices[5].transformed.x(), cuboid.vertices[5].transformed.y(), cuboid.vertices[5].transformed.z());

    glVertex3f(cuboid.vertices[2].transformed.x(), cuboid.vertices[2].transformed.y(), cuboid.vertices[2].transformed.z());
    glVertex3f(cuboid.vertices[6].transformed.x(), cuboid.vertices[6].transformed.y(), cuboid.vertices[6].transformed.z());

    glVertex3f(cuboid.vertices[3].transformed.x(), cuboid.vertices[3].transformed.y(), cuboid.vertices[3].transformed.z());
    glVertex3f(cuboid.vertices[7].transformed.x(), cuboid.vertices[7].transformed.y(), cuboid.vertices[7].transformed.z());
    glEnd();

    glPointSize(12.0f);
    glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_POINTS);
    for (const auto& vertex : cuboid.vertices) {
        glVertex3f(vertex.transformed.x(), vertex.transformed.y(), vertex.transformed.z());
    }
    glEnd();

    glLineWidth(2.0f);
    glPointSize(10.0f);
}

void GLWidget::drawCoordinateSystem()
{
    glPushMatrix();
    glLoadMatrixf(viewMatrix.constData());

    if (projectionType == 0) {
        draw3DAxes();
        draw3DGrid();
    } else {
        draw2DGrid();
        draw2DAxes();
    }

    glPopMatrix();
}

void GLWidget::draw3DAxes()
{
    glLineWidth(3.5f);

    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);
    glVertex3f(4.7f, 0.2f, 0.0f);
    glVertex3f(4.7f, -0.2f, 0.0f);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.8f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.8f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glVertex3f(0.2f, 4.7f, 0.0f);
    glVertex3f(-0.2f, 4.7f, 0.0f);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(0.8f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.8f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);
    glVertex3f(0.2f, 0.0f, 4.7f);
    glVertex3f(-0.2f, 0.0f, 4.7f);
    glEnd();
}

void GLWidget::draw3DGrid()
{
    glLineWidth(1.0f);
    glColor4f(0.7f, 0.7f, 0.7f, 0.3f);

    glBegin(GL_LINES);

    float gridSize = 5.0f;
    float gridStep = 0.5f;

    for (float x = -gridSize; x <= gridSize; x += gridStep) {
        glVertex3f(x, 0.0f, -gridSize);
        glVertex3f(x, 0.0f, gridSize);
    }
    for (float z = -gridSize; z <= gridSize; z += gridStep) {
        glVertex3f(-gridSize, 0.0f, z);
        glVertex3f(gridSize, 0.0f, z);
    }

    for (float x = -gridSize; x <= gridSize; x += gridStep) {
        glVertex3f(x, -gridSize, 0.0f);
        glVertex3f(x, gridSize, 0.0f);
    }
    for (float y = -gridSize; y <= gridSize; y += gridStep) {
        glVertex3f(-gridSize, y, 0.0f);
        glVertex3f(gridSize, y, 0.0f);
    }

    for (float y = -gridSize; y <= gridSize; y += gridStep) {
        glVertex3f(0.0f, y, -gridSize);
        glVertex3f(0.0f, y, gridSize);
    }
    for (float z = -gridSize; z <= gridSize; z += gridStep) {
        glVertex3f(0.0f, -gridSize, z);
        glVertex3f(0.0f, gridSize, z);
    }

    glEnd();

    glPointSize(5.0f);
    glBegin(GL_POINTS);

    glColor3f(1.0f, 0.0f, 0.0f);
    for (int i = -10; i <= 10; i++) {
        if (i != 0) glVertex3f(i * 0.5f, 0.0f, 0.0f);
    }

    glColor3f(0.0f, 0.8f, 0.0f);
    for (int i = -10; i <= 10; i++) {
        if (i != 0) glVertex3f(0.0f, i * 0.5f, 0.0f);
    }

    glColor3f(0.0f, 0.0f, 1.0f);
    for (int i = -10; i <= 10; i++) {
        if (i != 0) glVertex3f(0.0f, 0.0f, i * 0.5f);
    }

    glEnd();
}

void GLWidget::draw2DGrid()
{
    glDisable(GL_DEPTH_TEST);

    glLineWidth(1.0f);
    glColor4f(0.7f, 0.7f, 0.7f, 0.3f);

    float gridSize = 8.0f;
    float gridStep = 0.5f;

    glBegin(GL_LINES);

    switch (projectionType) {
    case 1:
        for (float x = -gridSize; x <= gridSize; x += gridStep) {
            glVertex3f(x, -gridSize, 0.0f);
            glVertex3f(x, gridSize, 0.0f);
        }
        for (float y = -gridSize; y <= gridSize; y += gridStep) {
            glVertex3f(-gridSize, y, 0.0f);
            glVertex3f(gridSize, y, 0.0f);
        }
        break;

    case 2:
        for (float x = -gridSize; x <= gridSize; x += gridStep) {
            glVertex3f(x, 0.0f, -gridSize);
            glVertex3f(x, 0.0f, gridSize);
        }
        for (float z = -gridSize; z <= gridSize; z += gridStep) {
            glVertex3f(-gridSize, 0.0f, z);
            glVertex3f(gridSize, 0.0f, z);
        }
        break;

    case 3:
        for (float y = -gridSize; y <= gridSize; y += gridStep) {
            glVertex3f(0.0f, y, -gridSize);
            glVertex3f(0.0f, y, gridSize);
        }
        for (float z = -gridSize; z <= gridSize; z += gridStep) {
            glVertex3f(0.0f, -gridSize, z);
            glVertex3f(0.0f, gridSize, z);
        }
        break;
    }

    glEnd();

    glEnable(GL_DEPTH_TEST);
}

void GLWidget::draw2DAxes()
{
    glDisable(GL_DEPTH_TEST);

    glLineWidth(2.5f);

    float axisLength = 8.0f;

    glBegin(GL_LINES);

    switch (projectionType) {
    case 1:
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-axisLength, 0.0f, 0.0f);
        glVertex3f(axisLength, 0.0f, 0.0f);

        glColor3f(0.0f, 0.8f, 0.0f);
        glVertex3f(0.0f, -axisLength, 0.0f);
        glVertex3f(0.0f, axisLength, 0.0f);
        break;

    case 2:
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-axisLength, 0.0f, 0.0f);
        glVertex3f(axisLength, 0.0f, 0.0f);

        glColor3f(0.8f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -axisLength);
        glVertex3f(0.0f, 0.0f, axisLength);
        break;

    case 3:
        glColor3f(0.0f, 0.8f, 0.0f);
        glVertex3f(0.0f, -axisLength, 0.0f);
        glVertex3f(0.0f, axisLength, 0.0f);

        glColor3f(0.8f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -axisLength);
        glVertex3f(0.0f, 0.0f, axisLength);
        break;
    }

    glEnd();

    glBegin(GL_TRIANGLES);

    switch (projectionType) {
    case 1:
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(axisLength, 0.0f, 0.0f);
        glVertex3f(axisLength - 0.3f, 0.2f, 0.0f);
        glVertex3f(axisLength - 0.3f, -0.2f, 0.0f);

        glColor3f(0.0f, 0.8f, 0.0f);
        glVertex3f(0.0f, axisLength, 0.0f);
        glVertex3f(0.2f, axisLength - 0.3f, 0.0f);
        glVertex3f(-0.2f, axisLength - 0.3f, 0.0f);
        break;

    case 2:
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(axisLength, 0.0f, 0.0f);
        glVertex3f(axisLength - 0.3f, 0.0f, 0.2f);
        glVertex3f(axisLength - 0.3f, 0.0f, -0.2f);

        glColor3f(0.8f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, axisLength);
        glVertex3f(0.2f, 0.0f, axisLength - 0.3f);
        glVertex3f(-0.2f, 0.0f, axisLength - 0.3f);
        break;

    case 3:
        glColor3f(0.0f, 0.8f, 0.0f);
        glVertex3f(0.0f, axisLength, 0.0f);
        glVertex3f(0.0f, axisLength - 0.3f, 0.2f);
        glVertex3f(0.0f, axisLength - 0.3f, -0.2f);

        glColor3f(0.8f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, axisLength);
        glVertex3f(0.0f, 0.2f, axisLength - 0.3f);
        glVertex3f(0.0f, -0.2f, axisLength - 0.3f);
        break;
    }

    glEnd();

    glPointSize(4.0f);
    glBegin(GL_POINTS);

    switch (projectionType) {
    case 1:
        glColor3f(1.0f, 0.0f, 0.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(i * 0.5f, 0.0f, 0.0f);
        }
        glColor3f(0.0f, 0.8f, 0.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(0.0f, i * 0.5f, 0.0f);
        }
        break;

    case 2:
        glColor3f(1.0f, 0.0f, 0.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(i * 0.5f, 0.0f, 0.0f);
        }
        glColor3f(0.8f, 0.0f, 1.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(0.0f, 0.0f, i * 0.5f);
        }
        break;

    case 3:
        glColor3f(0.0f, 0.8f, 0.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(0.0f, i * 0.5f, 0.0f);
        }
        glColor3f(0.8f, 0.0f, 1.0f);
        for (int i = -10; i <= 10; i++) {
            if (i != 0) glVertex3f(0.0f, 0.0f, i * 0.5f);
        }
        break;
    }

    glEnd();
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        isLeftMousePressed = true;
    } else if (event->button() == Qt::RightButton) {
        isRightMousePressed = true;
    } else if (event->button() == Qt::MiddleButton) {
        isMiddleMousePressed = true;
    }

    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->pos() - lastMousePos;

    if (isLeftMousePressed) {
        cameraYaw += delta.x() * cameraRotationSpeed;
        cameraPitch -= delta.y() * cameraRotationSpeed;

        if (cameraPitch > 89.0f) cameraPitch = 89.0f;
        if (cameraPitch < -89.0f) cameraPitch = -89.0f;

        updateCamera();
        update();
        emit cameraChanged();
    } else if (isRightMousePressed) {
        float panSpeed = 0.05f;
        cameraTarget += QVector3D(-delta.x() * panSpeed, delta.y() * panSpeed, 0);
        updateCamera();
        update();
        emit cameraChanged();
    } else if (isMiddleMousePressed) {
        cameraDistance += delta.y() * 0.1f;
        if (cameraDistance < 1.0f) cameraDistance = 1.0f;
        if (cameraDistance > 50.0f) cameraDistance = 50.0f;
        updateCamera();
        update();
        emit cameraChanged();
    }

    lastMousePos = event->pos();
    event->accept();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isLeftMousePressed = false;
    } else if (event->button() == Qt::RightButton) {
        isRightMousePressed = false;
    } else if (event->button() == Qt::MiddleButton) {
        isMiddleMousePressed = false;
    }

    event->accept();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numDegrees.isNull()) {
        float zoomFactor = numDegrees.y() > 0 ? 0.9f : 1.1f;
        cameraDistance *= zoomFactor;

        if (cameraDistance < 1.0f) cameraDistance = 1.0f;
        if (cameraDistance > 50.0f) cameraDistance = 50.0f;

        updateCamera();
        update();
        emit cameraChanged();
    }

    event->accept();
}
