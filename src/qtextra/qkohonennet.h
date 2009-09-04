/***************************************************************************
                          kohonennet.h  -  description
                             -------------------
    begin                : Sun Jan 9 2000
    copyright            : (C) 2000 by Gavin James Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

#ifndef _QTEXTRA_QKOHONENNET
#define _QTEXTRA_QKOHENENNET

#include <qfile.h>
#include <qstring.h>
#include <q3memarray.h>

#include <exscalibar.h>

#include <newmat/newmatap.h>

namespace QtExtra
{

class QFeatures;

class DLLEXPORT QKohonenNet
{
public:
	inline				QKohonenNet() : theWidth(0), theHeight(0), theDimensions(0) {}

	uint				width() const { return theWidth; }
	uint				height() const { return theHeight; }
	uint				dimensions() const { return theDimensions; }
	inline double		value(uint x, uint y, uint d) const { return theData[(y * theWidth + x) * theDimensions + d]; }
	
	void				init(uint width, uint height, uint dims);
	void				init(uint width, uint height, QFeatures const& _f);
	uint				learn(const Matrix &data, uint iterations, double alpha, double diameter, double k, bool const _randomOrder = false, bool const _batch = false, bool const _verbose = false);
	uint				train(const Matrix &data, uint width, uint height, uint iterations, double alpha, double diameter, double k, bool const _randomOrder = true, bool const _batch = false, bool const _verbose = false);
	ReturnMatrix		project(const Matrix &data);
	
	void				readCod(QString const& file);
	void				writeCod(QString const& file);

private:

	inline void			setValue(uint x, uint y, uint d, double newValue) { theData[(y * theWidth + x) * theDimensions + d] = newValue; }
	inline double &		value(uint x, uint y, uint d) { return theData[(y * theWidth + x) * theDimensions + d]; }
	
	void				resize(uint w, uint h, uint d) { theData.resize(w * h * d); theWidth = w; theHeight = h; theDimensions = d; }
	void				update(const Real *vector, double alpha, double diameter);
	double				closest(const Real *Vector, uint &returnX, uint &returnY) const;
	double				proximity(int x1, int y1, int x2, int y2, double Diameter) const;
	double				distance(const float *vector, uint x, uint y) const;

	Q3MemArray<double>	theData;
	uint				theWidth;
	uint				theHeight;
	uint				theDimensions;
};

}

#endif
