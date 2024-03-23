#include "mainview.h"

#include <QDateTime>
#include <cmath>

/**
 * @brief MainView::MainView Constructs a new main view.
 *
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
    qDebug() << "MainView constructor";

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateRotation()));

    timer.start(0);
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 */
MainView::~MainView() {
    qDebug() << "MainView destructor";

    makeCurrent();

    destroyModelBuffers();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions.
 */
void MainView::initializeGL() {
    qDebug() << ":: Initializing OpenGL";
    initializeOpenGLFunctions();

    connect(&debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
            SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

    if (debugLogger.initialize()) {
        qDebug() << ":: Logging initialized";
        debugLogger.startLogging(QOpenGLDebugLogger::SynchronousLogging);
    }

    QString glVersion{reinterpret_cast<const char *>(glGetString(GL_VERSION))};
    qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable backface culling
    glEnable(GL_CULL_FACE);

    // Default is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // Set the color to be used by glClear. This is, effectively, the background color.
    glClearColor(0.31f, 0.0f, 0.51f, 0.0f);

    createShaderProgram();
    loadMesh(":/models/terrain2.obj");
    loadSun();
    loadShip();

    //rotation 0, 349, 28
    //translation 0, 1, 28

    // Initialize transformations
    updateProjectionTransform();
    // translation = QVector3D(-100, -30, -17);
    meshTransform.setToIdentity();
    meshTransform.translate(-100, -30, -17);
    meshTransform.rotate(QQuaternion::fromEulerAngles({20, rotation.y(), rotation.z()}));
    // meshTransform.scale(150);
    // updateModelTransforms();


    sunTransform.setToIdentity();
    sunTransform.translate(0, -10, -150); //actual sun 0, -10, -150
    sunTransform.rotate(QQuaternion::fromEulerAngles({0, 0, 0}));
    sunTransform.scale(150);

    spaceShipTransform.setToIdentity();
    shipTranslation = QVector3D(0, -10, -28);
    spaceShipTransform.translate(0, -10, -28);
    spaceShipTransform.rotate(QQuaternion::fromEulerAngles({0, 349, 28}));
    spaceShipTransform.scale(2);

    // set lighting
    lightPosition = QVector3D(100.0F, 50.0F, 0.0F);
    lightColor = QVector3D(1.0F, 1.0F, 1.0F);
    shaderPrograms[shadingMode].setUniformValue("lightPosition", lightPosition);
    shaderPrograms[shadingMode].setUniformValue("lightColor", lightColor);

    // set material properties
    shaderPrograms[shadingMode].setUniformValue("materialCoeffecients", QVector4D(0.4F, 0.4F, 0.4F, 8.0F));
}

void MainView::updateRotation() {
    flying += 0.2f;
    if(flying >= 800) {
        flying = 0;
    }

    for (int i = 0; i < meshSize; ++i) {
        float actualx = (terrainVertices[i].x() + 2);
        float actualz = (terrainVertices[i].z() - 2) * -1;
        terrainVertices[i].setY(noise.pixelColor(actualx, actualz + flying).red() / 6.0f);
    }

    // Bind and fill vertex coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, meshPositionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, terrainVertices.size() * sizeof(QVector3D),
                    terrainVertices.constData());

    // Unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shipTranslation = QVector3D(0, -10 + noise.pixelColor(25, 25 + flying).red() / 12.0f, shipTranslation.z());
    updateSpaceShipTransform();
}
/**
 * @brief MainView::createShaderProgram Creates a new shader program with a
 * vertex and fragment shader.
 */
void MainView::createShaderProgram() {
    // Create shader program

    shaderPrograms[PHONG].addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                  ":/shaders/vertshader_phong.glsl");
    shaderPrograms[PHONG].addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                  ":/shaders/fragshader_phong.glsl");
    shaderPrograms[NORMAL].addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                   ":/shaders/vertshader_lines.glsl");
    shaderPrograms[NORMAL].addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                   ":/shaders/fragshader_lines.glsl");
    shaderPrograms[BLACKGREENWHITE].addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                    ":/shaders/vertshader_gouraud.glsl");
    shaderPrograms[BLACKGREENWHITE].addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                    ":/shaders/fragshader_gouraud.glsl");
    shaderPrograms[RAINBOWLAYERS].addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                            ":/shaders/vertshader_rainbowlayers.glsl");
    shaderPrograms[RAINBOWLAYERS].addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                            ":/shaders/fragshader_rainbowlayers.glsl");

    shaderPrograms[PHONG].link();
    shaderPrograms[NORMAL].link();
    shaderPrograms[BLACKGREENWHITE].link();
    shaderPrograms[RAINBOWLAYERS].link();
}

