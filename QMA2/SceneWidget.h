/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QtCore/QtCore>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>
#include <LinearMath/btVector3.h>

namespace vpvl {
class PMDModel;
class Scene;
class VMDMotion;
class XModel;
}

class World;
class QProgressDialog;

class SceneWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QSettings *settings, QWidget *parent = 0);
    ~SceneWidget();

    void setCurrentFPS(int value);

    const QHash<QString, vpvl::PMDModel *> &models() const {
        return m_models;
    }
    vpvl::PMDModel *selectedModel() const {
        return m_selected;
    }
    void setSelectedModel(vpvl::PMDModel *value) {
        m_selected = value;
    }

    static const QString toUnicodeModelName(const vpvl::PMDModel *model);

public slots:
    void addModel();
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void setStage();
    void setCamera();
    void deleteSelectedModel();
    void resetCamera();
    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotateInternal(10.0f, 0.0f); }
    void rotateDown() { rotateInternal(-10.0f, 0.0f); }
    void rotateLeft() { rotateInternal(0.0f, 10.0f); }
    void rotateRight() { rotateInternal(0.0f, -10.0f); }
    void translateUp() { translateInternal(0.0f, 1.0f); }
    void translateDown() { translateInternal(0.0f, -1.0f); }
    void translateLeft() { translateInternal(-1.0f, 0.0f); }
    void translateRight() { translateInternal(1.0f, 0.0f); }

signals:
    void modelAdded(const vpvl::PMDModel *model);
    void modelDeleted(const vpvl::PMDModel *model);
    void motionAdded(const vpvl::VMDMotion *motion);
    void stageSet(const vpvl::XModel *model);
    void cameraMotionSet(const vpvl::VMDMotion *motion);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeGL(int w, int h);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void initializeSurface();
    vpvl::PMDModel *addModelInternal(const QString &baseName, const QDir &dir);
    vpvl::VMDMotion *addMotionInternal(vpvl::PMDModel *model, const QString &path);
    vpvl::VMDMotion *setCameraInternal(const QString &path);
    vpvl::XModel *setStageInternal(const QString &baseName, const QDir &dir);
    void rotateInternal(float x, float y);
    void translateInternal(float x, float y);
    void setLighting();
    bool loadTexture(const QString &path, GLuint &textureID);
    bool loadToonTexture(const QString &name, const QDir &model, GLuint &textureID);
    void loadModel(vpvl::PMDModel *model, const QDir &dir);
    void unloadModel(const vpvl::PMDModel *model);
    void drawModel(const vpvl::PMDModel *model);
    void drawModelEdge(const vpvl::PMDModel *model);
    void drawModelShadow(const vpvl::PMDModel *model);
    void loadStage(vpvl::XModel *model, const QDir &dir);
    void unloadStage(const vpvl::XModel *model);
    void drawStage(const vpvl::XModel *model);
    void drawSurface();
    void updateFPS();
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void startSceneUpdateTimer();
    void stopSceneUpdateTimer();
    void drawInformativeText(QPainter &painter);
    QProgressDialog *getProgressDialog(const QString &label, int max);
    const QString openFileDialog(const QString &name, const QString &desc, const QString &exts);

    vpvl::Scene *m_scene;
    vpvl::XModel *m_stage;
    vpvl::VMDMotion *m_camera;
    vpvl::PMDModel *m_selected;
    World *m_world;
    QSettings *m_settings;
    QHash<QString, vpvl::PMDModel *> m_models;
    QList<vpvl::VMDMotion *> m_motions;
    QTime m_timer;
    QPoint m_prevPos;
    int m_frameCount;
    int m_currentFPS;
    int m_defaultFPS;
    int m_interval;
    int m_internalTimerID;
};

#endif // SCENEWIDGET_H