/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <iostream>
#include <cstdlib>
using namespace std;

#include <GAT.hpp>
using namespace GAT;

#include <qapplication.h>
#include <q3ptrvector.h>
#include <qstringlist.h>

#include "qsubapp.h"

#include "rgeddei.h"
using namespace rGeddei;

#include "networkspec.h"

#define MESSAGES 0

const char *thisHostname = "144.32.161.11";

class MySubApp: public QSubApp { void main(); } subApp;

// Should be up here since we want it running in the main thread along with the QApplication.
LocalSession local(thisHostname);

void MySubApp::main()
{
	// take a .net file as argv[1]
	if(qApp->argc() < 2) qFatal("Require an argument for the net file!");
	NetworkSpec net(qApp->argv()[1]);
	
	// print it out
	cout << net;
	
	// setup gat envorinment
	Self::SetExecutionEnvironment(qApp->argc(), qApp->argv());
	Context context;

	// for now we'll appoint a node per processor
	cout << net.realCount() << " nodes required..." << endl;
	
	// we need to ask gat for the nodes:
	// first, let's find what's available...
	ResourceBroker broker(context);
	HardwareResourceDescription hrd(ConstructTable("memory.size", 0.256f)("disk.size", 10.f)("cpu.speed", 1.0f)("cpu.type", "unknown")("machine.type", "i686")/*("machine.node", "fun")*/);
	list<HardwareResource> found;
//	broker.FindResources(hrd, found);
	
	// and print them out...
	//for(list<HardwareResourceDescription>::iterator i = found.begin(); i != found.end(); i++)
	//	qDebug("Resource %s", (*i).GetDescription().Get<String>("machine.name").GetBuffer());
	
	// then we need to select the ones we want to use
	QStringList nodes;
	// we'll cheat until get starts working...
	for(uint i = 0; i < net.realCount(); i++) nodes << "localhost";
	
	// and reserve them for use
	// then submit the jobs to the reservations.
	// then wait until everyone has registered with us
	// now we can connect to their nodeservers with RemoteSessions
	Q3ValueVector<RemoteSession *> sessions(nodes.count());
	int ii = 0;
	for(QStringList::iterator i = nodes.begin(); i != nodes.end(); i++,ii++)
		sessions[ii] = new RemoteSession(*i);
	
	// then we need to create the RemoteProcessors, and our LocalProcessors for data in/out
	AbstractProcessorGroup group;
	QStringList names = net.names();
	ii = 0;
	for(QStringList::iterator i = names.begin(); i != names.end(); i++,ii++)
	{	ProcessorSpec p = net[*i];
		if(p.family() == ProcessorSpec::Factory || p.family() == ProcessorSpec::SubFactory)
			(AbstractProcessor::create(*(sessions[ii]), p.type()))->init(p.name(), group, p.properties());
	}
	// then we just connect them up and go!
}
