/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_SCENE_H_
#define VPVL2_SCENE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

namespace vpvl2
{

class ICamera;
class IEffect;
class IEncoding;
class ILight;
class IModel;
class IMotion;
class IRenderDelegate;
class IRenderEngine;

class VPVL2_API Scene
{
public:
    enum AccelerationType {
        kSoftwareFallback,
        kOpenCLAccelerationType1,
        kVertexShaderAccelerationType1
    };

    static ICamera *createCamera();
    static ILight *createLight();
    static bool isAcceleratorSupported();
    static const Scalar &defaultFPS();

    Scene();
    virtual ~Scene();

    IRenderEngine *createRenderEngine(vpvl2::IRenderDelegate *delegate, IModel *model) const;
    void addModel(IModel *model, IRenderEngine *engine);
    void addMotion(IMotion *motion);
    IEffect *createEffect(const IString *path, IRenderDelegate *delegate);
    IEffect *createEffect(const IString *dir, const IModel *model, IRenderDelegate *delegate);
    void deleteModel(vpvl2::IModel *&model);
    void removeMotion(IMotion *motion);
    void advance(const IKeyframe::TimeIndex &delta);
    void seek(const IKeyframe::TimeIndex &timeIndex);
    void updateModel(IModel *model) const;
    void updateModels();
    void updateRenderEngines();
    void updateCamera();
    void setPreferredFPS(const Scalar &value);
    bool isReachedTo(const IKeyframe::TimeIndex &timeIndex) const;
    float maxFrameIndex() const;
    const Array<IModel *> &models() const;
    const Array<IMotion *> &motions() const;
    const Array<IRenderEngine *> &renderEngines() const;
    IModel *findModel(const IString *name) const;
    IRenderEngine *findRenderEngine(IModel *model) const;
    ILight *light() const;
    ICamera *camera() const;
    const Scalar &preferredFPS() const;

    AccelerationType accelerationType() const;
    void setAccelerationType(AccelerationType value);

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpvl2 */

#endif
