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

#ifndef VPVL2_MVD_MODELSECTION_H_
#define VPVL2_MVD_MODELSECTION_H_

#include "vpvl2/mvd/BaseSection.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{
class ModelKeyframe;

class VPVL2_API ModelSection : public BaseSection
{
public:
    ModelSection(IModel *model, NameListSection *nameListSectionRef, size_t align);
    ~ModelSection();

    static bool preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info);

    void release();
    void read(const uint8_t *data);
    void seek(const IKeyframe::TimeIndex &timeIndex);
    void setParentModel(IModel *model);
    void write(uint8_t *data) const;
    size_t estimateSize() const;
    size_t countKeyframes() const;
    void addKeyframe(IKeyframe *keyframe);
    void deleteKeyframe(IKeyframe *&keyframe);
    void getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                      const IKeyframe::LayerIndex &layerIndex,
                      Array<IKeyframe *> &keyframes);

private:
    class PrivateContext;
    IModel *m_modelRef;
    ModelKeyframe *m_keyframePtr;
    PrivateContext *m_contextPtr;
    size_t m_adjustAlighment;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ModelSection)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif

