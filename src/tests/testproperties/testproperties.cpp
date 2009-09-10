/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@kde.org                                                     *
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
#define __GEDDEI_BUILD

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



