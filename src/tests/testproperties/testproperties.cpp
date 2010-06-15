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

#include "properties.h"
using namespace Geddei;

int main()
{
	std::cout << "First run..." << std::endl;
	{
		Properties p = Properties("X", 1)("Y", 3.1415)("Z", "Gav Wood!");
		std::cout << "X: " << p["X"].toInt() << std::endl;
		std::cout << "Y: " << p["Y"].toDouble() << std::endl;
		std::cout << "Z: " << qPrintable(p["Z"].toString()) << std::endl;
	}
}



