#define __GEDDEI_BUILD

#include <iostream>

#include "processor.h"
#include "connection.h"
#include "processorgroup.h"
#include "processorfactory.h"
#include "monitor.h"
using namespace geddei;

int main()
{
	Processor	*O = ProcessorFactory::create("Player"),
				*A = ProcessorFactory::create("Add");
	Monitor		*W = new Monitor;

	ProcessorGroup objects;
	O->init("O", objects);
	A->init("A", objects);
	W->init("W", objects);

	while(1)
	{
		std::cout << "Connecting..." << std::endl;
		(*O).share(0, 131072);
		O->connect(0, A, 0);
		O->connect(0, A, 1);
/*		O->split(0);
		(*O)[0] >>= (*A)[0].setSize(131072);
		(*O)[0] >>= (*A)[1].setSize(131072);
	*/	(*A)[0] >>= (*W)[0].setSize(131072);

		std::cout << "Checking network..." << std::endl;
		objects.confirmTypes();

		std::cout << "Starting objects..." << std::endl;
		objects.go();

		std::cout << "Waiting 1 seconds..." << std::endl;
		sleep(1);

		std::cout << "Stopping objects..." << std::endl;
		objects.stop();

		std::cout << "Disconnecting..." << std::endl;
		objects.disconnectAll();
		
		std::cout << "Elements processed: " << W->elementsProcessed() << std::endl;
	}
}
