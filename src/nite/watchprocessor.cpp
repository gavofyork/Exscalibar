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

#if 0

#include <cmath>
#include <QPixmap>
using namespace std;

#include "watchprocessor.h"

#include "buffer.h"
using namespace Geddei;

#include <QDateTime>
#include <QStatusBar>
#include <QApplication>
#include <QPainter>
#include <QLabel>
#include <QTimer>

#include "geddeinite.h"

template<class A> A nmin(A a, A b) { return a > b ? b : a; }
template<class A> A nmax(A a, A b) { return a < b ? b : a; }

WatchProcessor::WatchProcessor(GeddeiNite *gn) : Processor("WatchProcessor")
{
	theWidth = 512;
	theHeight = 128;
	theGeddeiNite = gn;
	theDockWatch = new QDockWidget("Watch", theGeddeiNite);
	theLabel = new QLabel(theDockWatch);
	QPixmap p;
	p.resize(theWidth, theHeight);
	p.fill();
	theLabel->setPixmap(p);
	theLabel->adjustSize();
	theLabel->show();
//	theDockWatch->setHorizontallyStretchable(true);
//	theDockWatch->setVerticallyStretchable(true);
//	theDockWatch->setResizeEnabled(true);
	gn->addDockWidget(Qt::BottomDockWidgetArea, theDockWatch);
	theDockWatch->setWidget(theLabel);
	theDockWatch->adjustSize();
	theDockWatch->show();
	theTimer = new QTimer(this);
	QObject::connect(theTimer, SIGNAL(timeout()), this, SLOT(repaint()));
	theTimer->start(100);
}

WatchProcessor::~WatchProcessor()
{
	delete theTimer;
	delete theDockWatch;
}

void WatchProcessor::repaint()
{
	if (theThroughput)
	{
//		qDebug("Throughput = %d sps", theThroughput * 10);
//		qDebug("Drawing time = %d ms", theDrawingTime);
//		qDebug("Peek/Read+Out = %d/%d ms", theReadTime, theOutTime);
		theDrawingTime = 0;
		theThroughput = 0;
		theReadTime = 0;
		theOutTime = 0;
		theLabel->update();
	}
//	else qDebug("-");
}

bool WatchProcessor::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	outTypes[0] = inTypes[0];
	if (inTypes[0].isA<Value>())
		theWatcher = 0;
	else if (inTypes[0].isA<Wave>())
		theWatcher = 1;
	else if (inTypes[0].isA<Spectrum>())
		theWatcher = 2;
	else if (inTypes[0].isA<SquareMatrix>())
		theWatcher = 3;
	theFrequency = inTypes[0].frequency();
	theScope = inTypes[0].scope();
	return true;
}

void WatchProcessor::watchWave()
{
	uint width = theWidth, height = theHeight;
	uint widthM1 = width - 1, heightM1 = height - 1;
	QPainter *p;
	float min = -1, max = 1;
	QTime t;
	t.start();
	while (true)
	{
		{
		t.restart();
		const BufferData d = input(0).peekSecond();
		theReadTime += t.elapsed();
		if (!theThroughput)
		{
		if (min == max) min = max = d(0, 0);
		for (uint i = 0; i < d.samples(); i++)
		{
			min = ::min((d(i, 0)), min);
			max = ::max((d(i, 0)), max);
		}

		QTime t;
		t.restart();
		while (!qApp->tryLock()) usleep(1);
		QPixmap px = *theLabel->pixmap();
		px.fill();
		p = new QPainter(&px);
		p->setPen(QColor(176, 176, 176));
		p->drawLine(0, heightM1 - int((-min) / (max - min) * heightM1), widthM1, heightM1 - int((-min) / (max - min) * heightM1));
		p->setPen(QColor(0, 0, 0));
		int oy = heightM1 - int((d(0, 0) - min) / (max - min) * heightM1);
		for (uint x = 1; x < width; x++)
		{	uint y = heightM1 - int((d(x * d.samples() / width, 0) - min) / (max - min) * heightM1);
			p->drawLine(x - 1, oy, x, y);
			oy = y;
		}
		delete p;
		theLabel->setPixmap(px);
		theThroughput++;
		theDrawingTime += t.elapsed();
		qApp->unlock();
		}
		}
		t.restart();
		output(0) << input(0).readSamples();
		theOutTime += t.elapsed();
	}
}

