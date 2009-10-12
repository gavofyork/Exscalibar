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

#include <QBuffer>
#include <QDataStream>
#include <QVariant>
#include <QMap>
#include <QStringList>

#include <exscalibar.h>

class NodeServerSession;

namespace rGeddei { class RemoteSession; }

namespace Geddei
{

class PropertiesInfo;

/** @ingroup Geddei
 * @brief A set of name/value pairs that act as attributes to a Processor object.
 * @author Gav Wood <gav@kde.org>
 *
 * Properties is a simple class to store a set of name-value pairs. The values
 * are allowed to be of any valid Qt type.
 *
 * The only real "magic" is its cool convenience construction method from a
 * string.
 */
class DLLEXPORT Properties
{
	friend class RSCoupling;
	friend class DRCoupling;
	friend class rGeddei::RemoteSession;
	friend class ::NodeServerSession;

	QMap<QString, QVariant> theData;

	/**
	 * Save the mapping to a QBuffer object.
	 *
	 * @param data The buffer to store the mapping.
	 */
	void toBuffer(QBuffer &data) const;

	/**
	 * Load the mapping from a QBuffer object.
	 *
	 * @param data The buffer form which to retrieve the mapping.
	 */
	void fromBuffer(QBuffer &data);

	/**
	 * Save this Properties object (i.e. the mappings) to a QByteArray.
	 *
	 * @return A QByteArray containing all the data stored in this object.
	 */
	QByteArray serialise() const;

	/**
	 * Load this Properties object (i.e. the mappings) from a QByteArray.
	 *
	 * @param data A QByteArray containing all the data to be stored in this
	 * object.
	 */
	void deserialise(QByteArray &data);

	/**
	 * Constructor for a roperties object from a serialise()-ed Properties
	 * object's QByteArray.
	 *
	 * @param a The QByteArray from which to deserialise() the object.
	 *
	 * @sa serialise() @sa deserialise()
	 */
	Properties(QByteArray &a) { deserialise(a); }

public:
	/**
	 * Retrieve a value from the mapping.
	 *
	 * Example, assuming P is an already populated Properties object:
	 *
	 * @code
	 * QString s = P.get("stringKey").toString();
	 * int i = P.get("intKey").toInt();
	 * @endcode
	 *
	 * @param key The key whose value is to be retrieved.
	 * @return A QVariant object (which acts similarly to a union of all types
	 * supported) containing the data mapped to @a key .
	 */
	QVariant get(const QString &key) const { return theData[key]; }

	Properties stashed() const { Properties ret; foreach (QString k, keys()) ret[":" + k] = get(k); return ret; }
	Properties unstash() { Properties ret; foreach (QString k, keys()) if (k.startsWith(":")) { ret[k.mid(1)] = get(k); remove(k); } return ret; }
	Properties operator+(Properties const& _p) const { Properties ret(_p); ret.theData.unite(theData); return ret; }
	void defaultFrom(const PropertiesInfo& _i);

	/**
	 * Set a value in the mapping.
	 *
	 * Example:
	 *
	 * @code
	 * Properties P;
	 * QString s = P.set("stringKey", "Hello world");
	 * int i = P.get("intKey", 69);
	 * @endcode
	 *
	 * @param key The key of the value to be set.
	 * @param value The value intended for key @a key .
	 */
	void set(const QString &key, const QVariant value) { theData[key] = value; }

	void remove(QString const& _key) { theData.remove(_key); }

	/** @internal
	 * Destructively merge key-value pairs into the mapping.
	 *
	 * That is, any keys that exist in both this Properties mapping and
	 * @a pairs will be overwritten with @a pairs value.
	 *
	 * @param pairs The Properties with which to write over this.
	 */
	void set(const Properties &pairs);

	/** @internal
	 * Get the number of keys in this object.
	 *
	 * @return The number of keys in this object.
	 */
	uint size() const { return theData.size(); }

	/** @internal
	 * Get a list of the keys in this object.
	 *
	 * @return A QStringList containing an entry of each key in this object.
	 */
	const QStringList keys() const { return theData.keys(); }

