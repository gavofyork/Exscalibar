#define __GEDDEI_BUILD

#include <iostream>

#include "geddei.h"
using namespace Geddei;

#include "wave.h"

int main()
{
/*	SignalTypes::Wave t;
	Buffer x(16, &t), y(16, &t);
	BufferReader a(&x), b(&x);
	x << x.makeScratchElements(5);
	x.appendPlunger();
	x.appendPlunger();
	x << x.makeScratchElements(5);
	x.appendPlunger();
	x << x.makeScratchElements(5);
	x.appendPlunger();
	x.debug();
	
	while(1)
	{
		char c;
		cout << "Enter a or b to read, A or B to skip or any other key to quit: " << flush;
		cin >> c;
		
		switch(c)
		{	case 'a': { { BufferData d = a.readElements(5); std::cout << "A" << " reads... " << (d.plunger() ? "plunger" : "") << d << std::flush; } std::cout << ". Next plunger in " << a.nextPlunger() << " elements." << std::endl; break; }
			case 'b': { { BufferData d = b.readElements(5); std::cout << "B" << " reads... " << (d.plunger() ? "plunger" : "") << d << std::flush; } std::cout << ". Next plunger in " << b.nextPlunger() << " elements." << std::endl; break; }
			case 'A': a.skipElements(5); std::cout << ". Next plunger in " << a.nextPlunger() << " elements." << std::endl; break;
			case 'B': b.skipElements(5); std::cout << ". Next plunger in " << b.nextPlunger() << " elements." << std::endl; break;
			default: exit(0);
		}
		x.debug();
	}
*/	return 0;
}