void MainView::loadSun() {
    Model model(":/models/sun.obj");
    QImage image(":/textures/starry-night-sky.jpg");
    QVector<QVector2D> sunTextureCoords = model.getTextureCoords();
    QVector<QVector3D> sunNormals = model.getNormals();
    QVector<QVector3D> sunCoords = model.getCoords();

    QVector<quint8> textureVector = imageToBytes(image);

    sunSize = sunCoords.size();

    // Generate VAO
    glGenVertexArrays(1, &sunVAO);
    glBindVertexArray(sunVAO);

    // Generate VBOs
    glGenBuffers(1, &sunPositionVBO);
    glGenBuffers(1, &sunNormalVBO);
    glGenBuffers(1, &sunTextureCoordVBO);
    glGenTextures(1, &textureName);

    // Bind and fill vertex coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, sunPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sunCoords.size() * sizeof(QVector3D),
                 sunCoords.data(), GL_STATIC_DRAW);

    // Set vertex coordinates attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(0);

    // Bind and fill normals VBO
    glBindBuffer(GL_ARRAY_BUFFER, sunNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, sunNormals.size() * sizeof(QVector3D),
                 sunNormals.data(), GL_STATIC_DRAW);

    // Set normals attribute pointer
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(1);

    // Bind and fill texture coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, sunTextureCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sunTextureCoords.size() * sizeof(QVector2D),
                 sunTextureCoords.data(), GL_STATIC_DRAW);

    // Set texture coordinates attribute pointer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(2);

    // Unbind VBOs and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureName);
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureVector.data());
}

void MainView::loadShip() {
    Model model(":/models/sun.obj");
    QImage image(":/textures/path836.png");
    QVector<QVector2D> spaceShipTextureCoords = model.getTextureCoords();
    QVector<QVector3D> spaceShipNormals = model.getNormals();
    QVector<QVector3D> spaceShipCoords = model.getCoords();

    QVector<quint8> textureVector = imageToBytes(image);

    spaceShipSize = spaceShipCoords.size();

    // Generate VAO
    glGenVertexArrays(1, &spaceShipVAO);
    glBindVertexArray(spaceShipVAO);

    // Generate VBOs
    glGenBuffers(1, &spaceShipPositionVBO);
    glGenBuffers(1, &spaceShipNormalVBO);
    glGenBuffers(1, &spaceShipTextureCoordVBO);
    glGenTextures(1, &shipTexture);

    // Bind and fill vertex coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, spaceShipPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, spaceShipCoords.size() * sizeof(QVector3D),
                 spaceShipCoords.data(), GL_STATIC_DRAW);

    // Set vertex coordinates attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(0);

    // Bind and fill normals VBO
    glBindBuffer(GL_ARRAY_BUFFER, spaceShipNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, spaceShipNormals.size() * sizeof(QVector3D),
                 spaceShipNormals.data(), GL_STATIC_DRAW);

    // Set normals attribute pointer
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(1);

    // Bind and fill texture coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, spaceShipTextureCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, spaceShipTextureCoords.size() * sizeof(QVector2D),
                 spaceShipTextureCoords.data(), GL_STATIC_DRAW);

    // Set texture coordinates attribute pointer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(2);

    // Unbind VBOs and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, shipTexture);
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureVector.data());
}

