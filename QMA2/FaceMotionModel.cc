#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, vpvl::Face *face, bool isRoot, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_face(face),
          m_isRoot(isRoot)
    {
    }
    ~TreeItem() {
        qDeleteAll(m_children);
    }

    void addChild(ITreeItem *item) {
        m_children.append(static_cast<TreeItem *>(item));
    }
    ITreeItem *parent() const {
        return m_parent;
    }
    ITreeItem *child(int row) const {
        return m_children.value(row);
    }
    const QString &name() const {
        return m_name;
    }
    vpvl::Face *face() const {
        return m_face;
    }
    bool isRoot() const {
        return m_isRoot;
    }
    int rowIndex() const {
        return m_parent ? m_parent->m_children.indexOf(const_cast<TreeItem *>(this)) : 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<TreeItem *> m_children;
    QString m_name;
    TreeItem *m_parent;
    vpvl::Face *m_face;
    bool m_isRoot;
};

class CopyFramesCommand : public QUndoCommand
{
public:
    CopyFramesCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~CopyFramesCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
};

class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~SetFramesCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~ResetAllCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
};

class SetFaceCommand : public QUndoCommand
{
public:
    SetFaceCommand(vpvl::PMDModel *model, vpvl::PMDModel::State *state)
        : QUndoCommand(),
          m_model(model),
          m_newState(0),
          m_oldState(state)
    {
        m_newState = m_model->saveState();
    }
    virtual ~SetFaceCommand() {
        m_model->discardState(m_newState);
        m_model->discardState(m_oldState);
    }

    void undo() {
        m_model->restoreState(m_oldState);
        m_model->updateImmediate();
    }
    void redo() {
        execute();
    }

private:
    void execute() {
        m_model->restoreState(m_newState);
        m_model->updateImmediate();
    }

    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_newState;
    vpvl::PMDModel::State *m_oldState;
};

}

FaceMotionModel::FaceMotionModel(QUndoGroup *undo, QObject *parent)
    : MotionBaseModel(undo, parent),
      m_state(0)
{
    m_root = new TreeItem("", 0, true, 0);
}

FaceMotionModel::~FaceMotionModel()
{
}

void FaceMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    if (m_model) {
        vpvl::FaceAnimation *animation = motion->mutableFaceAnimation();
        foreach (QVariant value, values()) {
            vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
            const QByteArray &bytes = value.toByteArray();
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyFrame(newFrame);
        }
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void FaceMotionModel::copyFrames(int /* frameIndex */)
{
}

void FaceMotionModel::startTransform()
{
    if (m_model) {
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void FaceMotionModel::commitTransform()
{
    if (m_model && m_state) {
        addUndoCommand(new SetFaceCommand(m_model, m_state));
        m_state = 0;
    }
}

void FaceMotionModel::setFrames(const QList<Frame> &frames)
{
    if (!m_model || !m_motion) {
        qWarning("No model or motion to register a bone frame.");
        return;
    }
    QString key;
    foreach (Frame pair, frames) {
        int frameIndex = pair.first;
        vpvl::Face *face = pair.second;
        key = internal::toQString(face->name());
        const Keys keys = this->keys();
        if (keys.contains(key)) {
            const QModelIndex &modelIndex = frameToIndex(keys[key], frameIndex);
            vpvl::FaceAnimation *animation = m_motion->mutableFaceAnimation();
            vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
            newFrame->setName(face->name());
            newFrame->setWeight(face->weight());
            newFrame->setFrameIndex(frameIndex);
            QByteArray bytes(vpvl::FaceKeyFrame::strideSize(), '0');
            newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
            animation->addKeyFrame(newFrame);
            animation->refresh();
            setData(modelIndex, bytes, Qt::EditRole);
        }
        else {
            qWarning("Tried registering not face key frame: %s", qPrintable(key));
        }
    }
}

void FaceMotionModel::resetAllFaces()
{
    if (m_model) {
        m_model->resetAllFaces();
        updateModel();
    }
}

void FaceMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    clearKeys();
    if (model) {
        const vpvl::FaceList &faces = model->facesForUI();
        uint32_t nFaces = faces.count();
        TreeItem *eyeblow = new TreeItem(tr("Eyeblow"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *eye = new TreeItem(tr("Eye"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *lip = new TreeItem(tr("Lip"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *other = new TreeItem(tr("Other"), 0, false, static_cast<TreeItem *>(m_root));
        Keys &keys = m_keys[model];
        for (uint32_t i = 0; i < nFaces; i++) {
            vpvl::Face *face = faces[i];
            const QString name = internal::toQString(face);
            TreeItem *child, *parent = 0;
            switch (face->type()) {
            case vpvl::Face::kEyeblow:
                parent = eyeblow;
                break;
            case vpvl::Face::kEye:
                parent = eye;
                break;
            case vpvl::Face::kLip:
                parent = lip;
                break;
            case vpvl::Face::kOther:
                parent = other;
                break;
            default:
                break;
            }
            if (parent) {
                child = new TreeItem(name, face, false, parent);
                parent->addChild(child);
                keys.insert(name, child);
            }
        }
        m_root->addChild(eyeblow);
        m_root->addChild(eye);
        m_root->addChild(lip);
        m_root->addChild(other);
        MotionBaseModel::setPMDModel(model);
        qDebug("Set a model in FaceMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    reset();
}

void FaceMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::FaceAnimation &animation = motion->faceAnimation();
        uint32_t nFaceFrames = animation.countKeyFrames();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            const Keys keys = this->keys();
            if (keys.contains(key)) {
                uint32_t frameIndex = frame->frameIndex();
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                ITreeItem *item = keys[key];
                const QModelIndex &modelIndex = frameToIndex(item, frameIndex);
                vpvl::FaceKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes);
            }
        }
        m_motion = motion;
        refreshModel();
        setModified(false);
        qDebug("Loaded a motion to the model in FaceMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
    }
}

void FaceMotionModel::deleteMotion()
{
    m_selected.clear();
    clearValues();
    setModified(false);
    reset();
    resetAllFaces();
}

void FaceMotionModel::deleteModel()
{
    deleteMotion();
    clearKeys();
    delete m_root;
    m_root = new TreeItem("", 0, true, 0);
    setPMDModel(0);
    reset();
}

void FaceMotionModel::deleteFrame(const QModelIndex &index)
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    vpvl::Face *face = item->face();
    m_motion->mutableFaceAnimation()->deleteKeyFrame(index.column(), face->name());
    setData(index, kInvalidData, Qt::EditRole);
}

void FaceMotionModel::selectFaces(QList<vpvl::Face *> faces)
{
    m_selected = faces;
}

vpvl::Face *FaceMotionModel::findFace(const QString &name)
{
    QByteArray bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        vpvl::Face *face = static_cast<TreeItem *>(item)->face();
        if (!qstrcmp(reinterpret_cast<const char *>(face->name()), bytes))
            return face;
    }
    return 0;
}

void FaceMotionModel::setWeight(float value)
{
    if (!m_selected.isEmpty())
        setWeight(value, m_selected.last());
}

void FaceMotionModel::setWeight(float value, vpvl::Face *face)
{
    if (face) {
        face->setWeight(value);
        updateModel();
    }
}
