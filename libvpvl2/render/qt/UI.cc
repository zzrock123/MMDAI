/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "UI.h"
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CustomGLContext.h>
#include <vpvl2/qt/RenderContext.h>
#include <vpvl2/qt/Encoding.h>
#include <vpvl2/qt/CString.h>
#include <vpvl2/qt/Util.h>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrent>
#endif

#ifdef VPVL2_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
BT_DECLARE_HANDLE(aiScene);
#endif

/* internal headers */
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/vmd/Motion.h"

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

using namespace vpvl2;
using namespace vpvl2::qt;

QDebug operator<<(QDebug debug, const Vector3 &v)
{
    debug.nospace() << "(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Color &v)
{
    debug.nospace() << "(r=" << v.x() << ", g=" << v.y() << ", b=" << v.z() << ", a=" << v.w() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const IString *str)
{
    if (str) {
        debug.nospace() << static_cast<const CString *>(str)->value();
    }
    else {
        debug.nospace() << "(null)";
    }
    return debug;
}

QDebug operator<<(QDebug debug, const pmx::Bone *bone)
{
    if (!bone) {
        debug.nospace() << "Bone is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Bone id                          = " << bone->index();
    debug << "\n";
    debug << "     name                        = " << bone->name();
    debug << "\n";
    debug << "     english                     = " << bone->englishName();
    debug << "\n";
    debug << "     origin                      = " << bone->origin();
    debug << "\n";
    if (bone->parentBoneRef()) {
        debug << "     parent                      = " << bone->parentBoneRef()->name();
        debug << "\n";
    }
    debug << "     index                       = " << bone->layerIndex();
    debug << "\n";
    debug << "     offset                      = " << bone->origin();
    debug << "\n";
    if (bone->hasInverseKinematics()) {
        debug << "     targetBone                  = " << bone->targetBoneRef()->name();
        debug << "\n";
        debug << "     constraintAngle             = " << bone->constraintAngle();
        debug << "\n";
    }
    if (bone->hasPositionInherence()) {
        debug << "     parentPositionInherenceBone = " << bone->parentInherenceBoneRef()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasRotationInherence()) {
        debug << "     parentRotationInherenceBone = " << bone->parentInherenceBoneRef()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasFixedAxes()) {
        debug << "     axis                        = " << bone->axis();
        debug << "\n";
    }
    if (bone->hasLocalAxes()) {
        debug << "     axisX                       = " << bone->axisX();
        debug << "\n";
        debug << "     axisZ                       = " << bone->axisZ();
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Material *material)
{
    if (!material) {
        debug.nospace() << "Material is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Material id                      = " << material->index();
    debug << "\n";
    debug << "         name                    = " << material->name();
    debug << "\n";
    debug << "         english                 = " << material->englishName();
    debug << "\n";
    debug << "         mainTexture             = " << material->mainTexture();
    debug << "\n";
    debug << "         sphereTexture           = " << material->sphereTexture();
    debug << "\n";
    debug << "         toonTexture             = " << material->toonTexture();
    debug << "\n";
    debug << "         ambient                 = " << material->ambient();
    debug << "\n";
    debug << "         diffuse                 = " << material->diffuse();
    debug << "\n";
    debug << "         specular                = " << material->specular();
    debug << "\n";
    debug << "         edgeColor               = " << material->edgeColor();
    debug << "\n";
    debug << "         shininess               = " << material->shininess();
    debug << "\n";
    debug << "         edgeSize                = " << material->edgeSize();
    debug << "\n";
    debug << "         indices                 = " << material->sizeofIndices();
    debug << "\n";
    debug << "         isSharedToonTextureUsed = " << material->isSharedToonTextureUsed();
    debug << "\n";
    debug << "         isCullDisabled          = " << material->isCullFaceDisabled();
    debug << "\n";
    debug << "         hasShadow               = " << material->hasShadow();
    debug << "\n";
    debug << "         isShadowMapDrawin       = " << material->isShadowMapDrawn();
    debug << "\n";
    debug << "         isEdgeDrawn             = " << material->isEdgeDrawn();
    debug << "\n";
    switch (material->sphereTextureRenderMode()) {
    case pmx::Material::kAddTexture:
        debug << "         sphere                  = add";
        break;
    case pmx::Material::kMultTexture:
        debug << "         sphere                  = modulate";
        break;
    case pmx::Material::kNone:
        debug << "         sphere                  = none";
        break;
    case pmx::Material::kSubTexture:
        debug << "         sphere                  = subtexture";
        break;
    case pmx::Material::kMaxSphereTextureRenderModeType:
        break;
    }
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Morph *morph)
{
    if (!morph) {
        debug.nospace() << "Morph is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Morph id       = " << morph->index();
    debug << "\n";
    debug << "      name     = " << morph->name();
    debug << "\n";
    debug << "      english  = " << morph->englishName();
    debug << "\n";
    debug << "      weight   = " << morph->weight();
    debug << "\n";
    debug << "      category = ";
    switch (morph->category()) {
    case IMorph::kBase:
        debug << "kBase";
        break;
    case IMorph::kEyeblow:
        debug << "kEyeblow";
        break;
    case IMorph::kEye:
        debug << "kEye";
        break;
    case IMorph::kLip:
        debug << "kLip";
        break;
    case IMorph::kOther:
        debug << "kOther";
        break;
    default:
        debug << "(unknown)";
        break;
    }
    debug << "\n";
    debug << "      type     = ";
    switch (morph->type()) {
    case IMorph::kGroup:
        debug << "kGroup";
        break;
    case IMorph::kVertex:
        debug << "kVertex";
        break;
    case IMorph::kBone:
        debug << "kBone";
        break;
    case IMorph::kTexCoord:
        debug << "kTexCoord";
        break;
    case IMorph::kUVA1:
        debug << "kUVA1";
        break;
    case IMorph::kUVA2:
        debug << "kUVA2";
        break;
    case IMorph::kUVA3:
        debug << "kUVA3";
        break;
    case IMorph::kUVA4:
        debug << "kUVA4";
        break;
    case IMorph::kMaterial:
        debug << "kMaterial";
        break;
    case IMorph::kFlip:
        debug << "kFlip";
        break;
    case IMorph::kImpulse:
        debug << "kImpulse";
        break;
    default:
        debug << "(unknown)";
        break;
    }
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Label *label)
{
    if (!label) {
        debug.nospace() << "Label is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Label id        = " << label->index();
    debug << "\n";
    debug << "      name      = " << label->name();
    debug << "\n";
    debug << "      english   = " << label->englishName();
    debug << "\n";
    debug << "      isSpecial = " << label->isSpecial();
    debug << "\n";
    debug << "      count     = " << label->count();
    debug << "\n";
    for (int i = 0; i < label->count(); i++) {
        if (IBone *bone = label->bone(i))
            debug << "      bone      = " << bone->name();
        else if (IMorph *morph = label->morph(i))
            debug << "      morph     = " << morph->name();
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::RigidBody *body)
{
    if (!body) {
        debug.nospace() << "RigidBody is null";
        return debug.space();
    }
    debug.nospace();
    debug << "RigidBody id                 = " << body->index();
    debug << "\n";
    debug << "          name               = " << body->name();
    debug << "\n";
    debug << "          english            = " << body->englishName();
    debug << "\n";
    debug << "          size               = " << body->size();
    debug << "\n";
    debug << "          position           = " << body->position();
    debug << "\n";
    debug << "          rotation           = " << body->rotation();
    debug << "\n";
    debug << "          mass               = " << body->mass();
    debug << "\n";
    debug << "          linearDamping      = " << body->linearDamping();
    debug << "\n";
    debug << "          angularDamping     = " << body->angularDamping();
    debug << "\n";
    debug << "          restitution        = " << body->restitution();
    debug << "\n";
    debug << "          friction           = " << body->friction();
    debug << "\n";
    debug << "          groupID            = " << body->groupID();
    debug << "\n";
    debug << "          collisionGroupMask = " << body->collisionGroupMask();
    debug << "\n";
    debug << "          collisionGroupID   = " << body->collisionGroupID();
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Joint *joint)
{
    if (!joint) {
        debug.nospace() << "Joint is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Joint id                 = " << joint->index();
    debug << "\n";
    debug << "      name               = " << joint->name();
    debug << "\n";
    debug << "      english            = " << joint->englishName();
    debug << "\n";
    debug << "      position           = " << joint->position();
    debug << "\n";
    debug << "      rotation           = " << joint->rotation();
    debug << "\n";
    debug << "      positionLowerLimit = " << joint->positionLowerLimit();
    debug << "\n";
    debug << "      positionUpperLimit = " << joint->positionUpperLimit();
    debug << "\n";
    debug << "      rotationLowerLimit = " << joint->rotationLowerLimit();
    debug << "\n";
    debug << "      rotationUpperLimit = " << joint->rotationUpperLimit();
    debug << "\n";
    debug << "      positionStiffness  = " << joint->positionStiffness();
    debug << "\n";
    debug << "      rotationStiffness  = " << joint->rotationStiffness();
    debug << "\n";
    if (joint->rigidBody1()) {
        debug << "      rigidBody1         = " << joint->rigidBody1()->name();
        debug << "\n";
    }
    if (joint->rigidBody2()) {
        debug << "      rigidBody2         = " << joint->rigidBody2()->name();
        debug << "\n";
    }
    return debug.space();
}

namespace vpvl2
{
namespace render
{
namespace qt
{

static void UIBuildConstantsDictionary(QSettings *settings, Encoding::Dictionary &dictionary)
{
    QMap<QString, IEncoding::ConstantType> str2const;
    str2const.insert("arm", IEncoding::kArm);
    str2const.insert("asterisk", IEncoding::kAsterisk);
    str2const.insert("center", IEncoding::kCenter);
    str2const.insert("elbow", IEncoding::kElbow);
    str2const.insert("finger", IEncoding::kFinger);
    str2const.insert("left", IEncoding::kLeft);
    str2const.insert("leftknee", IEncoding::kLeftKnee);
    str2const.insert("right", IEncoding::kRight);
    str2const.insert("rightknee", IEncoding::kRightKnee);
    str2const.insert("spaextension", IEncoding::kSPAExtension);
    str2const.insert("sphextension", IEncoding::kSPHExtension);
    str2const.insert("wrist", IEncoding::kWrist);
    QMapIterator<QString, IEncoding::ConstantType> it(str2const);
    while (it.hasNext()) {
        it.next();
        const QVariant &value = settings->value("constants." + it.key());
        dictionary.insert(it.value(), new CString(value.toString()));
    }
}

class UI::ShadowMap : protected QGLFunctions {
public:
    ShadowMap(int width, int height)
        : QGLFunctions(),
          m_size(width, height, 0),
          m_frameBuffer(0),
          m_colorTexture(0),
          m_depthTexture(0)
    {
        initializeGLFunctions();
    }
    ~ShadowMap() {
        release();
    }

    void create() {
        release();
        glGenFramebuffers(1, &m_frameBuffer);
        glGenTextures(1, &m_colorTexture);
        glGenTextures(1, &m_depthTexture);
        size_t width = m_size.x(), height = m_size.y();
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0);
        setTextureParameters();
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
        setTextureParameters();
        glBindTexture(GL_TEXTURE_2D, 0);
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
        unbind();
    }
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const Vector3 &size() const { return m_size; }
    void *bufferRef() const { return &m_colorTexture; }

private:
    void setTextureParameters() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    void release() {
        glDeleteFramebuffers(1, &m_frameBuffer);
        m_frameBuffer = 0;
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }

    Vector3 m_size;
    GLuint m_frameBuffer;
    mutable GLuint m_colorTexture;
    GLuint m_depthTexture;
};

UI::UI(const QGLFormat &format)
    : QGLWidget(new CustomGLContext(format), 0),
      m_settings(0),
      m_sm(0),
      m_world(0),
      m_delegate(0),
      m_scene(0),
      m_factory(0),
      m_encoding(0),
      m_prevElapsed(0),
      m_currentFrameIndex(0)
{
    m_world = new World();
    m_scene = new Scene();
    setMinimumSize(320, 240);
    setMouseTracking(true);
}

UI::~UI()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::DefaultLogger::kill();
#endif
    qDeleteAll(m_dictionary);
    delete m_delegate;
    m_delegate = 0;
    delete m_factory;
    m_factory = 0;
    delete m_encoding;
    m_encoding = 0;
    delete m_scene;
    m_scene = 0;
    delete m_world;
    m_world = 0;
    delete m_sm;
    m_sm = 0;
}

void UI::load(const QString &filename)
{
    m_settings = new QSettings(filename, QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");
    UIBuildConstantsDictionary(m_settings, m_dictionary);
    m_encoding = new Encoding(m_dictionary);
    m_factory = new Factory(m_encoding);
    QHash<QString, QString> settings;
    foreach (const QString &key, m_settings->allKeys()) {
        settings.insert(key, m_settings->value(key).toString());
    }
    m_delegate = new RenderContext(settings, m_scene, this);
    m_delegate->initialize(m_settings->value("effect.msaa", true).toBool());
    m_delegate->updateMatrices(size());
    resize(m_settings->value("window.width", 640).toInt(), m_settings->value("window.height", 480).toInt());
    m_scene->setPreferredFPS(qMax(m_settings->value("scene.fps", 30).toFloat(), Scene::defaultFPS()));
    if (m_settings->value("enable.opencl", false).toBool())
        m_scene->setAccelerationType(Scene::kOpenCLAccelerationType1);
    else if (m_settings->value("enable.vss", false).toBool())
        m_scene->setAccelerationType(Scene::kVertexShaderAccelerationType1);
    ICamera *camera = m_scene->camera();
    camera->setZNear(qMax(m_settings->value("scene.znear", 0.1f).toFloat(), 0.1f));
    camera->setZFar(qMax(m_settings->value("scene.zfar", 10000.0).toFloat(), 100.0f));
    ILight *light = m_scene->light();
    light->setToonEnable(m_settings->value("enable.toon", true).toBool());
    light->setSoftShadowEnable(m_settings->value("enable.ss", true).toBool());
    m_sm = new ShadowMap(1024, 1024);
    if (m_settings->value("enable.sm", false).toBool()) {
        m_sm->create();
        light->setDepthTextureSize(m_sm->size());
        light->setDepthTexture(m_sm->bufferRef());
        light->setHasFloatTexture(true);
    }
    if (loadScene()) {
        m_updateTimer.start(0, this);
        m_refreshTimer.start();
    }
    else {
        qFatal("Unable to load scene");
    }
}

void UI::rotate(float x, float y)
{
    ICamera *camera = m_scene->camera();
    Vector3 angle = camera->angle();
    angle.setX(angle.x() + x);
    angle.setY(angle.y() + y);
    camera->setAngle(angle);
}

void UI::translate(float x, float y)
{
    ICamera *camera = m_scene->camera();
    const Vector3 &diff = camera->modelViewTransform() * Vector3(x, y, 0);
    Vector3 position = camera->lookAt() + diff;
    camera->setLookAt(position + diff);
}

void UI::closeEvent(QCloseEvent *event)
{
    QThreadPool *pool = QThreadPool::globalInstance();
    if (pool->activeThreadCount() > 0)
        pool->waitForDone();
    event->accept();
}

void UI::initializeGL()
{
    initializeGLFunctions();
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    qDebug("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void UI::timerEvent(QTimerEvent *)
{
    const Scalar &elapsed = m_refreshTimer.elapsed() / static_cast<Scalar>(60.0f);
    Scalar delta(elapsed - m_prevElapsed);
    m_prevElapsed = elapsed;
    if (delta < 0)
        delta = elapsed;
    m_scene->advance(delta, Scene::kUpdateAll);
    const Array<IMotion *> &motions = m_scene->motions();
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        if (motion->isReachedTo(motion->maxTimeIndex())) {
            motion->reset();
            m_currentFrameIndex = 0;
        }
    }
    m_world->stepSimulation(delta);
    m_delegate->updateMatrices(size());
    m_scene->update(Scene::kUpdateAll);
    updateGL();
}

void UI::mousePressEvent(QMouseEvent *event)
{
    m_prevPos = event->pos();
    setMousePositions(event);
}

void UI::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        const QPoint &diff = event->pos() - m_prevPos;
        if (modifiers & Qt::ShiftModifier) {
            translate(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotate(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
    setMousePositions(event);
}

void UI::mouseReleaseEvent(QMouseEvent *event)
{
    setMousePositions(event);
}

void UI::wheelEvent(QWheelEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();
    ICamera *camera = m_scene->camera();
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        const qreal step = 1.0;
        camera->setFov(qMax(event->delta() > 0 ? camera->fov() - step : camera->fov() + step, 0.0));
    }
    else {
        qreal step = 4.0;
        if (modifiers & Qt::ControlModifier)
            step *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            step *= 0.2f;
        if (step != 0.0f)
            camera->setDistance(event->delta() > 0 ? camera->distance() - step : camera->distance() + step);
    }
}

void UI::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void UI::paintGL()
{
    if (!m_delegate) {
        glViewport(0, 0, width(), height());
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }
    renderDepth();
    renderOffscreen();
    renderWindow();
}

void UI::renderDepth()
{
    if (m_scene->light()->depthTexture()) {
        m_sm->bind();
        Vector3 target(0, 10, 0);
        Scalar maxRadius = 0;
        const Array<IRenderEngine *> &engines = m_scene->renderEngines();
        const Array<IModel *> &models = m_scene->models();
        const int nengines = engines.count();
        const int nmodels = models.count();
        Array<Scalar> radiusArray;
        Array<Vector3> centerArray;
        Vector3 aabbMin, aabbMax;
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            if (model->isVisible()) {
                model->getAabb(aabbMin, aabbMax);
                if (!aabbMin.isZero() && !aabbMax.isZero()) {
                    radiusArray.add((aabbMax - aabbMin).length());
                    centerArray.add((aabbMin + aabbMax) / 2);
                }
            }
        }
        for (int i = 0; i < nmodels; i++) {
            IModel *model = models[i];
            if (model->isVisible()) {
                const Vector3 &c = centerArray[i];
                const Scalar &r = radiusArray[i];
                const Scalar &d = target.distance(c) + r;
                btSetMax(maxRadius, d);
            }
        }
        const Vector3 &size = m_sm->size();
        const Scalar &angle = 45;
        const Scalar &aspectRatio = size.x() / size.y();
        const Scalar &distance = maxRadius / btSin(btRadians(angle) * 0.5);
        const Scalar &margin = 50;
        const Vector3 &eye = -m_scene->light()->direction().normalized() * maxRadius + target;
        QMatrix4x4 lightViewMatrix, lightProjectionMatrix;
        lightViewMatrix.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                               QVector3D(target.x(), target.y(), target.z()),
                               QVector3D(0, 1, 0));
        lightProjectionMatrix.perspective(angle, aspectRatio, 1, distance + maxRadius + margin);
        m_delegate->setLightMatrices(QMatrix4x4(), lightViewMatrix, lightProjectionMatrix);
        glViewport(0, 0, size.x(), size.y());
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            engine->renderZPlot();
        }
        m_sm->unbind();
    }
}

void UI::renderOffscreen()
{
    m_delegate->renderOffscreen(size());
}

void UI::renderWindow()
{
    const Array<IRenderEngine *> &engines = m_scene->renderEngines();
    const int nengines = engines.count();
    glViewport(0, 0, width(), height());
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPostProcess);
        engine->setEffect(IEffect::kPostProcess, effect, 0);
        engine->preparePostProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPreProcess);
        engine->setEffect(IEffect::kPreProcess, effect, 0);
        engine->performPreProcess();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kStandard);
        engine->setEffect(IEffect::kStandard, effect, 0);
        engine->renderModel();
        engine->renderEdge();
        engine->renderShadow();
    }
    for (int i = 0; i < nengines; i++) {
        IRenderEngine *engine = engines[i];
        IEffect *effect = engine->effect(IEffect::kPostProcess);
        engine->setEffect(IEffect::kPostProcess, effect, 0);
        engine->performPostProcess();
    }
}

void UI::setMousePositions(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    const QSizeF &size = geometry().size() / 2;
    const qreal w = size.width(), h = size.height();
    const Vector3 &value = Vector3((pos.x() - w) / w, (pos.y() - h) / -h, 0);
    Qt::MouseButtons buttons = event->buttons();
    m_delegate->setMousePosition(value, buttons & Qt::LeftButton, IRenderContext::kMouseLeftPressPosition);
    m_delegate->setMousePosition(value, buttons & Qt::MiddleButton, IRenderContext::kMouseMiddlePressPosition);
    m_delegate->setMousePosition(value, buttons & Qt::RightButton, IRenderContext::kMouseRightPressPosition);
    m_delegate->setMousePosition(value, false, IRenderContext::kMouseCursorPosition);
}

bool UI::loadScene()
{
#ifdef VPVL2_LINK_ASSIMP
    Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
    Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
#endif
    const QString &modelMotionPath = QDir(m_settings->value("dir.motion").toString())
            .absoluteFilePath(m_settings->value("file.motion").toString());
    const QString &cameraMotionPath = QDir(m_settings->value("dir.camera").toString())
            .absoluteFilePath(m_settings->value("file.camera").toString());
    QProgressDialog dialog(this);
    dialog.setWindowModality(Qt::ApplicationModal);
    dialog.setLabelText("Loading scene...");
    dialog.setMaximum(-1);
    dialog.show();
    int nmodels = m_settings->beginReadArray("models");
    for (int i = 0; i < nmodels; i++) {
        m_settings->setArrayIndex(i);
        const QString &path = m_settings->value("path").toString();
        IModel *model = addModel(path, dialog);
        if (model)
            addMotion(modelMotionPath, model);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    m_settings->endArray();
    int nassets = m_settings->beginReadArray("assets");
    for (int i = 0; i < nassets; i++) {
        m_settings->setArrayIndex(i);
        const QString &path = m_settings->value("path").toString();
        IModel *model = addModel(path, dialog);
        Q_UNUSED(model)
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    m_settings->endArray();
    IMotion *cameraMotion = loadMotion(cameraMotionPath, 0);
    if (cameraMotion)
        m_scene->camera()->setMotion(cameraMotion);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    dialog.setValue(dialog.value() + 1);
    m_scene->seek(0, Scene::kUpdateAll);
    m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    return true;
}

IModel *UI::createModelAsync(const QString &path) const
{
    QByteArray bytes;
    if (!UISlurpFile(path, bytes)) {
        qWarning("Failed loading the model");
        return 0;
    }
    bool ok = true;
    const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
    return m_factory->createModel(data, bytes.size(), ok);
}

IMotion *UI::createMotionAsync(const QString &path, IModel *model) const
{
    QByteArray bytes;
    if (UISlurpFile(path, bytes)) {
        bool ok = true;
        IMotion *motion = m_factory->createMotion(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size(), model, ok);
        motion->seek(0);
        return motion;
    }
    else {
        qWarning("Failed parsing the model motion, skipped...");
    }
    return 0;
}

IModel *UI::addModel(const QString &path, QProgressDialog &dialog)
{
    const QFileInfo info(path);
    const QFuture<IModel *> &future = QtConcurrent::run(this, &UI::createModelAsync, path);
    dialog.setLabelText(QString("Loading %1...").arg(info.fileName()));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QScopedPointer<IModel> modelPtr(future.result());
    dialog.setValue(dialog.value() + 1);
    if (!modelPtr || future.isCanceled()) {
        return 0;
    }
    if (modelPtr->error() != IModel::kNoError) {
        qWarning("Failed parsing the model: %d", modelPtr->error());
        return 0;
    }
    m_delegate->addModelPath(modelPtr.data(), info.fileName());
    CString s1(info.absoluteDir().absolutePath());
    const QFuture<IEffect *> &future2 = QtConcurrent::run(m_delegate, &RenderContext::createEffectAsync, modelPtr.data(), &s1);
    dialog.setLabelText(QString("Loading an effect of %1...").arg(info.fileName()));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    IEffect *effect = future2.result();
    int flags = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
    if (!effect) {
        qWarning() << "Effect" <<  m_delegate->effectFilePath(modelPtr.data(), &s1) << "does not exists";
    }
    else if (!effect->internalPointer()) {
        CGcontext c = static_cast<CGcontext>(effect->internalContext());
        qWarning() << cgGetLastListing(c);
    }
    else {
        flags = Scene::kEffectCapable;
    }
#else
    Q_UNUSED(effect)
#endif
    IModel *model = 0;
    QScopedPointer<IRenderEngine> enginePtr(m_scene->createRenderEngine(m_delegate, modelPtr.data(), flags));
    if (enginePtr->upload(&s1)) {
        modelPtr->setEdgeWidth(m_settings->value("edge.width", 1.0).toFloat());
        if (m_settings->value("enable.physics", true).toBool())
            m_world->addModel(modelPtr.data());
        if (!modelPtr->name()) {
            CString s(info.fileName());
            modelPtr->setName(&s);
        }
        m_scene->addModel(modelPtr.data(), enginePtr.data());
#ifdef VPVL2_ENABLE_NVIDIA_CG
        enginePtr->setEffect(IEffect::kAutoDetection, effect, &s1);
        m_delegate->parseOffscreenSemantic(effect, info.absoluteDir());
#endif
        model = modelPtr.take();
        enginePtr.take();
    }
    else {
        return 0;
    }
#if 0
    pmx::Model *pmx = dynamic_cast<pmx::Model*>(model);
    if (pmx) {
        const Array<pmx::Material *> &materials = pmx->materials();
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++)
            qDebug() << materials[i];
        const Array<pmx::Bone *> &bones = pmx->bones();
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++)
            qDebug() << bones[i];
        const Array<pmx::Morph *> &morphs = pmx->morphs();
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++)
            qDebug() << morphs.at(i);
        const Array<pmx::Label *> &labels = pmx->labels();
        const int nlabels = labels.count();
        for (int i = 0; i < nlabels; i++)
            qDebug() << labels.at(i);
        const Array<pmx::RigidBody *> &bodies = pmx->rigidBodies();
        const int nbodies = bodies.count();
        for (int i = 0; i < nbodies; i++)
            qDebug() << bodies.at(i);
        const Array<pmx::Joint *> &joints = pmx->joints();
        const int njoints = joints.count();
        for (int i = 0; i < njoints; i++)
            qDebug() << joints.at(i);
    }
#endif
    return model;
}

IMotion *UI::addMotion(const QString &path, IModel *model)
{
    IMotion *motion = loadMotion(path, model);
    if (motion)
        m_scene->addMotion(motion);
    return motion;
}

IMotion *UI::loadMotion(const QString &path, IModel *model)
{
    const QFuture<IMotion *> &future = QtConcurrent::run(this, &UI::createMotionAsync, path, model);
    IMotion *motion = future.result();
    if (!motion || future.isCanceled()) {
        delete motion;
        return 0;
    }
    return motion;
}

}
}
}