/**
 * @brief MainView::loadMesh Loads a mesh from a given file. Note that only one
 * mesh can be loaded at a time.
 * @param filename Filename of where the mesh is located.
 */
void MainView::loadMesh(const QString &filename) {
    Model model(filename);
    QImage image(":/textures/noiseTextureG.png");
    noise = image;
    terrainVertices = model.getCoords();

    meshSize = terrainVertices.size();

    // Generate VAO
    glGenVertexArrays(1, &meshVAO);
    glBindVertexArray(meshVAO);

    // Generate VBOs
    glGenBuffers(1, &meshPositionVBO);

    // Bind and fill vertex coordinates VBO
    glBindBuffer(GL_ARRAY_BUFFER, meshPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(QVector3D),
                 terrainVertices.data(), GL_STATIC_DRAW);

    // Set vertex coordinates attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(0);

    // Unbind VBOs and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


// --- OpenGL drawing

/**
 * @brief MainView::paintGL Actual function used for drawing to the screen.
 *
 */
void MainView::paintGL() {
    hue += 0.1f;
    // Convert HSV to RGB
    float r, g, b;
    hsvToRgb(hue, 1.0f, 1.0f, r, g, b);

    // if(shadingMode == NORMAL)
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    // Clear the screen before rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderPrograms[shadingMode].link();
    shaderPrograms[shadingMode].bind();

    // Update the uniform values. Note that it is better to only do this when the
    // matrices change, but for the sake of simplicity this was not done.
    shaderPrograms[shadingMode].setUniformValue("modelViewTransform", meshTransform);
    shaderPrograms[shadingMode].setUniformValue("projectionTransform", projectionTransform);
    if(shadingMode == NORMAL) {
        shaderPrograms[NORMAL].setUniformValue("lineColor", QVector3D(r, g, b));
    }
    if(shadingMode == BLACKGREENWHITE) {
        hsvToRgb(bottomHue, 1.0f, 1.0f, r, g, b);
        shaderPrograms[BLACKGREENWHITE].setUniformValue("bottomColor", QVector3D(r, g, b));
        hsvToRgb(middleHue, 1.0f, 1.0f, r, g, b);
        shaderPrograms[BLACKGREENWHITE].setUniformValue("middleColor", QVector3D(r, g, b));
        hsvToRgb(topHue, 1.0f, 1.0f, r, g, b);
        shaderPrograms[BLACKGREENWHITE].setUniformValue("topColor", QVector3D(r, g, b));
    }


    glBindVertexArray(meshVAO);
    glDrawArrays(GL_TRIANGLES, 0, meshSize);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    shaderPrograms[PHONG].bind();
    shaderPrograms[PHONG].setUniformValue("materialCoeffecients", QVector4D(0.4F, 0.4F, 0.4F, 8.0F));
    shaderPrograms[PHONG].setUniformValue("lightPosition", lightPosition);
    shaderPrograms[PHONG].setUniformValue("lightColor", lightColor);
    shaderPrograms[PHONG].setUniformValue("modelViewTransform", sunTransform);
    shaderPrograms[PHONG].setUniformValue("projectionTransform", projectionTransform);
    shaderPrograms[PHONG].setUniformValue("normalMatrix", normalMatrix);
    shaderPrograms[PHONG].setUniformValue("samplerUniform", 0);

    glBindVertexArray(sunVAO);
    glDrawArrays(GL_TRIANGLES, 0, sunSize);


    glBindTexture(GL_TEXTURE_2D, shipTexture);
    shaderPrograms[PHONG].setUniformValue("modelViewTransform", spaceShipTransform);
    shaderPrograms[PHONG].setUniformValue("normalMatrix", spaceShipTransform.normalMatrix());
    shaderPrograms[PHONG].setUniformValue("samplerUniform", 0);

    glBindVertexArray(spaceShipVAO);
    glDrawArrays(GL_TRIANGLES, 0, spaceShipSize);

    shaderPrograms[shadingMode].release();
}

void MainView::hsvToRgb(float h, float s, float v, float &r, float &g, float &b) {
    int i = static_cast<int>(std::floor(h / 60.0f)) % 6;
    float f = h / 60.0f - std::floor(h / 60.0f);
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
    }
}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
    Q_UNUSED(newWidth)
    Q_UNUSED(newHeight)
    updateProjectionTransform();
}

