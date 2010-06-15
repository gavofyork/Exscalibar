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

#include "properties.h"
#include "autoproperties.h"
using namespace Geddei;

namespace Geddei
{

void AutoProperties::loadProperties(Properties const& _p, bool _dynamics)
{
#define HANDLE_TYPE(t, T) else if (memberInfo()[i].type == QString(#t)) *(t*)((uint8_t*)this + memberInfo()[i].offset) = _p[n].to ## T()
	for (int i = 0; i < memberCount(); i++)
	{
		QString n(memberInfo()[i].name);
		if (n.startsWith("m_"))
			n = n.mid(2);
		n[0] = n[0].toUpper();
		if (_p.contains(n) && (m_dynamics.contains(n) == _dynamics))
		{
			if (0) {}
			HANDLE_TYPE(float, Float);
			HANDLE_TYPE(double, Double);
			HANDLE_TYPE(int, Int);
			HANDLE_TYPE(bool, Bool);
		}
	}
#undef HANDLE_TYPE
}

void AutoProperties::updateDynamics(PropertiesInfo const& _pi)
{
	m_dynamics.clear();
	foreach (QString k, _pi.keys())
		if (_pi.datum(k).isDynamic)
			m_dynamics << k;
}

}
