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

#include "subprocessor.h"
#include "abstractdomprocessor.h"
#include "remotedomprocessor.h"
#include "localdomprocessor.h"
#include "localsession.h"
#include "remotesession.h"
using namespace Geddei;
using namespace rGeddei;

namespace rGeddei
{

LocalDomProcessor *AbstractDomProcessor::create(LocalSession &session, SubProcessor *primary)
{
	return new LocalDomProcessor(session, primary);
}

LocalDomProcessor *AbstractDomProcessor::create(LocalSession &session, const QString &type)
{
	return new LocalDomProcessor(session, type);
}

RemoteDomProcessor *AbstractDomProcessor::create(RemoteSession &session, const QString &type)
{
	return new RemoteDomProcessor(session, type);
}

}