void WatchProcessor::watchGraph()
{
	QPainter *p;
	uint width = theWidth, height = theHeight;
	uint widthM1 = width - 1, heightM1 = height - 1;
	uint speed = 2;
	int oldy = heightM1;
	float min = 0, max = 1;

	while (true)
	{
		const BufferData d = input(0).readSamples();

//		if (min > d[0]) min = d[0];
//		else if (max < d[0]) max = d[0];

		int y = (int)heightM1 - int((d[0] - min) / (max - min) * heightM1);
		y = ::max(0, ::min((int)heightM1, y));

		while (!qApp->tryLock()) usleep(1);
		QPixmap px = *theLabel->pixmap();
		bitBlt(&px, 0, 0, &px, speed, 0);
		p = new QPainter(&px);
		p->eraseRect(width - speed, 0, speed, height);
		p->setPen(QColor(176, 176, 176));
		p->drawLine(widthM1 - speed, heightM1 - int((-min) / (max - min) * heightM1), widthM1, heightM1 - int((-min) / (max - min) * heightM1));
		p->setPen(QColor(0, 0, 0));
		p->drawLine(widthM1 - speed, oldy, widthM1, y);
		delete p;
		theLabel->setPixmap(px);
		theThroughput++;
		qApp->unlock();

		oldy = y;
		output(0) << d;
	}
}

void WatchProcessor::watchSpectrograph()
{
	uint width = theWidth, height = theHeight;
	uint widthM1 = width - 1/*, heightM1 = height - 1*/;
	QPainter *p;
	uint bandWidth = theScope;
	while (true)
	{
		const BufferData d = input(0).readSample();

		while (!qApp->tryLock()) usleep(1);
		QPixmap px = *theLabel->pixmap();
		bitBlt(&px, 0, 0, &px, 1, 0);
		p = new QPainter(&px);
		for (uint y = 0; y < height; y++)
		{	float f = d[y * bandWidth / height];
			p->setPen(QColor(int(abs(f) * 360), 255, 255, QColor::Hsv));
			p->drawPoint(widthM1, y);
		}
		delete p;
		theLabel->setPixmap(px);
		theThroughput++;
		qApp->unlock();
		output(0) << d;
	}
}

void WatchProcessor::watchMatrix()
{
	uint width = theWidth, height = theHeight;
	uint size = ::min(width, height);
	QPainter *p;
	uint bandWidth = uint(sqrt(double(theScope)));

	while (true)
	{
		const BufferData d = input(0).readSamples();

		float min = d[0], max = d[0];
		for (uint i = 0; i < bandWidth * bandWidth; i++)
			if (finite(d[i]))
			{	min = ::min(d[i], min);
				max = ::max(d[i], max);
			}
		while (!qApp->tryLock()) usleep(1);
		QPixmap px = *theLabel->pixmap();
		p = new QPainter(&px);
		for (uint x = 0; x < size; x++)
			for (uint y = 0; y < size; y++)
			{	float f = (d[(x * bandWidth / size) * bandWidth + (y * bandWidth / size)] - min) / (max - min);
				f = ::max((float)0, ::min((float)1, f));
				p->setPen(QColor(0, 0, int(f * 255), QColor::Hsv));
//				p->setPen(QColor(int(f * 359), 255, 255, QColor::Hsv));
				p->drawPoint(x, y);
			}
		delete p;
		theLabel->setPixmap(px);
		theThroughput++;
		qApp->unlock();
		output(0) << d;
	}
}

void WatchProcessor::watchSpectrum()
{
	uint width = theWidth, height = theHeight;
	uint heightM1 = height - 1;
	QPainter *p;
	cin.get();
	while (true)
	{
		const BufferData d = input(0).readSamples();

		while (!qApp->tryLock()) usleep(1);
		QPixmap px = *theLabel->pixmap();
		px.fill();
		p = new QPainter(&px);
		p->setPen(QColor(0, 0, 0));
		uint bandWidth = theScope;
		for (uint i = 0; i < bandWidth; i++)
		{	uint x = i * width / bandWidth;
			uint y = uint(abs(d(0, i) * heightM1));
			p->drawLine(x, heightM1, x, heightM1 - y);
		}
		delete p;
		theLabel->setPixmap(px);
		theThroughput++;
		qApp->unlock();

		output(0) << d;
	}
}

void WatchProcessor::specifyInputSpace(QVector<uint> &samples)
{
	if (theWatcher == 0)
		samples[0] = 1;
	else if (theWatcher == 1)
		samples[0] = int(theFrequency);
	else if (theWatcher == 2)
		samples[0] = 1;
	else if (theWatcher == 3)
		samples[0] = 1;
}

void WatchProcessor::processor()
{
	if (theWatcher == 0)
		watchGraph();
	else if (theWatcher == 1)
		watchWave();
	else if (theWatcher == 2)
		watchSpectrograph();
	else if (theWatcher == 3)
		watchMatrix();
}

#endif
