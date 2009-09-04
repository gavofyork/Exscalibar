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
using namespace std;

#include <GAT.hpp>
using namespace GAT;

#include <qstring.h>

int main(int argc, char **argv)
{
	cout << "Invocation: ";
	for(int i = 0; i < argc; i++) cout << "\"" << argv[i] << "\" ";
	cout << endl;
	Self::SetExecutionEnvironment(argc, argv);
	Context context;
	assert(context.GetTarget() != 0);
	cout << "GAT version: " << hex << GetVersion() << endl;
	cout << "GAT C++ wrapper version: " << hex << GetWrapperVersion() << endl;
	cout << "Self-id: " << GAT::Self::GetJob(context).GetJobID().str() << endl;

	if(argc == 2 && argv[1] == QString("list"))
	{
		ResourceBroker broker(context);
		list<HardwareResource> resources = broker.FindResources<HardwareResource>(HardwareResourceDescription(ConstructTable("machine.type", "i686")));
		cout << resources.size() << " resource(s) found." << endl;
		for(list<HardwareResource>::iterator i = resources.begin(); i != resources.end(); i++)
		{
			Table t = (*i).GetResourceDescription().GetDescription();
			list<string> keys;
			t.GetKeys(keys);
			cout << "Resource: " << t.Get("machine.node") << endl;
			for(list<string>::iterator i = keys.begin(); i != keys.end(); i++)
				cout << "   " << *i << " = " << t.Get((*i).c_str()) << endl;
		}
	}
	if(argc < 2)
	{
		ResourceBroker broker(context);
		list<String> args;
		args.push_back("write");
		GAT::Job job = broker.SubmitJob(JobDescription(context,
		                                SoftwareDescription(ConstructTable("location", Location("/home/gav/Projects/exscalibar/src/tests/testgat/testgat"))("arguments", args)),
		                                HardwareResourceDescription(ConstructTable("memory.size", 0.256f)("disk.size", 10.f)("cpu.speed", 1.0f)("cpu.type", "unknown")("machine.type", "i686")("machine.node", "fun"))));
		GAT::JobID jobid = job.GetJobID();
	}
	if(argc < 2 || argv[1] == QString("read"))
	{
		cout << "Creating advert service" << endl;
		AdvertService advertService(context);
		cout << "Creating endpoint" << endl;
		Endpoint endpoint(context);
		cout << "Adding advert" << endl;
		Table t = ConstructTable("name", "myEndpoint");
		advertService.Add(endpoint, t, "exscalibar.sf.net/adverts");
		cout << "Listening for connection..." << endl;
		Pipe p = endpoint.Listen();
		cout << "Pipe setup. Reading message..." << endl;
		char msg[256];
		p.Read(msg, sizeof(msg));
		cout << msg << endl;
	}
	else if(argv[1] == QString("write"))
	{
		cout << "Checking for advert..." << endl;
		AdvertService advertService(context);
		cout << "Searching for advert..." << endl;
		list<String> found;
		for(int i = 0; i < 1000 && !found.size(); i++, found = advertService.Find(ConstructTable("GAT_TYPE","GATEndpoint")))
			usleep(1000);
		cout << "Finding endpoint..." << endl;
		Endpoint endpoint = advertService.GetAdvertisable<Endpoint>(*found.begin());
		cout << "Connecting..." << endl;
		Pipe p = endpoint.Connect();
		cout << "Pipe setup. Sending message..." << endl;
		p.Write((char *)"Hello world from Exscalibar using GAT C++!\n", 44);
	}
	return 0;
}
