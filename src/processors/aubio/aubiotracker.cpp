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

#if defined(HAVE_AUBIO)

#include <aubio.h>

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
#include "value.h"
using namespace SignalTypes;
/*
aubio_onset_energy;
aubio_onset_specdiff;
aubio_onset_hfc;
aubio_onset_complex;
aubio_onset_complex;
aubio_onset_phase;
aubio_onset_mkl;
aubio_onset_kl;
*/
class AubioTracker: public CoProcessor
{
	unsigned int pos;    /* frames%dspblocksize */
	uint_t usepitch;
	fvec_t * out;
	aubio_tempo_t * bt ;
	uint_t istactus;

	fvec_t* rin;

	virtual bool processorStarted();
	virtual void processorStopped();
	virtual int canProcess();
	virtual int process();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(40, 0, 160); }
	virtual void initFromProperties(Properties const&)
	{
		setupIO(1, 1);
	}
	virtual void specifyInputSpace(QVector<uint>& _s) { for (int i = 0; i < _s.count(); i++) _s[i] = 512; }
	virtual PropertiesInfo specifyProperties() const
	{
		return PropertiesInfo();
	}
public:
	AubioTracker(): CoProcessor("AubioTracker") {}
};

bool AubioTracker::verifyAndSpecifyTypes(const SignalTypeRefs & _i, SignalTypeRefs & _o)
{
	if (!_i[0].isA<Wave>())
		return false;
	_o[0] = Value(_i[0].frequency() / 512, 1, 0);
	return true;
}

bool AubioTracker::processorStarted()
{
	pos          = 0;    /* frames%dspblocksize */
	usepitch           = 0;
	out              = NULL;
	bt        = NULL;
	istactus           = 0;
  out = new_fvec(2,1);
  bt  = new_aubio_tempo(aubio_onset_kl,1024,512,1);

	rin = new_fvec(512, 1);

	return true;
}

int AubioTracker::canProcess()
{
	return CanWork;
}

int AubioTracker::process()
{
	{
		BufferData in = input(0).readSamples(512);
		in.copyTo(rin->data[0]);
	}
	aubio_tempo(bt,rin,out);
	BufferData bout = output(0).makeScratchSample(true);
	bout[0] = out->data[0][0];
	return DidWork;
}

void AubioTracker::processorStopped()
{
  del_aubio_tempo(bt);
  del_fvec(out);
	del_fvec(rin);
}

EXPORT_CLASS(AubioTracker, 0,1,0, Processor);

#endif
