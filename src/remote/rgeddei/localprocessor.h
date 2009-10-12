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

#include <qstring.h>

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "processor.h"
#include "abstractprocessor.h"
#include "abstractprocessorgroup.h"
#else
#include <geddei/processor.h>
#include <rgeddei/abstractprocessor.h>
#include <rgeddei/abstractprocessorgroup.h>
#endif
using namespace Geddei;
using namespace rGeddei;

namespace rGeddei
{

/** @ingroup rGeddei
 * @brief AbstractProcessor realisation representing a local Processor object.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is used when a Geddei Processor object needs to be introduced
 * into the rGeddei API system. This is generally useful in three situations;
 * either you have a Geddei Processor class with extra functionality on its
 * interface that simply cannot be accessed over rGeddei, or you have a locally
 * defined Geddei Processor object that you don't want to have to propogate to
 * the plugin system to use (perhas because you're busy in development), or
 * finally perhaps you want to use Processor objects on the local machine but
 * don't want to have to run a whole other nodeserver.
 *
 * Because of this, rGeddei allows locally created Geddei Processor objects to
 * be introduced directly into rGeddei with no Factory/plugin middleman.
 */
class DLLEXPORT LocalProcessor : virtual public AbstractProcessor
{
	friend class AbstractProcessor;
	friend class LocalSession;
	friend class RemoteProcessor;
	friend class LocalDomProcessor;
	friend class RemoteDomProcessor;

	/**
	 * Basic constructor. Constructs a LocalProcessor object that adopts the
	 * given Processor object.
	 *
	 * @note This object will take ownership of @a processor . You should not
	 * attempt to delete it or other control any of the base Geddei
	 * functionality. Use this class for that control instead. Typically you
	 * will use this constructor with a "new" operator:
	 *
	 * @code
	 * LocalProcessor l(mySession, new MyCustomProcessorSubClass);
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param processor A pointer to the processor which this LocalProcessor
	 * object will take ownership.int maxBand
	 */
	LocalProcessor(LocalSession &session, Processor *processor);

	/** @overload
	 * Basic constructor. Constructs a LocalProcessor object that uses, but
	 * does not adopt the given Processor object.
	 *
	 * @note This object will not take ownership of @a processor . You must
	 * delete it when finished with.
	 *
	 * However, you should not attempt to control any of the base Geddei
	 * functionality directly. Use this class for that control instead.
	 *
	 * Example:
	 *
	 * @code
	 * MyCustomProcessor p;
	 * LocalProcessor l(mySession, p);
	 *
	 * // Correct:
	 * l.init();
	 *
	 * // Wrong:
	 * //p.init();
	 * @endcode
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param processor A reference to the processor which this LocalProcessor
	 * object will...
	 */
	LocalProcessor(LocalSession &session, Processor &processor);

	/** @overload
	 * Basic constructor. Constructs a LocalProcessor object together with its
	 * associated (and owned) Processor object.
	 *
	 * @param session The LocalSession under which to create the Processor
	 * object.
	 * @param type The Processor-derived subclass to be created. The resident
	 * ProcessorFactory singleton will be used for creation, so the Processor
	 * derived subclass must be available as a plugin.
	 */
	LocalProcessor(LocalSession &session, const QString &type);

protected:
	/** @internal The session we are associated with. */
	LocalSession *theSession;
	/** @internal The Processor object we represent. */
	Processor *theProcessor;
	/** @internal If true, we own theProcessor, and it must be deleted. */
	bool theAdopted;

	//* Reimplementations from AbstractProcessor.
private:
	virtual void doInit(const QString &name, AbstractProcessorGroup *g, const Properties &p);
public:
	virtual bool go() { return theProcessor->go(); }
	virtual Processor::ErrorType waitUntilGoing(int *errorData = 0) { return theProcessor->waitUntilGoing(errorData); }
	virtual void waitUntilDone() { theProcessor->waitUntilDone(); }
	virtual void stop() { theProcessor->stop(); }
	virtual void reset() { theProcessor->reset(); }
	virtual void share(uint sourceIndex) { theProcessor->share(sourceIndex); }
	virtual void split(uint sourceIndex) { theProcessor->split(sourceIndex); }
	virtual bool connect(uint sourceIndex, const RemoteProcessor *sink, uint sinkIndex, uint bufferSize = 1);
	virtual bool connect(uint sourceIndex, const LocalProcessor *sink, uint sinkIndex, uint bufferSize = 1);
	virtual void disconnect(uint sourceIndex);
	virtual void disconnectAll();
	virtual const QString name() const { return theProcessor->name(); }

	/**
	 * Default destructor.
	 */
	virtual ~LocalProcessor();
};

}
