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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace TransmissionTypes;

class Normalise: public HeavyProcessor
{
	QVector<float> f;
	uint theOutputSpace, m_arity;

	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out);
	virtual void specifyOutputSpace(QVector<uint> &out);
	virtual void processor();
	virtual void receivedPlunger();

public:
	Normalise();
};

Normalise::Normalise(): HeavyProcessor("Normalise", NotMulti, Guarded)
{
}

PropertiesInfo Normalise::specifyProperties() const
{
	return PropertiesInfo("OutputSpace", 8192, "The minimum amount of output space to insist upon for the output buffer");
}

void Normalise::initFromProperties(const Properties &p)
{
	theOutputSpace = p["OutputSpace"].toUInt();
	setupIO(1, 1);
}

bool Normalise::verifyAndSpecifyTypes(const SignalTypeRefs &in, SignalTypeRefs &out)
{
	m_arity = in[0].asA<TransmissionType>().arity();
	out[0] = in[0];
	return true;
}

void Normalise::specifyOutputSpace(QVector<uint> &out)
{
	out[0] = theOutputSpace;
}

void Normalise::processor()
{
	f.clear();
	float last = 0.f;
	while (thereIsInputForProcessing(1))
	{
		const BufferData d = input(0).readSamples();
		for (uint i = 0; i < d.elements(); i++)
		{
			if (isinf(d[i]) || isnan(d[i]))
			{	qDebug("ERROR: Cannot normalise stream with nan/inf in it.");
				f.append(last);
			}
			else
				f.append(last = d[i]);
		}
	}
}

void Normalise::receivedPlunger()
{
	if (!f.size()) return;
	float mini = f[0], maxi = f[0], tu = 0., tb = 0., avgu = 0., avgb = 0., delta, avg = 0.;
	for (uint i = 1; i < (uint)f.size(); i++)
		if (f[i] > maxi) maxi = f[i];
		else if (f[i] < mini) mini = f[i];
	int t = 0;
	for (uint i = 0; i < (uint)f.size(); i++)
		if (f[i] != mini && f[i] != maxi)
		{ avg += f[i] / float(f.size()); t++; }
	for (uint i = 0; i < (uint)f.size(); i++)
		if (f[i] != mini && f[i] != maxi)
		{	if (f[i] > avg) { avgu += f[i]; tu++; }
			else { avgb += f[i]; tb++; }
		}
	avgu /= float(tu);
	avgb /= float(tb);
	float avguu = 0., avgbb = 0.;
	tu = 0.; tb = 0.;
	for (uint i = 0; i < (uint)f.size(); i++)
		if (f[i] != mini && f[i] != maxi)
		{	if (f[i] > avgu) { avguu += f[i]; tu++; }
			else if (f[i] < avgb) { avgbb += f[i]; tb++; }
		}
	avguu /= float(tu);
	avgbb /= float(tb);
	mini = max(avg + (avgb - avg) * 2.f, avgbb);
	maxi = min(avg + (avgu - avg) * 2.f, avguu);
//		mini = avgbb;
//		maxi = avguu;
/*		for (uint i = 1; i < f.size(); i++)
			if (f[i] > maxi && f[i] < avg * 10.) maxi = f[i];
			else if (f[i] < mini && f[i] > avg / 10.) mini = f[i];
*/
	delta = maxi - mini;
	if (!delta) delta = 1.;
	BufferData d(f.size(), m_arity);
	for (uint i = 0; i < (uint)f.size(); i++)
		d[i] = finite(f[i]) ? std::min(1.f, std::max(0.f, (f[i] - mini) / delta)) : 0.;
	output(0) << d;
	f.clear();
}

EXPORT_CLASS(Normalise, 0,1,1, Processor);


