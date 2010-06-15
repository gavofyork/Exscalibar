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

	while (1)
	{
		char c;
		cout << "Enter a or b to read, A or B to skip or any other key to quit: " << flush;
		cin >> c;

		switch (c)
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