	/**
	 * Subscript operator giving access to the mapping.
	 *
	 * Functions the same as get().
	 *
	 * @param key The key of the value to be retrieved.
	 * @return The value whose key is @a key .
	 *
	 * @sa get()
	 */
	QVariant operator[](const QString &key) const { if (!theData.contains(key)) qWarning("*** WARNING: Reading undefined property (%s)", qPrintable(key)); return theData[key]; }

	/**
	 * Subscript operator giving full access to the mapping. This can be used
	 * for writing to the mapping.
	 *
	 * Example; both lines are semanticly equivalent:
	 *
	 * @code
	 * P.set("stringKey", "Boo!");
	 * P["stringKey"] = "Boo!";
	 * @endcode
	 *
	 * @param key The key of the value to be retrieved.
	 * @return A reference to the value whose key is @a key .
	 *
	 * @sa set()
	 */
	QVariant &operator[](const QString &key) { return theData[key]; }

	/**
	 * Operator to add a key value pair to an existing Properties object.
	 *
	 * Typical use would be with a constructor:
	 *
	 * @code
	 * Properties p = Properties("a", 1)("b", 2)("c", 3);
	 * @endcode
	 *
	 * It may be used similarly inline when calling methods with Properties
	 * arguments:
	 *
	 * @code
	 * someObject.someMethod(Properties("a", 1)("b", 2)("c", 3));
	 * @endcode
	 *
	 * @param key The key of the pair to be added.
	 * @param value The value of the pair to be added.
	 * @return A reference to this, with the new pair having been added.
	 */
	Properties &operator()(const QString &key, const QVariant &value) { set(key, value); return *this; }

	/**
	 * Constructor for initialising a Properties object with one key/value pair
	 * in it.
	 *
	 * @param key The key of the key/value pair that will be in the object.
	 * @param value The value of the key/value pair that will be in the object.
	 */
	Properties(const QString &key, const QVariant &value) { set(key, value); }

	/**
	 * Basic constructor. Creates an empty Properties object.
	 */
	Properties() {}

	/** @internal
	 * Constructor to create a Properties object from a PropertiesInfo
	 * instance. This just throws away any extra data the PropertiesInfo stores
	 * and assigns the PropertiesInfo defaults into the values of this object.
	 *
	 * @param info The PropertiesInfo object from which this will be populated.
	 */
	Properties(const PropertiesInfo &info);
};

/** @internal
 * Class to hold any extra data for each Property of the the PropertiesInfo
 * class.
 */
struct PropertiesDatum
{
	QString description;
};

/** @ingroup Geddei
 * @brief A set of tuples to describe the properties for a Processor object.
 * @author Gav Wood <gav@kde.org>
 *
 * PropertiesInfo can be thought of as a map between names and certain
 * attributes for each property. Those attributes are a default value (which
 * implicitly stores type information) and a string description of the what
 * the property does.
 *
 * It comes with a nice inline construction mechanism to create instances
 * with an unlimited number of tuples.
 */
class DLLEXPORT PropertiesInfo: private Properties
{
	friend class Properties;

	QMap<QString, PropertiesDatum> theInfo;

public:
	/**
	 * Get the number of keys in this object.
	 *
	 * @return The number of keys in this object.
	 */
	uint size() const { return Properties::size(); }

	PropertiesInfo stashed() const { PropertiesInfo ret; foreach (QString k, keys()) ret.set(":" + k, Properties::get(k), theInfo[k].description); return ret; }
	PropertiesInfo unstashed() { PropertiesInfo ret; foreach (QString k, keys()) if (k.startsWith(":")) { ret.set(k.mid(1), Properties::get(k), theInfo[k].description); remove(k); } return ret; }
	PropertiesInfo destash() { PropertiesInfo ret = *this; (*this) = ret.unstashed(); return ret; }
	PropertiesInfo operator+(PropertiesInfo const& _p) const { PropertiesInfo ret(_p); ret.Properties::operator+(*this); ret.theInfo.unite(theInfo); return ret; }

	void remove(QString const& _k) { Properties::remove(_k); theInfo.remove(_k); }

