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

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "connection.h"
#else
#include <geddei/connection.h>
#endif

namespace Geddei
{

class ProcessorGroup;

class DLLEXPORT Groupable
{
public:
	/**
	 * The various error types that a Processor may trip up on.
	 */
	enum ErrorType
	{	NoError = 0, ///< Indicates no error occured.
		Pending, ///< Indicates the operation has yet to finish.
		Custom, ///< Indicates the overridden processorStarted returned false.
		NotInitialised, ///< Indicates processor was never initialised.
		TypesNotConfirmed, ///< Indicates the types were not confirmed before starting.
		InputTypeNull, ///< Indicates an input has been given a null type.
		InputNotConnected, ///< Indicates an input port was left unconnected.
		InputsNotHomogeneous, ///< Indicates a MultiIn type Processor has inputs of differing types.
		OutputsNull, ///< Indicates one or more outputs were left undefined.
		InvalidInputs, ///< Indicates a false return from verifyAndSpecifyTypes.
		RecursiveFailure, ///< Indicates a failure of a Processor that this depends on.
		NotStarted ///< Internal - Indicated the operation has yet to start.
	};

	/**
	 * Puts the Processor into a gvien ProcessorGroup.
	 *
	 * @param g The ProcessorGroup the Processor should become a member of.
	 *
	 * @sa setNoGroup()
	 */
	void setGroup(ProcessorGroup &g);

	/**
	 * Resets the group of the object. The processor will not be associated with any group
	 * after this call.
	 *
	 * @sa setGroup()
	 */
	void setNoGroup();

	virtual QString name() const = 0;
	virtual bool confirmTypes() = 0;
	virtual bool go() = 0;
	virtual ErrorType waitUntilGoing(int *errorData = 0) = 0;
	virtual Connection::Tristate isGoingYet() = 0;
	virtual QString error() const = 0;
	virtual ErrorType errorType() const = 0;
	virtual long int errorData() const = 0;
	virtual void stop() = 0;
	virtual void reset() = 0;
	virtual void disconnectAll() = 0;
	virtual void resetMulti() = 0;

	Groupable();
	virtual ~Groupable();

private:
	ProcessorGroup* theGroup;
};

}
