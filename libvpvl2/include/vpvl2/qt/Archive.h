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

#ifndef VPVL2_QT_ARCHIVE_H
#define VPVL2_QT_ARCHIVE_H

#include "vpvl2/Common.h"
#include "vpvl2/qt/unzip.h"

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QString>

namespace vpvl2
{
namespace qt
{

class Archive
{
public:
    enum ErrorType {
        kNone,
        kGetCurrentFileError,
        kGoToNextFileError,
        kGoToFirstFileError,
        kOpenCurrentFileError,
        kReadCurrentFileError,
        kCloseCurrentFileError
    };

    Archive();
    ~Archive();

    void setTextCodec(QTextCodec *value);
    bool open(const QString &filename, QStringList &entryNames);
    bool close();
    bool uncompress(const QStringList &entryNames);
    void replaceFilePath(const QString &from, const QString &to);
    ErrorType error() const;
    const QStringList entryNames() const;
    const QByteArray data(const QString &name) const;

private:
    unzFile m_file;
    unz_global_info64 m_header;
    ErrorType m_error;
    QTextCodec *m_codec;
    QHash<QString, QByteArray> m_entries;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Archive)
};

}
}

#endif // ARCHIVE_H
