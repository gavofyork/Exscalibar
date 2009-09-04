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

#define __GEDDEI_BUILD

#include "player.h"

#include "buffer.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

#include "geddeinite.h"

Player::Player(const QString &path) : Processor("Player"), thePath(path)
{
#ifdef HAVE_SNDFILE
	SF_INFO sfinfo;
	SNDFILE *theFile = sf_open(thePath, SFM_READ, &sfinfo);
	theLength = sfinfo.frames;
	theChannels = sfinfo.channels;
	theRate = sfinfo.samplerate;
	sf_close(theFile);
#else
	theLength = 0;
	theChannels = 0;
	theRate = 0;
#endif
	theBlockSize = 131072;
	thePosition = 0;
}

void Player::paintProcessor(QPainter &p)
{
	p.setPen(QColor(200, 200, 200));
	p.setBrush(QColor(232, 232, 232));
	p.drawRect(2, 2, 146, 12);
	p.setPen(QColor(128, 128, 128));
	p.drawText(4, 12, thePath);

	p.setPen(QColor(160, 160, 160));
	p.setBrush(QColor(232, 232, 232));
	p.drawRect(2, 24, 146, 4);
	bool seg = false;
	for(uint i = 0; i < theLength; i += 60 * theRate)
	{	seg = !seg;
		uint ni = i + 60 * theRate;
		if(ni > theLength) ni = theLength;
		if(thePosition > i && thePosition < ni)
		{	p.fillRect(3 + int(144 * i / theLength), 25, int(144 * (thePosition - i) / theLength), 2, QColor(10, 64, seg ? 232 : 255, QColor::Hsv));
			p.fillRect(3 + int(144 * thePosition / theLength), 25, int(144 * (ni - thePosition) / theLength), 2, QColor(60, 0, seg ? 232 : 255, QColor::Hsv));
		}
		else
			p.fillRect(3 + int(144 * i / theLength), 25, int(144 * (ni - i) / theLength), 2, QColor(10, thePosition > i ? 64 : 0, seg ? 232 : 255, QColor::Hsv));
	}
}

const bool Player::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &outTypes)
{
	outTypes = Wave(theRate);
	return true;
}

void Player::specifyOutputSpace(Q3ValueVector<uint> &sizes)
{
	for(uint i = 0; i < theChannels; i++)
		sizes[i] = theBlockSize * 2;
}

void Player::initFromProperties(const Properties &)
{
	setupIO(0, theChannels);
	setupVisual(150, 30, 1000);
}

void Player::processor()
{
#ifdef HAVE_SNDFILE
	SF_INFO sfinfo;
	theFile = sf_open(thePath, SFM_READ, &sfinfo);
	int frames = theBlockSize;
	float buffer[frames * theChannels];
	int in = 0;
	while(true)
	{	if((in = sf_readf_float(theFile, buffer, frames)) > 0)
		{	thePosition += in;
			for(uint i = 0; i < theChannels; i++)
			{	BufferData d = output(i).makeScratchSamples(in);
				for(int j = 0; j < in; j++) d[j] = buffer[j * theChannels + i];
				output(i) << d;
			}
		}
		else if(in == 0)
			return;
		else
			sf_perror(theFile);
	}
#endif
}

void Player::processorStopped()
{
#ifdef HAVE_SNDFILE
	sf_close(theFile);
	thePosition = 0;
#endif
}