	/**
	 * Get a list of the keys in this object.
	 *
	 * @return A QStringList containing an entry of each key in this object.
	 */
	QStringList keys() const { return Properties::keys(); }

	/**
	 * Inserts (or overwrites) a set of new tuples into this object.
	 *
	 * Typical usage:
	 *
	 * @code
	 * PropertiesInfo a, b;
	 * // a & b are populated...
	 * PropertiesInfo p;
	 * p.set(a);
	 * p.set(b);
	 * // p now contains all properties from a and b.
	 * @endcode
	 *
	 * @param merge The PropertiesInfo object from which to draw data.
	 * @return A reference to this object.
	 *
	 * @sa operator()()
	 */
	void set(const PropertiesInfo &merge) { for (QMap<QString, PropertiesDatum>::const_iterator i = merge.theInfo.begin(); i != merge.theInfo.end(); i++) theInfo[i.key()] = i.value(); Properties::set(merge); }

	/**
	 * Inserts (or overwrites) a new tuple into this object.
	 *
	 * @param key The name of the property the new tuple will describe.
	 * @param defaultValue The default value this property will take.
	 * @param description A description of what this property means.
	 *
	 * @sa operator()()
	 */
	void set(const QString &key, const QVariant defaultValue, const QString &description) { Properties::set(key, defaultValue); theInfo[key].description = description; }

	/**
	 * Retrieve the default value for a given property.
	 *
	 * @param key The name of the property.
	 * @return The default value for the property named @a key.
	 */
	QVariant defaultValue(const QString &key) const { return Properties::get(key); }

	/**
	 * Retrieve the description of a given property.
	 *
	 * @param key The name of the property.
	 * @return The description of property @a key.
	 */
	const QString description(const QString &key) const;

	/**
	 * Inserts (or overwrites) a new tuple into this object.
	 *
	 * This should only be used in conjunction with the tuple constructor,
	 * since otherwise it can make code quite cryptic.
	 *
	 * Typical usage:
	 *
	 * @code
	 * PropertiesInfo p = PropertiesInfo("a", 1, "A's value")
	 *                                  ("b", 2, "B's value");
	 * @endcode
	 *
	 * It may be used similarly inline when calling methods with Properties
	 * arguments:
	 *
	 * @code
	 * someObject.someMethod(PropertiesInfo("a", 1, "A's value")
	 *                                     ("b", 2, "B's value"));
	 * @endcode
	 *
	 * @param key The name of the property the new tuple will describe.
	 * @param defaultValue The default value this property will take.
	 * @param description A description of what this property means.
	 * @return A reference to this object.
	 *
	 * @sa set()
	 */
	PropertiesInfo &operator()(const QString &key, const QVariant defaultValue, const QString &description) { set(key, defaultValue, description); return *this; }

	/**
	 * Inserts (or overwrites) a set of new tuples into this object.
	 *
	 * This should only be used in conjunction with the tuple constructor,
	 * since otherwise it can make code quite cryptic.
	 *
	 * Typical usage:
	 *
	 * @code
	 * PropertiesInfo a, b;
	 * // a & b are populated...
	 * PropertiesInfo p = PropertiesInfo(a)(b);
	 * // p now contains all properties from a and b.
	 * @endcode
	 *
	 * @param merge The PropertiesInfo object from which to draw data.
	 * @return A reference to this object.
	 *
	 * @sa set()
	 */
	PropertiesInfo &operator()(const PropertiesInfo &merge) { set(merge); return *this; }

	/**
	 * Constructor to create a PropertiesInfo object already populated with a
	 * tuple.
	 *
	 * This can be utilised with the parentheses operator to construct objects
	 * with multiple tuples.
	 *
	 * @param key The name of the property the populated tuple will describe.
	 * @param defaultValue The default value this property will take.
	 * @param description A description of what this property means.
	 */
	PropertiesInfo(const QString &key, const QVariant defaultValue, const QString &description) { set(key, defaultValue, description); }

	/**
	 * Basic constructor. Creates an empty Properties object.
	 */
	PropertiesInfo() {}
};

}
