#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

#include "model.h"
#include "shadingmode.h"

/**
 * @brief The MainView class is resonsible for the actual content of the main
 * window.
 */
class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

 public:
  MainView(QWidget *parent = nullptr);
  ~MainView() override;

  // Functions for widget input events
  void setRotation(int rotateX, int rotateY, int rotateZ);
  void setScale(float scale);
  void setShadingMode(ShadingMode shading);
  void setTranslation(QVector3D position);
  void setBottomHue(float value);
  void setMiddleHue(float value);
  void setTopHue(float value);


 protected:
  void initializeGL() override;
  void resizeGL(int newWidth, int newHeight) override;
  void paintGL() override;


  // Functions for keyboard input events
  void keyPressEvent(QKeyEvent *ev) override;
  void keyReleaseEvent(QKeyEvent *ev) override;

  // Function for mouse input events
  void mouseDoubleClickEvent(QMouseEvent *ev) override;
  void mouseMoveEvent(QMouseEvent *ev) override;
  void mousePressEvent(QMouseEvent *ev) override;
  void mouseReleaseEvent(QMouseEvent *ev) override;
  void wheelEvent(QWheelEvent *ev) override;

 private slots:
  void onMessageLogged(QOpenGLDebugMessage Message);
  void updateRotation();

 private:
  void createShaderProgram();
  void loadMesh(const QString &filename);
  void loadSun();
  void loadShip();
  void hsvToRgb(float h, float s, float v, float &r, float &g, float &b);
  void destroyModelBuffers();
  void updateProjectionTransform();
  void updateModelTransforms();
  void updateBackgroundTransform();
  void updateSpaceShipTransform();
  QVector<quint8> imageToBytes(const QImage &image);

  QOpenGLDebugLogger debugLogger;
  QTimer timer;  // timer used for animation

  QOpenGLShaderProgram shaderPrograms[5];

  // Mesh values
  GLuint meshVAO, sunVAO, spaceShipVAO;
  GLuint meshPositionVBO, meshNormalVBO, sunPositionVBO, sunNormalVBO, spaceShipPositionVBO, spaceShipNormalVBO;
  GLuint meshSize, sunSize, spaceShipSize;
  QMatrix4x4 meshTransform, sunTransform, spaceShipTransform;

  // Transforms
  float scale = 1.0F, shipScale = 2.0F;
  QVector3D rotation, shipRotation = {0, 349, 28};
  QVector3D translation, shipTranslation;
  QMatrix4x4 projectionTransform;

  //stuff we added
  QMatrix3x3 normalMatrix;
  ShadingMode shadingMode;
  QVector3D lightPosition;
  QVector3D lightColor;
  GLuint textureName, shipTexture;
  // GLint samplerUniform;
  GLuint meshTextureCoordVBO, sunTextureCoordVBO, spaceShipTextureCoordVBO;

  QVector<QVector3D> terrainVertices;
  QImage noise;
  float flying = 0;
  float hue = 0, bottomHue = 0, middleHue= 0, topHue = 0;
};

#endif  // MAINVIEW_H
