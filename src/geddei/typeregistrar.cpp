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

#include "transmissiontype.h"
#include "typeregistrar.h"
using namespace Geddei;

namespace Geddei
{

TypeRegistrar* TypeRegistrar::s_one = 0;

TransmissionType* TypeRegistrar::create(QString const& _type)
{
	if (m_list.contains(_type))
		return m_list.value(_type)->create();
	return new TransmissionType;
}

TransmissionType* TypeRegistrar::copy(TransmissionType const* _src)
{
	if (m_list.contains(_src->type()))
		return m_list.value(_src->type())->copy(_src);
	return new TransmissionType;
}

}