/**
 * @brief MainView::updateProjectionTransform Updates the projection transform
 * matrix taking into consideration the current aspect ratio.
 */
void MainView::updateProjectionTransform() {
    float aspectRatio =
        static_cast<float>(width()) / static_cast<float>(height());
    projectionTransform.setToIdentity();
    projectionTransform.perspective(60.0F, aspectRatio, 0.2F, 600.0F);
}

/**
 * @brief MainView::updateModelTransforms Updates the model transform matrix of
 * the mesh to reflect the current rotation and scale values.
 */
void MainView::updateModelTransforms() {
    meshTransform.setToIdentity();
    meshTransform.translate(translation.x(), translation.y(), translation.z());
    meshTransform.rotate(QQuaternion::fromEulerAngles(rotation));
    meshTransform.scale(scale);
    normalMatrix = meshTransform.normalMatrix();
    update();
}

void MainView::updateBackgroundTransform() {
    sunTransform.setToIdentity();
    sunTransform.translate(translation.x(), translation.y(), translation.z());
    sunTransform.rotate(QQuaternion::fromEulerAngles(rotation));
    sunTransform.scale(scale);
    update();
}

void MainView::updateSpaceShipTransform() {
    spaceShipTransform.setToIdentity();
    spaceShipTransform.translate(shipTranslation.x(), shipTranslation.y(), shipTranslation.z());
    spaceShipTransform.rotate(QQuaternion::fromEulerAngles(shipRotation));
    spaceShipTransform.scale(shipScale);
    update();
}

void MainView::setBottomHue(float value)
{
    bottomHue = value;
}

void MainView::setMiddleHue(float value)
{
    middleHue = value;
}

void MainView::setTopHue(float value)
{
    topHue = value;
}

/**
 * @brief MainView::destroyModelBuffers Cleans up the memory used by OpenGL.
 */
void MainView::destroyModelBuffers() {
    glDeleteBuffers(1, &meshPositionVBO);
    glDeleteBuffers(1, &sunPositionVBO);
    glDeleteBuffers(1, &spaceShipPositionVBO);
    glDeleteBuffers(1, &meshNormalVBO);
    glDeleteBuffers(1, &sunNormalVBO);
    glDeleteBuffers(1, &spaceShipNormalVBO);
    glDeleteVertexArrays(1, &meshVAO);
    glDeleteVertexArrays(1, &sunVAO);
    glDeleteVertexArrays(1, &spaceShipVAO);
    glDeleteBuffers(1, &meshTextureCoordVBO);
    glDeleteBuffers(1, &sunTextureCoordVBO);
    glDeleteBuffers(1, &spaceShipTextureCoordVBO);
    glDeleteTextures(1, &textureName);
}

/**
 * @brief MainView::setRotation Changes the rotation of the displayed objects.
 * @param rotateX Number of degrees to rotate around the x axis.
 * @param rotateY Number of degrees to rotate around the y axis.
 * @param rotateZ Number of degrees to rotate around the z axis.
 */
void MainView::setRotation(int rotateX, int rotateY, int rotateZ) {
    shipRotation = {static_cast<float>(rotateX), static_cast<float>(rotateY),
                static_cast<float>(rotateZ)};
    qDebug() << QVector3D(rotateX, rotateY, rotateZ);
    updateSpaceShipTransform();
}

void MainView::setTranslation(QVector3D position) {
    shipTranslation = position;
    qDebug() << position;
    updateSpaceShipTransform();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float newScale) {
    scale = newScale;
    updateModelTransforms();
}

void MainView::setShadingMode(ShadingMode shading) {
    shadingMode = shading;
    qDebug() << "Changed shading to" << shading;
}

/**
 * @brief MainView::onMessageLogged OpenGL logging function, do not change.
 *
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
    qDebug() << " → Log:" << Message;
}
