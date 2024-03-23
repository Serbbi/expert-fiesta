#include "sun.h"
#include "qimage.h"

Sun::Sun(QVector3D translation, QVector3D rotation, float scale) {
    transformMatrix.setToIdentity();
    transformMatrix.translate(translation);
    transformMatrix.rotate(QQuaternion::fromEulerAngles(rotation));
    transformMatrix.scale(scale);

    initializeModel();
}

Sun::~Sun()
{
}



QMatrix4x4 Sun::getTransformationMatrix()
{
    return transformMatrix;
}

QMatrix3x3 Sun::getNormalMatrix()
{
    return transformMatrix.normalMatrix();
}

GLuint Sun::getVao()
{
    return vao;
}

GLuint Sun::getTexture()
{
    return textureName;
}

GLuint Sun::getSize()
{
    return size;
}

void Sun::initializeModel() {
    Model model(":/models/sun.obj");
    QImage image(":/textures/starry-night-sky.jpg");
    textureCoords = model.getTextureCoords();
    normals = model.getNormals();
    coords = model.getCoords();

    QVector<quint8> textureVector = imageToBytes(image);

    size = coords.size();
}


