/***************************************************************************
                          kohonennet.cpp  -  description
                             -------------------
    begin                : Sun Jan 9 2000
    copyright            : (C) 2000 by Gavin James Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

#include <qfile.h>
#include <q3textstream.h>

#include "qpca.h"
#include "qkohonennet.h"
using namespace QtExtra;

uint random(uint low, uint high)
{
	return (rand() % (high - low)) + low;
}

inline double random(double low, double high)
{
	return (double(high - low) * double(rand()) / double(RAND_MAX)) + double(low);
}

void initRandom()
{
	QFile fin("/dev/random");
	if(fin.open(QIODevice::ReadOnly))
	{
		uint a = fin.getch(), b = fin.getch(), c = fin.getch();
		srand(a + b * 256 + c * 65536);
	}
}

void QKohonenNet::readCod(QString const& file)
{
	QFile f(file);
	if(!f.open(QIODevice::ReadOnly)) return;
	Q3TextStream fin(&f);
	
	uint w, h, d;
	fin >> w >> h >> d;

	resize(w, h, d);
	for(uint i = 0; !fin.atEnd() && i < w * h * d; i++)
		fin >> theData[i];
}

void QKohonenNet::writeCod(QString const& file)
{
	QFile f(file);
	if(!f.open(QIODevice::WriteOnly)) return;
	Q3TextStream fout(&f);
	fout << theWidth << " " << theHeight << " " << theDimensions << endl;
	for(uint i = 0; i < theWidth * theHeight * theDimensions; i++)
		fout << theData[i] << " ";
}

void QKohonenNet::init(uint width, uint height, QFeatures const& _f)
{
	resize(width, height, _f.eigenVectors().ncols());
	
	for(uint y = 0; y < theHeight; y++)
		for(uint x = 0; x < theWidth; x++)
			for(uint i = 0; i < theDimensions; i++)
			{	float xt = float(x)/float(theWidth-1) * 2.f - 1.f;
				float yt = float(y)/float(theHeight-1) * 2.f - 1.f;
				value(x, y, i) = _f.eigenValues()[0] * xt * _f.eigenVectors()[i][0] + 
									_f.eigenValues()[1] * yt * _f.eigenVectors()[i][1] +
									_f.means()[i];
			}
}

void QKohonenNet::init(uint width, uint height, uint dims)
{
	resize(width, height, dims);
	initRandom();
	for(uint i = 0; i < theWidth * theHeight * theDimensions; i++)
		theData[i] = random(0.f, 1.f);
}

uint QKohonenNet::train(const Matrix &data, uint width, uint height, uint iterations, double alpha, double diameter, double k, bool const _randomOrder, bool const _batch, bool const _verbose)
{
	init(width, height, data.ncols());
	return learn(data, iterations, alpha, diameter, k, _randomOrder, _batch, _verbose);
}

/*
bool QKohonenNet::learn(const Matrix &data, uint iterations, double alpha, double diameter, double k, bool const _randomOrder)
{
	if(!iterations) return false;
	if(_randomOrder) initRandom();

	double lambda = exp(-k);
	int curItem = 0;
	int index = 0;
	for(uint t = 0; t < iterations; t++, curItem++)
	{
		if(curItem >= data.nrows())
		{	curItem = 0;
			diameter *= lambda;
			alpha *= lambda;
		}
		index = _randomOrder ? random() % data.nrows() : curItem;
		update(data[index], alpha, diameter);
	}
	return true;
}*/

