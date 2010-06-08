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

#include "qfactoryexporter.h"
#include "qpca.h"

#include "transmissiontype.h"
#include "value.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Rotate: public SubProcessor
{
public:
	Rotate(): SubProcessor("Rotate", InOut) {}

private:
	virtual QString simpleText() const { return QChar(0x2933); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _c) const;

	QString		m_file;
	QFeatures	m_features;
	bool		m_rowsAreVectors;
};

PropertiesInfo Rotate::specifyProperties() const
{
	return PropertiesInfo	("File", "/tmp/projection", "The file to take the rotation matrix from. { "": Use input to deliver dynamic rotation }")
							("Rows are Vectors", true, "Which way the matrix is organised.");
}

void Rotate::initFromProperties(Properties const& _p)
{
	m_file = _p["File"].toString();
	m_rowsAreVectors = _p["Rows are Vectors"].toBool();
	setupIO(Undefined, Undefined);
}

bool Rotate::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	if (!m_file.isEmpty())
	{
		QFile f(m_file);
		if (f.open(QIODevice::ReadOnly))
		{
			QTextStream ts(&f);
			// TODO: Grab rotation.
			int mp = _inTypes.count();
			NEWMAT::Matrix m(mp, mp);
			for (int y = 0; y < mp; y++)
				for (int x = 0; x < mp; x++)
				{
					if (x)
						ts.read(1);
					if (m_rowsAreVectors)
						ts >> m(x + 1, y + 1);
					else
						ts >> m(y + 1, x + 1);
				}
			m_features = QFeatures(m);
		}
	}
	else
	{
		// Will need to have n+1 inputs if dynamic :-/
		assert(false);
	}

	_outTypes = _inTypes;
	return _inTypes[0].asA<TransmissionType>().arity() == 1;
}

void Rotate::processChunks(BufferDatas const& _in, BufferDatas& _out, uint _c) const
{
	if (m_file.isEmpty())
	{
		// TODO: Grab new rotation.
	}

	if (!m_features.isEmpty())
	{
		NEWMAT::Matrix in(_c, _in.size());
		for (uint y = 0; y < _c; y++)
			for (uint x = 0; x < _in.size(); x++)
				in(y + 1, x + 1) = _in[x][y];
		NEWMAT::Matrix out = m_features.project(in, _out.size(), false, false);
		for (uint y = 0; y < _c; y++)
			for (uint x = 0; x < _out.size(); x++)
				_out[x][y] = out(y + 1, x + 1);
	}
}

EXPORT_CLASS(Rotate, 0,3,0, SubProcessor);
