/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __RECORDER_H
#define __RECORDER_H

#include <qmutex.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qwaitcondition.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qfastwaitcondition.h"
#include "processor.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <geddei/processor.h>
#endif

/** @ingroup Toolkit
 * @author Gav Wood <gav@cs.york.ac.uk>
 * @brief Data recording sink Processor object.
 * 
 * This is the Processor class for dumping data to disk. Data is dumped in text
 * format, with customisable delimiters, determined by the properties "Field 
 * Delimiter" and "Record Delimiter".
 * 
 * You can customise what record information is output also - if property
 * "Print Section" an extra field at the start of the record with the number of
 * plungers that have passed through will be printed. If "Print Sample" is true
 * then an extra field at the start (though after the section if there is one)
 * will be printed containing the number of records before this in this section
 * there are. Together these form a two dimensional counter.
 * 
 * You can control to which file the output goes with the property "Output",
 * and you can control how many inputs may be connected with the property
 * "Inputs".
 * 
 * This is guarded, so you can use Processor::waitUntilDone() on it.
 */
class DLLEXPORT Recorder: public Processor
{
	virtual void processor();
	virtual PropertiesInfo specifyProperties() const;
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &);
	virtual void initFromProperties(const Properties &p);
	virtual void receivedPlunger();
	
	// Properties
	QFile theOutput;
	Q3TextStream stream;
	QString theFieldDelimiter, theRecordDelimiter;
	bool thePrintSection, thePrintSample, thePrintTime;
	uint thePadBefore, thePadAfter;
	
	// State
	uint theCurrentSample, theCurrentSection;
public:
	/**
	 * Basic constructor.
	 */
	Recorder() : Processor("Recorder", NotMulti, Guarded) {}
};

#endif
