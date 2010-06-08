/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef HAVE_VORBISFILE
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#endif

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

#ifdef HAVE_MAD
#include <mad.h>
typedef struct bstdfile bstdfile_t;
#endif

#include <qfile.h>
#include <qdatetime.h>
#include <qmutex.h>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "qfastwaitcondition.h"
#include "processor.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/processor.h>
#endif

/** @ingroup Toolkit
 * @author Gav Wood <gav@kde.org>
 * @brief Wav-file playing source Processor object.
 *
 * This just plays a single file given by the property "Filename" into the the
 * output port(s). The number of ports on the object will be equivalent to the
 * number of channel the wav-file has.
 *
 * A plunger will be placed at the stream end.
 *
 * A further property, "Frames" determines how many samples of the file are
 * pushed to the output port per iteration. A higher value will typically
 * result in higher throughput, but will increase the minimum buffer size and
 * the overall latency of the system.
 */
class DLLEXPORT Player: public CoProcessor
{
	QFile m_file;
	enum { NoMode, ModeSF, ModeVF, ModeMAD } m_mode;

#ifdef HAVE_SNDFILE
	SNDFILE*				m_sndFile;
	SF_INFO					m_sfinfo;
	QVector<float>			m_buffer;
#endif
#ifdef HAVE_VORBISFILE
	OggVorbis_File			m_vorbisFile;
#endif
#ifdef HAVE_MAD
	FILE*					m_madFile;
	struct mad_stream		m_stream;
	struct mad_frame		m_frame;
	struct mad_synth		m_synth;
	mad_timer_t				m_timer;
	bstdfile_t*				m_bStdFile;
	uint					m_inputBufferSize;
	QVector<unsigned char>	m_inputBuffer;
	unsigned char*			m_guardPointer;
#endif
	uint m_channels;
	uint m_rate;
	uint m_length;
	uint m_position;
	uint m_readFrames;

	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual void specifyOutputSpace(QVector<uint> &samples);
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 0, 160); }

public:
	/**
	 * Basic constructor.
	 */
	Player(QString const& _filename = QString::null);
};