uint QKohonenNet::learn(const Matrix &data, uint iterations, double alpha, double diameter, double k, bool const _randomOrder, bool const _batch, bool const _verbose)
{
	double lambda = exp(-k);
	uint t = 0;
	if(_batch)
	{
		bool changed = true;
		while(changed && ++t < iterations)
		{
			Real avgFit[theWidth][theHeight][theDimensions];
			uint avgFitCount[theWidth][theHeight];
			for(uint y = 0; y < theHeight; y++)
				for(uint x = 0; x < theWidth; x++)
				{	for(uint i = 0; i < theDimensions; i++)
						avgFit[x][y][i] = 0.0;
					avgFitCount[x][y] = 0;
				}
			
			for(int d = 0; d < data.nrows(); ++d)
			{	uint x, y;
				closest(data[d], x, y);
				for(uint i = 0; i < theDimensions; ++i)
					avgFit[x][y][i] += data[d][i];
				avgFitCount[x][y]++;
			}
			
			for(uint y = 0; y < theHeight; y++)
				for(uint x = 0; x < theWidth; x++)
					for(uint i = 0; i < theDimensions; i++)
						avgFit[x][y][i] /= Real(avgFitCount[x][y]);
			
			Real hji;
			Real tc = 0.0;
			Real mc = 0.0;
			uint cn = 0;
			for(uint iy = 0; iy < theHeight; iy++)
				for(uint ix = 0; ix < theWidth; ix++)
				{
					Real denom = 0.0, num[theDimensions];
					for(uint i = 0; i < theDimensions; ++i) num[i] = 0.0;
					for(uint jy = 0; jy < theHeight; jy++)
						for(uint jx = 0; jx < theWidth; jx++)
							if((hji = proximity(ix, iy, jx, jy, diameter)) > 0.0 && avgFitCount[jx][jy])
							{	denom += hji * Real(avgFitCount[jx][jy]);
								for(uint i = 0; i < theDimensions; ++i)
									num[i] += hji * Real(avgFitCount[jx][jy]) * avgFit[jx][jy][i];
							}
					if(denom > 0.0)
					{
						cn++;
						Real cc = 0.0;
						for(uint i = 0; i < theDimensions; ++i)
						{	cc += (value(ix, iy, i) - num[i] / denom) * (value(ix, iy, i) - num[i] / denom);
							value(ix, iy, i) = num[i] / denom;
						}
						cc = sqrt(cc);
						tc += cc;
						if(cc > mc) mc = cc;
					}
				}
			diameter *= lambda;
			if(_verbose)
			{	printf(mc > 10 ? "!" : mc > 1 ? "#" : mc > 0.1 ? "O" : mc > .01 ? "o" : mc > .004 ? "," : ".");
				fflush(stdout);
			}
			changed = mc > alpha;
		}
	}
	else
	{
		uint perm[data.nrows()];
		for(int i = 0; i < data.nrows(); ++i) perm[i] = i;
		for(int i = 0, d = 1; i < data.nrows(); ++i, d = random()%data.nrows())
			perm[i] ^= perm[d] ^= perm[i] ^= perm[d];
		while(++t < iterations)
		{	
			alpha *= lambda;
			diameter *= lambda;
			for(int i = 0; i < data.nrows(); ++i)
				update(data[_randomOrder ? random() % data.nrows() : perm[i]], alpha, diameter);
		}
	}
	return t;
}
	
ReturnMatrix QKohonenNet::project(const Matrix &data)
{
	Matrix s(data.nrows(), 2);
	
	for(int i = 0; i < data.nrows(); i++)
	{
		uint x, y;
		closest(data[i], x, y);
		s[i][0] = float(x) / float(theWidth - 1);
		s[i][1] = float(y) / float(theHeight - 1);
	}
	
	s.release();
	return s;
}

void QKohonenNet::update(const Real *vector, double alpha, double diameter)
{
	uint closestX, closestY;
    closest(vector, closestX, closestY);
	for(uint x = 0; x < theWidth; x++)
		for(uint y = 0; y < theHeight; y++)
		{	double p = proximity(closestX, closestY, x, y, diameter);
			if(p < 0.001) continue;
			for(uint i = 0; i < theDimensions; i++)
				value(x, y, i) += alpha * p * (vector[i] - value(x, y, i));
		}
}

double QKohonenNet::closest(const Real *vector, uint &returnX, uint &returnY) const
{
    double minDist = 1.0e90;
	for(uint x = 0; x < theWidth; x++)
		for(uint y = 0; y < theHeight; y++)
		{	double distance = 0;
			for(uint d = 0; d < theDimensions; d++)
			{	double thisDist = vector[d] - value(x, y, d);
				distance += thisDist * thisDist;
			}
			if(distance < minDist)
			{	minDist = distance;
				returnX = x;
				returnY = y;
			}
		}
	
	return minDist;
}

double QKohonenNet::distance(const float *vector, uint x, uint y) const
{
	double dist = 0;
	for(uint d = 0; d < theDimensions; d++)
	{	double thisDist = vector[d] - value(x, y, d);
		dist += thisDist * thisDist;
	}
	return sqrt(dist);
}

double QKohonenNet::proximity(int x1, int y1, int x2, int y2, double diameter) const
{
	return exp(-Real((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))/(2.0 * diameter * diameter));
//    return ((abs((signed)(x1 - x2)) <= diameter) && (abs((signed)(y1 - y2)) <= diameter)) ? 1 : 0;
}
