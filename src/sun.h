#ifndef SUN_H
#define SUN_H

#include <QMatrix4x4>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector3D>

#include "model.h"

class Sun
{
public:
    Sun(QVector3D translation, QVector3D rotation, float scale);
    ~Sun();
    QMatrix4x4 getTransformationMatrix();
    QMatrix3x3 getNormalMatrix();
    GLuint getVao();
    GLuint getTexture();
    GLuint getSize();


private:
    GLuint vao, positionVbo, normalVbo, textureCoordsVbo, size, textureName;
    QMatrix4x4 transformMatrix;
    float scale = 1.0F;
    QVector3D rotation;
    QVector3D translation;
    QVector<QVector2D> textureCoords;
    QVector<QVector3D> coords;
    QVector<QVector3D> normals;

    void initializeModel();
    QVector<quint8> imageToBytes(const QImage &image);
};

#endif // SUN_H
