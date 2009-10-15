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

#include "domprocessor.h"
#include "subprocessor.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

SubProcessor::SubProcessor(const QString &type, const MultiplicityType &multi):
	theMulti(multi)
{
	theType = type;
	thePrimaryOf = 0;
	theIn = theStep = theOut = 1;
	theNumInputs = (theMulti & In) && !(theMulti & Const) ? Undefined : 1;
	theNumOutputs = (theMulti & Out) && !(theMulti & Const) ? Undefined : 1;
	theWidth = 32;
	theHeight = 32;
}

void SubProcessor::setupIO(uint numInputs, uint numOutputs)
{
	theNumInputs = numInputs;
	theNumOutputs = numOutputs;
}

void SubProcessor::setupSamplesIO(uint samplesIn, uint samplesStep, uint samplesOut)
{
	if (samplesIn < samplesStep)
	{	theIn = samplesStep;
		qDebug(	"*** WARNING: setupIO/setupSamplesIO(): samplesIn is less than samplesStep. Increasing samplesIn\n"
				"             to match samplesStep. Read subprocessor.h or the API docs for more\n"
				"             info.");
	}
	else
		theIn = samplesIn;
	theStep = samplesStep;
	theOut = samplesOut;
}

void SubProcessor::setupVisual(uint width, uint height)
{
	theWidth = width;
	theHeight = height;
}

void SubProcessor::paintProcessor(QPainter& _p) const
{
	QRectF area(0, 0, width() - 1, height() - 1);
	_p.setFont(QFont("Helvetica", height() *4/5, QFont::Black, false));
	_p.setPen(Qt::black);
	_p.drawText(area, Qt::AlignCenter, simpleText());
	area.setTopLeft(QPointF(1, 1));
	_p.setPen(Qt::white);
	_p.drawText(area, Qt::AlignCenter, simpleText());
}

PropertiesInfo SubProcessor::specifyProperties() const
{
	return PropertiesInfo();
}

void SubProcessor::processChunk(const BufferDatas &, BufferDatas &) const
{
	qFatal("*** FATAL: Missing SubProcessor processChunk implementation.");
}

void SubProcessor::processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const
{
	for (uint i = 0; i < chunks; i++)
	{	const BufferDatas ini = in.samples(i * theStep, theIn);
		BufferDatas outi = out.samples(i * theOut, theOut);
		processChunk(ini, outi);
	}
}

void SubProcessor::processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks)
{
	processChunks(in, out, chunks);
}

void SubProcessor::defineIO(uint numInputs, uint numOutputs)
{
	theNumInputs = numInputs;
	theNumOutputs = numOutputs;
}

}

#undef MESSAGES
