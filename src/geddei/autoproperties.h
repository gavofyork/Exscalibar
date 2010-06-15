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

#include <QStringList>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "memberinfo.h"
#else
#include <qtextra/memberinfo.h>
#endif

namespace Geddei
{

class Properties;
class PropertiesInfo;

class AutoProperties
{
	TT_BASE_CLASS(AutoProperties);
	TT_NO_MEMBERS;

protected:
	void loadProperties(Properties const& _p, bool _dynamicsOnly = false);
	void updateDynamics(PropertiesInfo const& _pi);

	QStringList m_dynamics;
};

}
