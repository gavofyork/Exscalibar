/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __PLAYER_H
#define __PLAYER_H

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
 * @author Gav Wood <gav@cs.york.ac.uk>
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
class DLLEXPORT Player: public Processor
{
	QString thePath;
	enum { NoMode, ModeSF, ModeVF, ModeMAD } theMode;

#ifdef HAVE_SNDFILE
	SNDFILE *theSndFile;
#endif
#ifdef HAVE_VORBISFILE
	OggVorbis_File theVorbisFile;
#endif
#ifdef HAVE_MAD
	FILE *theMadFile;
	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
	bstdfile_t			*BstdFile;
#endif
	uint theChannels, theRate, theLength, thePosition, theReadFrames;

	virtual void paintProcessor(QPainter &p);
	virtual void processor();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual void specifyOutputSpace(Q3ValueVector<uint> &samples);

public:
	/**
	 * Basic constructor.
	 */
	Player(QString const& _filename = QString::null);
};

#endif

