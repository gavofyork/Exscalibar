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

#include <cassert>
using namespace std;

#include "processor.h"
#include "subprocessor.h"
#include "bufferdata.h"
#include "lxconnection.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

LxConnection::LxConnection(Source *source, uint sourceIndex) : theSource(source), theSourceIndex(sourceIndex), m_sub(0)
{
	theSource->doRegisterOut(this, sourceIndex);

	theScratch = NULL;
	theScratchSize = 0;
}

LxConnection::~LxConnection()
{
	delete [] theScratch;

	theSource->undoRegisterOut(this, theSourceIndex);
}

void LxConnection::push(BufferData const& _data)
{
	if (m_sub)
	{
		m_midType->polishData(_data, theSource, theSourceIndex);

		int as = _data.samples() + m_leftOver / m_midType.size();
		int chunks = (as >= (int)m_sub->theIn) ? (as - m_sub->theIn) / (int)m_sub->theStep + 1 : 0;
		if (freeInDestinationBuffer() != Undefined)
			chunks = min<int>(chunks, freeInDestinationBuffer() / (theType.size() * m_sub->theOut));
		if (m_sub->isInplace())
		{
			BufferDatas d(1);
			d.copyData(0, _data);
			BufferDatas e(1);
			e.copyData(0, _data);
			m_sub->processOwnChunks(d, e, chunks);
			type()->polishData(e[0], theSource, theSourceIndex);
			pushBE(_data);
		}
		else
		{
			int samplesForNextTime = (as - chunks * (int)m_sub->theStep) / m_midType.size();
			if (chunks > 0)
			{
				BufferData bd(m_midScratch.constData(), (m_sub->theIn + m_sub->theStep * (chunks - 1)) * m_midType.size(), m_midType.size());
				BufferDatas d(1);
				d.copyData(0, bd);
				BufferDatas e(1);
				e.copyData(0, makeScratchElements(chunks * theType.size() * m_sub->theOut, false));

				m_sub->processOwnChunks(d, e, chunks);
				type()->polishData(e[0], theSource, theSourceIndex);
				pushBE(e[0]);
				assert(m_midScratch.count() >= (int)((chunks * m_sub->theStep + samplesForNextTime) * m_midType.size()));
				memmove(m_midScratch.data(), m_midScratch.constData() + chunks * m_sub->theStep * m_midType.size(), samplesForNextTime * m_midType.size() * sizeof(float));
			}
			m_leftOver = samplesForNextTime * m_midType.size();
		}
	}
	else
	{
		type()->polishData(_data, theSource, theSourceIndex);
		pushBE(_data);
	}
}

void LxConnection::setMinimumRead(uint _s)
{
	Connection::setMinimumRead(_s);
}

void LxConnection::setMinimumWrite(uint _s)
{
	if (m_sub)
		enforceMinimumWrite(_s / m_sub->theStep * m_sub->theOut * theType.size());
	else
		Connection::setMinimumWrite(_s);
}

void LxConnection::setType(Type const& _t)
{
	if (m_sub)
	{
		Types tins(1);
		Types touts(1);
		tins[0] = _t;
		touts[0] = _t;
		if (m_sub->proxyVSTypes(tins, touts))
		{
			m_midType = _t;
			theType = touts[0];
			m_midScratch.clear();
			m_leftOver = 0;
		}
		else
		{
			m_midType.nullify();
			theType.nullify();
		}
	}
	else
	{
		m_midType = _t;
		theType = _t;
	}
}

void LxConnection::setSub(SubProcessor* _s)
{
	m_sub = _s;
	m_midType.nullify();
}

uint LxConnection::maximumScratchSamples(uint minimum)
{
	uint e = freeInDestinationBuffer(theType->elementsFromSamples(minimum));
	if (e == Undefined)
		return Undefined;

	if (m_sub)
		return (max<int>(0, (theType->samples(e) / m_sub->theOut - 1) * m_sub->theStep + m_sub->theIn) * m_midType.size() - m_leftOver) / m_midType.size();
	else
		return theType->samples(e);
}

uint LxConnection::maximumScratchSamplesEver()
{
	uint e = freeInDestinationBufferEver();
	if (e == Undefined)
		return Undefined;

	if (m_sub)
		return max<int>(0, (theType->samples(e) / m_sub->theOut - 1) * m_sub->theStep + m_sub->theIn);
	else
		return theType->samples(freeInDestinationBufferEver());
}

BufferData LxConnection::makeScratchSamples(uint _samples, bool _autoPush)
{
	if (m_sub && !m_sub->isInplace())
	{
		uint aelements = _samples * m_midType.size() + m_leftOver;

		if (m_midScratch.size() < (int)aelements)
		{
			qDebug() << "*** MSS(" << _samples << "): RESIZING to " << aelements << m_leftOver;
			assert(aelements <= (m_sub->theIn + _samples) * m_midType.size());
			m_midScratch.resize((m_sub->theIn + _samples) * m_midType.size());
//			m_midScratch.resize(aelements);
		}

		BufferData d = BufferData(_samples * m_midType.size(), m_midType.size(), m_midScratch.data() + m_leftOver, dynamic_cast<ScratchOwner*>(this), _autoPush ? BufferInfo::Activate : BufferInfo::Forget);
		m_midType->initData(d, theSource, theSourceIndex);
		lastScratch = d.identity();
		return d;
	}
	else
	{
		BufferData d = makeScratchElements((m_sub ? m_midType : type())->elementsFromSamples(_samples), _autoPush);
		type()->initData(d, theSource, theSourceIndex);
		return d;
	}
}

BufferData LxConnection::makeScratchElements(uint _elements, bool _autoPush)
{
	if (theScratchSize < _elements)
	{
		delete [] theScratch;
		theScratch = new float[_elements];
		theScratchSize = _elements;
	}
	BufferData ret = BufferData(_elements, type().size(), theScratch, dynamic_cast<ScratchOwner *>(this), _autoPush ? BufferInfo::Activate : BufferInfo::Forget);
	if (!m_sub)
		lastScratch = ret.identity();
	return ret;
}

void LxConnection::forgetScratch(const BufferData &data)
{
	assert(lastScratch == data.identity());
	lastScratch = 0;
	data.ignoreDeath();
}

void LxConnection::pushScratch(const BufferData &data)
{
	assert(lastScratch == data.identity());
	data.ignoreDeath();
	push(data);
	data.invalidate();
}

}

#undef MESSAGES
