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

#pragma once

#include <QMap>
#include <QString>
#include <QDebug>

#include <qglobal.h>
#include <exscalibar.h>

namespace Geddei
{

class TypeRegistrationFace;
class TransmissionType;

class DLLEXPORT TypeRegistrar
{
public:
	static TypeRegistrar* get() { return s_one ? s_one : (s_one = new TypeRegistrar); }

	void registerType(TypeRegistrationFace* _ttr, QString const& _t) { m_list.insert(_t, _ttr); }
	void unregisterType(TypeRegistrationFace* _ttr) { m_list.remove(m_list.key(_ttr)); }

	TransmissionType* create(QString const& _type);
	TransmissionType* copy(TransmissionType const* _src);

private:
	QMap<QString, TypeRegistrationFace*> m_list;
	static TypeRegistrar* s_one;
};

struct DLLEXPORT TypeRegistrationFace
{
	inline TypeRegistrationFace(QString const& _type) { TypeRegistrar::get()->registerType(this, _type); }
	virtual inline ~TypeRegistrationFace() { TypeRegistrar::get()->unregisterType(this); }
	virtual QString type() const = 0;
	virtual TransmissionType* create() const = 0;
	virtual TransmissionType* copy(TransmissionType const* _src) const = 0;
};

template<class T>
struct DLLEXPORT TypeRegistration: public TypeRegistrationFace
{
	TypeRegistration(QString const& _t): TypeRegistrationFace(_t) {}
	virtual QString type() const { return T::staticType(); }
	virtual TransmissionType* create() const { return new T; }
	virtual TransmissionType* copy(TransmissionType const* _src) const { return new T(*static_cast<T const*>(_src)); }
};

}
