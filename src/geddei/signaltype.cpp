#define __GEDDEI_BUILD

#include "signaltype.h"
#include "qsocketsession.h"
using namespace Geddei;

#include "value.h"
#include "wave.h"
#include "spectrum.h"
#include "matrix.h"
using namespace SignalTypes;

namespace Geddei
{

SignalType::SignalType(uint scope, float frequency, float _max, float _min): theMin(_min), theMax(_max)
{
	theScope = scope;
	theFrequency = frequency;
}

void SignalType::send(QSocketSession &sink) const
{
	sink.safeSendWord((uint32_t)id());
	serialise(sink);
}

SignalType *SignalType::receive(QSocketSession &source)
{
	SignalType *s = create(source.safeReceiveWord<uint32_t>());
	s->deserialise(source);
	return s;
}

void SignalType::serialise(QSocketSession &sink) const
{
	sink.safeSendWord((uint32_t)theScope);
	sink.safeSendWord(theFrequency);
	sink.safeSendWord(theMin);
	sink.safeSendWord(theMax);
}

void SignalType::deserialise(QSocketSession &source)
{
	theScope = source.safeReceiveWord<int32_t>();
	theFrequency = source.safeReceiveWord<float>();
	theMin = source.safeReceiveWord<float>();
	theMax = source.safeReceiveWord<float>();
}

SignalType *SignalType::create(uint id)
{
	switch (id)
	{
		case 0: return new Value;
		case 1: return new Wave;
		case 2: return new Spectrum;
		case 3: return new Matrix;
		case 4: return new SquareMatrix;
		default: return 0;
	}
}

/*ostream &operator<<(ostream &out, const SignalType &me)
{
	if (me == SignalType::null)
		return out << "null";
	out << "[ " << me.theScope << "x" << me.theFrequency << "Hz - ";
	switch (me.theFormat)
	{	case SignalType::NoFormat: out << "NoFormat"; break;
		case SignalType::Wave: out << "Wave"; break;
		case SignalType::Spectrum: out << "Spectrum"; break;
		case SignalType::Matrix: out << "Matrix"; break;
		case SignalType::Unknown: out << "Unknown"; break;
	}
	return out << " ]";
}*/

}
