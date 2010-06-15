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

#pragma once

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "subprocessor.h"
#else
#include <geddei/subprocessor.h>
#endif

namespace Geddei
{

class DLLEXPORT Combination: public SubProcessor
{
public:
	Combination(SubProcessor *x, SubProcessor *y);
	~Combination();

	SubProcessor* x() const { return theX; }
	SubProcessor* y() const { return theY; }

	void resetTime() { m_totalTimeX = m_totalTimeY = 0; }
	double totalTimeX() const { return m_totalTimeX; }
	double totalTimeY() const { return m_totalTimeY; }

private:
	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual void processOwnChunks(const BufferDatas &in, BufferDatas &out, uint chunks);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &p);
	virtual void updateFromProperties(Properties const&p);
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual QString type() const { return theX->type() + "&" + theY->type(); }

	SubProcessor *theX;
	SubProcessor *theY;

	mutable double m_totalTimeX;
	mutable double m_totalTimeY;

	uint theInterScope;
	mutable BufferData *theResident;
};

}
