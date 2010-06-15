
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

#include <math.h>

#include <Plugin>
using namespace Geddei;

class Window: public SubProcessor
{
public:
	Window() : SubProcessor("Window") {}

	enum { Hann = 0, Hamming, Rectangular, Tukey, Kaiser, Blackman, Gaussian };

private:
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(210, 96, 160); }
	virtual QString simpleText() const { return "W"; }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual void updateFromProperties();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;

	int m_size;
	int m_hop;
	int m_padding;
	bool m_zeroPhase;
	int m_type;
	float m_parameter;
	DECLARE_6_PROPERTIES(Window, m_size, m_hop, m_padding, m_zeroPhase, m_type, m_parameter);

	QVector<float> m_window;
};

PropertiesInfo Window::specifyProperties() const
{
	return PropertiesInfo("Size", 2048, "The size of the block (in samples) from which to conduct a short time Fourier transform.", false, "#", AV(32, 16384, AllowedValue::Log2))
						 ("Hop", 1024, "The number of samples between consequent sampling blocks.", false, "h", AV(1, 8192, AllowedValue::Log2))
						 ("Padding", 0, "The amount of padding in samples.", false, "p", AV("0", "0", 0) AVand(32, 262144, AllowedValue::Log2))
						 ("ZeroPhase", true, "Make window zero phase.", true, "z", AVbool)
#define W(N, n) AVand(#N, #n, N)
						 ("Type", Rectangular, "Window type.", true, "t", AV("Rectangular", "r", Rectangular) W(Hamming, h) W(Hann, n) W(Tukey, t) W(Kaiser, k) W(Blackman, b) W(Gaussian, g))
#undef W
						 ("Parameter", 0.5f, "The window type parameter.", true, "?", AV(0.f, 1.f) AVand("2", "2", 2) AVand("3", "3", 3) AVand("4", "4", 4));
}
double io(double x)
{
  const double t = 1.e-08;
  double y = 0.5*x;
  double e = 1.0;
  double de = 1.0;
  int i;
  double xi;
  double sde;
  for (i=1;i<26;i++) {
	xi = i;
	de *= y/xi;
	sde = de*de;
	e += sde;
	if ((e*t-sde) > 0) break;
  }
  return(e);
}
void Window::updateFromProperties()
{
	m_window.resize(m_size);
	switch (m_type)
	{
	case Hamming:
		for (int i = 0; i < m_size; ++i)
			m_window[i] = .54f - .46f * cos(2.f * M_PI * float(i) / float(m_size - 1));
		break;
	case Hann:
		for (int i = 0; i < m_size; ++i)
			m_window[i] = .5f * (1.f - cos(2.f * M_PI * float(i) / float(m_size - 1)));
		break;
	case Tukey:
	{
		float a = m_parameter;//0.5f;
		float OmaNo2 = (1.f - a) * m_size / 2.f;

		for (int i = 0; i < m_size / 2; ++i)
			m_window[m_size - 1 - i] = m_window[i] = (i < OmaNo2) ? .5f * (1.f - cos(M_PI * float(i) / OmaNo2)) : 1.f;
		break;
	}
	case Gaussian:
	{
		float o = m_parameter;//0.4f;
		float const Nm1o2 = (m_size - 1) / 2.f;
		float const oNm1o2 = o * Nm1o2;
		for (int i = 0; i < m_size; ++i)
			m_window[i] = exp(-.5f * sqr(((float)i - Nm1o2) / (oNm1o2)));
		break;
	}
	case Kaiser:
	{

		float a = m_parameter;//3.f;
		float const pa = M_PI * a;
		float const ToNm1 = 2.f / float(m_size - 1);
		float const jpa = io(pa);
		for (int i = 0; i < m_size / 2; ++i)
			m_window[m_size / 2 - 1 - i] = m_window[m_size / 2 + i] = io(pa * sqrt(1.f - sqr(ToNm1 * (float)i))) / jpa;

/*		 double bes = 1.0/io(M_PI * m_parameter);
		 long i;
		 long odd = m_size%2;
		 double xi;
		 double xind = (m_size-1)*(m_size-1);
		 for (i=0;i<m_size;i++) {
		   if (odd) xi = i + 0.5;
		   else xi = i;
		   xi -= (m_size - 1.f) / 2.f;
		   xi = 4*xi*xi;
		   m_window[i]  = io(M_PI * m_parameter*sqrt(1.-xi/xind))*bes;
		 }*/
		 break;
	}
	case Blackman:
	{
		float a = m_parameter;//0.16f;
		float a0 = (1.f - a) / 2.f;
		float a1 = .5f;
		float a2 = a / 2.f;
		for (int i = 0; i < m_size; ++i)
			m_window[i] = a0 - a1 * cos(2.f * M_PI * float(i) / (m_size - 1)) + a2 * cos(4.f * M_PI * float(i) / (m_size - 1));
		break;
	}
	default:;
	}
}

void Window::initFromProperties()
{
	setupIO(1, 1);
	setupSamplesIO(m_size, m_hop, 1);
}

bool Window::verifyAndSpecifyTypes(Types const& _inTypes, Types& o_outTypes)
{
	Typed<Contiguous> in = _inTypes[0];
	if (!in || in->arity() != 1)
		return false;
	o_outTypes[0] = WaveChunk(m_size + m_padding, in->frequency() / float(m_hop), in->frequency(), in->max(), in->min());
	return true;
}

void Window::processChunk(BufferDatas const& _in, BufferDatas& _out) const
{
	uint off;

	if (m_zeroPhase)
	{
		off = (m_size + 1) / 2;
		if (m_type == Rectangular)
			for (int i = 0; i < m_size; i++)
				_out[0](0, (i + off + m_padding) % (m_padding + m_size)) = _in[0](i, 0);
		else
			for (int i = 0; i < m_size; i++)
				_out[0](0, (i + off + m_padding) % (m_padding + m_size)) = _in[0](i, 0) * m_window[i];
	}
	else
	{
		off = m_size;
		if (m_type == Rectangular)
			for (int i = 0; i < m_size; i++)
				_out[0](0, i) = _in[0](i, 0);
		else
			for (int i = 0; i < m_size; i++)
				_out[0](0, i) = _in[0](i, 0) * m_window[i];
	}

	for (uint i = off; i < off + m_padding; i++)
		_out[0](0, i) = 0.f;
}

EXPORT_CLASS(Window, 1,0,0, SubProcessor);
