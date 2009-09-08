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

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

#ifdef HAVE_SNDFILE

#include <sndfile.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "processor.h"
#else
#include <geddei/processor.h>
#endif

/** @ingroup Toolkit
 * @author Gav Wood <gav@cs.york.ac.uk>
 * @brief Multiple wav-file playing/queuing source Processor object.
 * 
 * This plays a series of wav files given though the QStringList property
 * "Filenames".
 * 
 * The signal of each file will be appended with a plunger. This allows another
 * class to count the plungers and determine the correct file title for any
 * given section of signal with the getTitle() method.
 * 
 * All files should be the same sampling frequency, number of channels and bits
 * per sample. Those not the same as the first in the list will be ignored.
 * 
 * A further property, "Frames" determines how many samples of the files are
 * pushed to the output port per iteration. A higher value will typically
 * result in higher throughput, but will increase the minimum buffer size and
 * the overall latency of the system.
 */
class DLLEXPORT MultiPlayer: public Processor
{
	QStringList thePaths;
	SNDFILE *theFile;
	int theChannels, theRate;
	uint theLength, thePosition, theFrames;
	uint theIndex;

	bool openFile();

	virtual void processor();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual void specifyOutputSpace(QVector<uint> &samples);

public:
	/**
	 * Get the title of the file of given index.
	 * 
	 * @param index The index of the file whose title you wish to attain.
	 * @return The title of the track with index @a index (starting at 0).
	 */
	const QString &getTitle(int index) { return thePaths[index]; }

	/**
	 * Basic constructor.
	 */
	MultiPlayer();
};

#endif

#endif
