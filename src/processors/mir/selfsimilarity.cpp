/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include <cmath>
using namespace std;

#include <stdint.h>

#include "qfactoryexporter.h"

#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

#include "spectrum.h"
#include "value.h"
#include "matrix.h"
using namespace Geddei;

static inline float cosineDistance(const float *x, const float *y, uint bandWidth)
{
	double ret = 0., mx = 0., my = 0.;

	for (uint i = 0; i < bandWidth; i++)
	{
		ret += abs(x[i] * y[i]);
		mx += x[i] * x[i];
		my += y[i] * y[i];
	}
	float div = sqrt(mx) * sqrt(my);
	if (!isFinite(div) || !isFinite(ret))
		qDebug() << "BAD NUMBER: " << ret << div << mx << my;
	if (isFinite(ret) && isFinite(div) && div > 0)
		return 1.f - ret / div;

	return 1;
}

static inline float magnitudeDistance(const float *x, const float *y, uint bandWidth)
{
	float ret = 0.f;
	for (uint i = 0; i < bandWidth; i++)
		ret += (x[i] - y[i]) * (x[i] - y[i]);
	return ret > 0 ? sqrt(ret / bandWidth) : 1.f;
}

static inline float magnitudeGreater(const float *x, const float *y, uint bandWidth)
{
	float xmag = 0.f;
	float ymag = 0.f;
	for (uint i = 0; i < bandWidth; i++)
		xmag += x[i] * x[i], ymag += y[i] * y[i];
	return sqrt(xmag / (ymag + 1));
}


class Distance: public SubProcessor
{
	uint m_arity;
	float(*m_distance)(const float *, const float *, uint);

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const
	{
		float const* p = in[0].readPointer();
		out[0][0] = m_distance(p, p + m_arity, m_arity);
	}
	virtual void initFromProperties(Properties const&) { setupSamplesIO(2, 1, 1); }
	virtual void updateFromProperties(Properties const& _p);
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
	{
		m_arity = inTypes[0].arity();
		outTypes[0] = Value(inTypes[0].asA<Contiguous>().frequency());
		return true;
	}
	virtual PropertiesInfo specifyProperties() const;

public:
	Distance() : SubProcessor("Distance") {}
};

void Distance::updateFromProperties(const Properties &properties)
{
	if (properties["Distance Function"].toInt() == 0)
		m_distance = cosineDistance;
	else if (properties["Distance Function"].toInt() == 1)
		m_distance = magnitudeDistance;
	else if (properties["Distance Function"].toInt() == 2)
		m_distance = magnitudeGreater;
	else
		qFatal("*** ERROR: Invalid distance function index given.");
}

PropertiesInfo Distance::specifyProperties() const
{
	return PropertiesInfo("Distance Function", 0, "The distance function to be used when calculating the similarity. { 0: Cosine; 1: Magnitude ; 2: MagnitudeGreater }", true, QChar(0x2248), QList<AllowedValue>() << AllowedValue("Cosine", QChar(0x2221), 0) << AllowedValue("Magnitude", "=", 1) << AllowedValue("Greater", ">", 2));
}

EXPORT_CLASS(Distance, 0,2,0, SubProcessor);

class SelfSimilarity : public SubProcessor
{
	uint theSize;
	uint theBandWidth;
	mutable QVector<float> theMatrix;
	float(*theDistance)(const float *, const float *, uint);

	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual QString simpleText() const { return QChar(0x2258); }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(40, 96, 160); }

public:
	SelfSimilarity() : SubProcessor("SelfSimilarity") {}
};

void SelfSimilarity::processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks)
{
	uint step;
	if (theMatrix.isEmpty())
	{
		theMatrix.resize(theSize * theSize);
		step = theSize;
	}
	else
		step = 1;

	for (uint c = 0; c < chunks; c++)
	{
		if (step < theSize)
			memmove(theMatrix.data(), theMatrix.data() + (theSize + 1) * step, (theSize * (theSize - step) - step) * sizeof(float));
		for (uint p = 0; p < step; p++)
		{
			uint li = theSize * (theSize - p) - 1 - p;
			uint ls = theSize - 1 - p + c;
			BufferData latestBD = in[0].sample(ls);
			float const* latest = latestBD.readPointer();
			for (uint i = 1; i < theSize - p; i++)
			{
				theMatrix[li - i] = theDistance(in[0].sample(ls - i).readPointer(), latest, theBandWidth);
				theMatrix[li - i * theSize] = theDistance(latest, in[0].sample(ls - i).readPointer(), theBandWidth);
			}
			theMatrix[li] = 0;
		}
		out[0].sample(0).copyFrom(theMatrix.constData());
		step = 1;
	}
	return;
}

bool SelfSimilarity::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes[0].isA<Spectrum>()) return false;
	outTypes[0] = SquareMatrix(theSize, inTypes[0].asA<Spectrum>().frequency(), 1.f / inTypes[0].asA<Spectrum>().frequency());
	theBandWidth = inTypes[0].asA<Spectrum>().bins();
	return true;
}

void SelfSimilarity::initFromProperties(const Properties &properties)
{
	theSize = properties.get("Size").toInt();
	theMatrix.clear();
	updateFromProperties(properties);
	setupSamplesIO(theSize, 1, 1);
}

void SelfSimilarity::updateFromProperties(const Properties &properties)
{
	if (properties["Distance Function"].toInt() == 0)
		theDistance = cosineDistance;
	else if (properties["Distance Function"].toInt() == 1)
		theDistance = magnitudeDistance;
	else if (properties["Distance Function"].toInt() == 2)
		theDistance = magnitudeGreater;
	else
		qFatal("*** ERROR: Invalid distance function index given.");
}

PropertiesInfo SelfSimilarity::specifyProperties() const
{
	return PropertiesInfo("Size", 64, "The size of the block (in samples) from which to create a self-similarity matrix.", false, QChar(0x2311), QList<AllowedValue>() << AllowedValue("Samples", "s", 8, 1024, AllowedValue::Log2))
						 ("Distance Function", 0, "The distance function to be used when calculating the similarity.", true, QChar(0x2248), QList<AllowedValue>() << AllowedValue("Cosine", QChar(0x2221), 0) << AllowedValue("Magnitude", "=", 1) << AllowedValue("Greater", ">", 2));
}

EXPORT_CLASS(SelfSimilarity, 0,2,0, SubProcessor);
