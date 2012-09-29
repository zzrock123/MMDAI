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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd/Joint.h"
#include "vpvl2/pmd/RigidBody.h"

namespace
{

using namespace vpvl2::pmd;

#pragma pack(push, 1)

struct JointUnit {
    uint8_t name[Joint::kNameSize];
    int bodyIDA;
    int bodyIDB;
    float position[3];
    float rotation[3];
    float positionLowerLimit[3];
    float positionUpperLimit[3];
    float rotationLowerLimit[3];
    float rotationUpperLimit[3];
    float positionStiffness[3];
    float rotationStiffness[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd
{

Joint::Joint(IEncoding *encodingRef)
    : common::Joint::Joint(),
      m_encodingRef(encodingRef)
{
}

Joint::~Joint()
{
    m_encodingRef = 0;
}

bool Joint::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size) || size * sizeof(JointUnit) > rest) {
        return false;
    }
    info.jointsCount = size;
    info.jointsPtr = ptr;
    internal::readBytes(size * sizeof(JointUnit), ptr, rest);
    return true;
}

bool Joint::loadJoints(const Array<Joint *> &joints, const Array<RigidBody *> &rigidBodies)
{
    const int njoints = joints.count();
    const int nRigidBodies = rigidBodies.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        const int rigidBodyIndex1 = joint->m_rigidBodyIndex1;
        if (rigidBodyIndex1 >= 0) {
            if (rigidBodyIndex1 >= nRigidBodies)
                return false;
            else
                joint->m_rigidBody1Ref = rigidBodies[rigidBodyIndex1];
        }
        const int rigidBodyIndex2 = joint->m_rigidBodyIndex2;
        if (rigidBodyIndex2 >= 0) {
            if (rigidBodyIndex2 >= nRigidBodies)
                return false;
            else
                joint->m_rigidBody2Ref = rigidBodies[rigidBodyIndex2];
        }
        if (joint->m_rigidBody1Ref && joint->m_rigidBody2Ref)
            joint->m_constraint = joint->createConstraint();
        joint->m_index = i;
    }
    return true;
}

void Joint::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    JointUnit unit;
    internal::getData(data, unit);
    m_name = m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize);
    m_rigidBodyIndex1 = unit.bodyIDA;
    m_rigidBodyIndex2 = unit.bodyIDB;
    internal::setPositionRaw(unit.position, m_position);
    internal::setPositionRaw(unit.rotation, m_rotation);
    internal::setPositionRaw(unit.positionLowerLimit, m_positionLowerLimit);
    internal::setPositionRaw(unit.rotationLowerLimit, m_rotationLowerLimit);
    internal::setPositionRaw(unit.positionUpperLimit, m_positionUpperLimit);
    internal::setPositionRaw(unit.rotationUpperLimit, m_rotationUpperLimit);
    internal::setPositionRaw(unit.positionStiffness, m_positionStiffness);
    internal::setPositionRaw(unit.rotationStiffness, m_rotationStiffness);
    size = sizeof(unit);
}

}
}
