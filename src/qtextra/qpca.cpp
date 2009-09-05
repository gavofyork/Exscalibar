#include <cmath>
using namespace std;

#include <exscalibar.h>

#include <qfile.h>
#include <q3textstream.h>

#include "qpca.h"
using namespace QtExtra;

ReturnMatrix QtExtra::readDump(const QString &_fn, uint const _dims)
{
	QFile f(_fn);
	if (!f.open(QIODevice::ReadOnly)) { Matrix m(0, 0); m.release(); return m; }
	uint count = f.size() / _dims;
	Matrix s(count, _dims);
	for (uint r = 0; r < count; ++r)
		for (uint c = 0; c < _dims; ++c)
			s[r][c] = float(f.getch()) / 255.f;
	s.release();
	return s;
}

void QtExtra::writeDump(const QString &_fn, const Matrix &_values)
{
	QFile o(_fn);
	if (!o.open(QIODevice::WriteOnly)) return;
	for (int s = 0; s < _values.nrows(); ++s)
		for (int f = 0; f < _values.ncols(); ++f)
			o.putch(uchar(qClamp(_values[s][f]) * 255.f));
}

void QFeatures::pca(const Matrix &_values)
{
	uint dims = _values.ncols();
	uint samples = _values.nrows();

	// get means
	m_means.resize(dims);
	for (uint x = 0; x < dims; x++)
	{
		m_means[x] = 0.f;
		for (uint s = 0; s < samples; s++)
			m_means[x] += _values[s][x];
		m_means[x] /= float(samples);
	}

	// get zero-mean samples
	Matrix adjusted(samples, dims);
	for (uint s = 0; s < samples; s++)
		for (uint x = 0; x < dims; x++)
			adjusted[s][x] = _values[s][x] - m_means[x];

	// find covariance matrix
	SymmetricMatrix covariance(dims);
	for (uint x = 0; x < dims; x++)
	{
		for (uint y = 0; y <= x; y++)
		{
			covariance[x][y] = 0.f;
			for (uint s = 0; s < samples; s++)
				covariance[x][y] += (adjusted[s][x]) * (adjusted[s][y]);
			covariance[x][y] /= float(samples - 1);
		}
	}
	
	// get eigenvalues
	eigenvalues(covariance, m_eigenValues, m_eigenVectors);
	// reverse order so biggest = first
	for (uint f = 0; f < dims / 2; f++)
	{	for (uint d = 0; d < dims; d++)
		{	double u = m_eigenVectors[d][f];
			m_eigenVectors[d][f] = m_eigenVectors[d][dims - f - 1];
			m_eigenVectors[d][dims - f - 1] = u;
		}
		double u = m_eigenValues[f];
		m_eigenValues[f] = m_eigenValues[dims - f - 1];
		m_eigenValues[dims - f - 1] = u;
	}
}

ReturnMatrix QFeatures::project(const Matrix &_values, const uint _count, bool const _normalise, bool const _clamp)
{
	Matrix features;
	features.resize(_values.ncols(), _count);
	for (uint f = 0; f < _count; f++)
		for (int d = 0; d < _values.ncols(); d++)
			features[d][f] = m_eigenVectors[d][f];
	
	Matrix v = _values.t();
	for (int d = 0; d < m_means.ncols(); d++)
		v.row(d + 1) -= m_means[d];
	Matrix ret = (features.t() * v).t();
	
	// normalise and bound by -ing mean, /ing by SD, /2 +.5, max(0, min(1, x))
	if (_normalise)
	{
		uint samples = _values.nrows();
		for (int x = 0; x < ret.ncols(); x++)
		{
			float mean = 0.f;
			for (uint s = 0; s < samples; s++)
				mean += ret[s][x];
			mean /= float(samples);
			for (uint s = 0; s < samples; s++)
				ret[s][x] -= mean;
			float sd = 0.f;
			for (uint s = 0; s < samples; s++)
				sd += ret[s][x] * ret[s][x];
			sd = sqrt(sd / float(samples));
			for (uint s = 0; s < samples; s++)
				ret[s][x] = float(ret[s][x]) / sd / 2.f + .5f;
			if (_clamp)
				for (uint s = 0; s < samples; s++)
					ret[s][x] = max(0., min(1., ret[s][x]));
		}
	}
	ret.release();
	return ret;
}

ReturnMatrix QFeatures::extrapolate(const Matrix &_projected)
{
	Matrix features;
	features.resize(m_eigenValues.ncols(), _projected.ncols());
	for (int f = 0; f < features.nrows(); f++)
		for (int d = 0; d < features.ncols(); d++)
			features[d][f] = m_eigenVectors[d][f];
	
	Matrix v = (features * _projected.t());
	for (int d = 0; d < m_means.ncols(); d++)
		v.row(d + 1) += m_means[d];
	v = v.t();
	v.release();
	return v;
}

void QFeatures::readVec(const QString &_fn)
{
	// load a feature vector
	QFile f(_fn);
	if (!f.open(QIODevice::ReadOnly)) return;
	
	uint dims;
	f.readBlock(reinterpret_cast<char*>(&dims), sizeof(uint));
	m_means.resize(dims);
	f.readBlock(reinterpret_cast<char*>(m_means.data()), sizeof(Real) * dims);
	if (!f.atEnd())
	{
		m_eigenVectors.resize(dims, dims);
		f.readBlock(reinterpret_cast<char*>(m_eigenVectors.data()), sizeof(Real) * dims * dims);
		m_eigenValues.resize(dims);
		f.readBlock(reinterpret_cast<char*>(m_eigenValues.data()), sizeof(Real) * dims);
	}
	if (f.status() != IO_Ok)
		qDebug("File error: %s", f.errorString().latin1());
	f.close();
}

void QFeatures::writeVec(const QString &_fn)
{
	QFile f(_fn);
	if (!f.open(QIODevice::WriteOnly)) return;
/*	QTextStream fout(&f);
	
	fout.precision(20);
	
	qDebug("Writing (fr=%d, fc=%d, mc=%d)", m_features.nrows(), m_features.ncols(), m_means.ncols());
	
	
	uint dims = m_features.nrows();
	uint count = m_features.ncols();
	
	if (f.status() != IO_Ok)
		qDebug("File error: %s", f.errorString().latin1());
	
	fout << dims << " " << count << endl;
	
	if (f.status() != IO_Ok)
		qDebug("File error: %s", f.errorString().latin1());
		
	// means first
	for (uint v = 0; v < dims; v++)
		fout << m_means[v] << endl << flush;
	fout << endl;
	
	if (f.status() != IO_Ok)
		qDebug("File error: %s", f.errorString().latin1());
		
	// actual features second (xposed)
	for (uint f = 0; f < count; f++)
		for (uint v = 0; v < dims; v++)
			fout << m_features[f][v] << endl << flush;
	*/
	
	uint dims = m_eigenValues.nrows();
	f.writeBlock(reinterpret_cast<char*>(&dims), sizeof(uint));
	f.writeBlock(reinterpret_cast<char*>(m_means.data()), sizeof(Real) * dims);
	f.writeBlock(reinterpret_cast<char*>(m_eigenVectors.data()), sizeof(Real) * dims * dims);
	f.writeBlock(reinterpret_cast<char*>(m_eigenValues.data()), sizeof(Real) * dims);
	if (f.status() != IO_Ok)
		qDebug("File error: %s", f.errorString().latin1());
}
