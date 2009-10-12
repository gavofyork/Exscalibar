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

/*
 * Usage:
 *
 * For all classes to be exported, you must provide *exactly one*:
 *   EXPORT_CLASS(...) (or any of the derivitive functions)
 * This should be found in single-instance file (i.e. a .cpp, *not* a .h)
 * This exports the given class for provision in the plugin library. It
 * also specifies the version, so that the most recent class can automatically
 * be provided by the factory.
 *
 * At the start of each .cpp file that uses the above command, you must
 * use FACTORY_TYPE(...) to declare the base type that all exported classes
 * are derived from. This *must be the same* throughout the plugin. If not,
 * you'll get no error, but a lot strange stuff when the classes are used.
 *
 * You must also use the FACTORY_EXPORT declaration _once_ in the entire
 * plugin. Put this in the library's equivalent of "main.cpp".
 *
 * If only one .cpp file is compiled you can use the compound command instead
 * of the last two commands:
 *   FACTORY_EXPORT_TYPE(...);
 * Whose argument must be the common base type that all exported classes
 * are derived from. This, like FACTORY_TYPE, should go before any
 * EXPORT_CLASS()s.
 *
 * Example 1: one .cpp file, one .h file:
 *
 * FILE "b.h":
 * #include <a.h>
 * #include <qtextra/qfactoryexporter.h>
 * class B: public A { void foo(); };
 *
 * FILE "b.cpp":
 * #include "b.h"
 * void B::foo() { return; }
 * FACTORY_EXPORT;
 * EXPORT_CLASS(B, 1,0,0, A);
 *
 * Example 2: two class .cpp files, one "main" .cpp file:
 *
 * FILE "common.cpp":
 * FACTORY_EXPORT;
 *
 * FILE "b.cpp":
 * #include <a.h>
 * class B: public A { void foo(); };
 * void B::foo() { return; }
 * EXPORT_CLASS(B, 1,0,0, A);
 *
 * FILE "c.cpp":
 * #include <a.h>
 * class C: public A { void foo(); };
 * void C::foo() { return; }
 * EXPORT_CLASS(C, 1,0,0, A);
 */

#pragma once

#include <typeinfo>

#include <QStringList>
#include <QMap>

#include <exscalibar.h>

/** @internal
 */
class DLLEXPORT Register
{
	static QMap<QString, QStringList> *theLists;
public:
	static QMap<QString, QStringList> &getLists() { return theLists ? *theLists : *(theLists = new QMap<QString, QStringList>); }
	Register(const char *name, const char *base) { getLists()[base] += name; }
};

#define FACTORY_EXPORT \
QMap<QString, QStringList> *Register::theLists = 0; \
extern "C" \
{ \
	DLLEXPORT const QStringList &getAvailable(const QString &base) { return Register::getLists()[base]; } \
}

#define EXPORT_CLASS(Class, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Class(#Class, typeid(Base).name()); \
	DLLEXPORT Base *create ## Class() { return new Class; } \
	DLLEXPORT int version ## Class() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}

#define EXPORT_CLASS_I1(Name, Class, I1, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Name(#Name, typeid(Base).name()); \
	DLLEXPORT Base *create ## Name() { return new Class(I1); } \
	DLLEXPORT int version ## Name() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}

#define EXPORT_CLASS_I2(Name, Class, I1, I2, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Name(#Name, typeid(Base).name()); \
	DLLEXPORT Base *create ## Name() { return new Class(I1, I2); } \
	DLLEXPORT int version ## Name() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}

#define EXPORT_CLASS_I3(Name, Class, I1, I2, I3, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Name(#Name, typeid(Base).name()); \
	DLLEXPORT Base *create ## Name() { return new Class(I1, I2, I3); } \
	DLLEXPORT int version ## Name() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}

#define EXPORT_CLASS_I4(Name, Class, I1, I2, I3, I4, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Name(#Name, typeid(Base).name()); \
	DLLEXPORT Base *create ## Name() { return new Class(I1, I2, I3, I4); } \
	DLLEXPORT int version ## Name() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}

#define EXPORT_CLASS_I5(Name, Class, I1, I2, I3, I4, I5, MajorVersion, MinorVersion, TinyVersion, Base) \
extern "C" \
{ \
	DLLEXPORT Register reg ## Name(#Name, typeid(Base).name()); \
	DLLEXPORT Base *create ## Name() { return new Class(I1, I2, I3, I4, I5); } \
	DLLEXPORT int version ## Name() { return (((MajorVersion << 8) + MinorVersion) << 8) + TinyVersion; }; \
}
