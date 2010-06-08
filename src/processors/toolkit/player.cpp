/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QPainter>

#ifdef HAVE_MAD
#undef SHRT_MAX
#include "madhelp.cpp"
#endif

#include "qfactoryexporter.h"

#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace TransmissionTypes;

#include "player.h"

#define MESSAGES 0

Player::Player(QString const& _filename): CoProcessor("Player", OutConst), m_file(_filename), m_channels(0), m_rate(0), m_length(0), m_position(0)
{
}

bool Player::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	QRectF textArea(2, 0, _s.width() - 4, 20);
	_p.setPen(Qt::black);
	_p.drawText(textArea, m_file.fileName());
	textArea.translate(1, 1);
	_p.setPen(Qt::white);
	_p.drawText(textArea, m_file.fileName());

	_p.setPen(QColor(160, 160, 160));
	_p.setBrush(QColor(232, 232, 232));
	QRectF progressArea(2, 20, _s.width() - 4, 10);
	_p.drawRect(progressArea);
	progressArea.adjust(1, 1, -1, -1);
	bool seg = false;
	for (uint i = 0; i < m_length; i += 60 * m_rate)
	{	seg = !seg;
		uint ni = i + 60 * m_rate;
		if (ni > m_length) ni = m_length;
		_p.fillRect(progressArea.x() + int(progressArea.width() * i / m_length), progressArea.y(), int(progressArea.width() * (ni - i) / m_length), progressArea.height(), QColor::fromHsv(0, 0, seg ? 232 : 255));
	}
	_p.fillRect(progressArea.x(), progressArea.y(), progressArea.width() * m_position / m_length, progressArea.height(), QColor(255, 0, 0, 64));
	return true;
}

bool Player::verifyAndSpecifyTypes(const Types &, Types &outTypes)
{
	outTypes = Wave(m_rate);
	return true;
}

void Player::specifyOutputSpace(QVector<uint> &samples)
{
	for (uint i = 0; i < m_channels; i++)
		samples[i] = m_readFrames;
}

void Player::initFromProperties(const Properties &p)
{
	m_readFrames = p["Frames"].toInt();
	m_file.setFileName(p["Filename"].toString());
	m_mode = NoMode;
	m_channels = 0;

	qDebug("Opening file %s...", qPrintable(m_file.fileName()));
#ifdef HAVE_VORBISFILE
	if (m_file.fileName().toLower().contains(".ogg"))
	{
		if (!m_file.open(QIODevice::ReadOnly))
			qWarning("*** WARNING: Cannot open file %s", qPrintable(m_file.fileName()));
		else if (ov_open(fdopen(m_file.handle(), "r"), &m_vorbisFile, NULL, 0) < 0)
			qWarning("*** WARNING: File %s does not appear to be an Ogg bitstream.", qPrintable(m_file.fileName()));
		else
		{
			vorbis_info *vi = ov_info(&m_vorbisFile, -1);
			m_channels = vi->channels;
			m_rate = vi->rate;
			m_length = (long)ov_pcm_total(&m_vorbisFile, -1);
			m_mode = ModeVF;
			ov_clear(&m_vorbisFile);
		}
	}
#endif
#ifdef HAVE_SNDFILE
	if (m_file.fileName().toLower().contains(".wav"))
	{
		SF_INFO sfinfo;
		if (!(m_sndFile = sf_open(m_file.fileName().toLocal8Bit(), SFM_READ, &sfinfo)))
			qWarning("*** WARNING: File %s cannot be read.", qPrintable(m_file.fileName()));
		else
		{
			m_length = sfinfo.frames;
			m_channels = sfinfo.channels;
			m_rate = sfinfo.samplerate;
			m_mode = ModeSF;
			sf_close(m_sndFile);
		}
	}
#endif
#ifdef HAVE_MAD
	if (m_file.fileName().toLower().contains(".mp3"))
	{
		m_inputBufferSize = (m_readFrames / 4) * 4 * 5;
		unsigned char InputBuffer[m_inputBufferSize+MAD_BUFFER_GUARD];
		if (!m_file.open(QIODevice::ReadOnly))
			qWarning("*** WARNING: Cannot open file %s", qPrintable(m_file.fileName()));
		else if (!(m_madFile = fdopen(m_file.handle(), "r")))
			qWarning("*** WARNING: File %s cannot be opened.", qPrintable(m_file.fileName()));
		else
		{
			mad_stream_init(&m_stream);
			mad_frame_init(&m_frame);
			if (!(m_bStdFile=NewBstdFile(m_madFile))) { qDebug("*** ERROR: Couldn't create bstdfile on file %s!", qPrintable(m_file.fileName())); return; }
			m_rate = 0;
			while (!m_rate || !m_channels)
			{
				if (m_stream.buffer==NULL || m_stream.error==MAD_ERROR_BUFLEN)
				{
					size_t ReadSize, Remaining;
					unsigned char *ReadStart;
					if (m_stream.next_frame)
					{
						Remaining = m_stream.bufend-m_stream.next_frame;
						memmove(InputBuffer, m_stream.next_frame, Remaining);
						ReadStart = InputBuffer + Remaining;
						ReadSize = m_inputBufferSize - Remaining;
					}
					else
						ReadSize=m_inputBufferSize, ReadStart=InputBuffer, Remaining=0;
					if ((ReadSize = BstdRead(ReadStart, 1, ReadSize, m_bStdFile)) <= 0) break;
					if (BstdFileEofP(m_bStdFile))
					{
						memset(ReadStart + ReadSize, 0, MAD_BUFFER_GUARD);
						ReadSize += MAD_BUFFER_GUARD;
					}
					mad_stream_buffer(&m_stream, InputBuffer, ReadSize + Remaining);
					m_stream.error = (mad_error)0;
				}

				if (mad_frame_decode(&m_frame, &m_stream))
				{
					if (MAD_RECOVERABLE(m_stream.error) || m_stream.error==MAD_ERROR_BUFLEN) continue; else break;
				}
				m_channels = MAD_NCHANNELS(&m_frame.header);
				m_rate = m_frame.header.samplerate;
				m_length = 1;
			}
			BstdFileDestroy(m_bStdFile);
			mad_frame_finish(&m_frame);
			mad_stream_finish(&m_stream);
			fclose(m_madFile);
			m_mode = ModeMAD;
		}
	}
#endif
	qDebug("Mode: %s, Length: %d, Channels: %d, Sampling: %d Hz", m_mode==ModeSF ? "Soundfile" : m_mode==ModeVF ? "Ogg/Vorbis" : m_mode==ModeMAD ? "MP3" : "None", m_length, m_channels, m_rate);
	m_position = 0;
	if (m_channels) setupIO(0, m_channels);
	setupVisual(160, 32, 1000);
}

PropertiesInfo Player::specifyProperties() const
{
	return PropertiesInfo("Filename", "/tmp/cdda.wav", "The name of the file to be played through the output(s)")
						 ("Frames", 1024, "The number of samples to output in each chunk.");
}

bool Player::processorStarted()
{
#ifdef HAVE_SNDFILE
	if (m_mode == ModeSF)
	{
		m_sndFile = sf_open(m_file.fileName().toLocal8Bit(), SFM_READ, &m_sfinfo);
		if (!m_sndFile) return false;
		m_buffer.resize(m_readFrames * m_channels);
		return true;
	}
#endif
#ifdef HAVE_VORBISFILE
	if (m_mode == ModeVF)
	{
		if (!m_file.open(QIODevice::ReadOnly)) return false;
		if (ov_open(fdopen(m_file.handle(), "r"), &m_vorbisFile, NULL, 0) < 0) return false;
		return true;
	}
#endif
#ifdef HAVE_MAD
	if (m_mode == ModeMAD)
	{
		if (!m_file.open(QIODevice::ReadOnly)) return false;
		m_madFile = fdopen(m_file.handle(), "r");
		m_inputBuffer.resize(m_inputBufferSize+MAD_BUFFER_GUARD);
		m_guardPointer = 0;

		mad_stream_init(&m_stream);
		mad_frame_init(&m_frame);
		mad_synth_init(&m_synth);
		mad_timer_reset(&m_timer);

		m_bStdFile = NewBstdFile(m_madFile);
		if (!m_bStdFile)
		{
			qDebug("*** ERROR: Couldn't create bstdfile on file %s!", qPrintable(m_file.fileName()));
			return false;
		}
		return true;
	}
#endif
	return false;
}

int Player::process()
{
#ifdef HAVE_SNDFILE
	if (m_mode == ModeSF)
	{
		int in = sf_readf_float(m_sndFile, m_buffer.data(), m_readFrames);
		if (in > 0)
		{
			assert((uint)in <= m_readFrames);
			m_position += in;
			for (uint i = 0; i < m_channels; i++)
			{	BufferData d = output(i).makeScratchSamples(in);
				if (!d.isNull())
					for (int j = 0; j < in; j++)
						d[j] = m_buffer[j * m_channels + i];
				output(i).push(d);
			}
		}
		else if (in == 0)
			return WillNeverWork;
		else
			sf_perror(m_sndFile);
		return DidWork;
	}
#endif
#ifdef HAVE_VORBISFILE
	if (m_mode == ModeVF)
	{
		float** buffer;
		int current_section;
		int in = ov_read_float(&m_vorbisFile, &buffer, m_readFrames, &current_section);
		if (in == 0)
			return WillNeverWork;
		else if (in < 0)
			qWarning("*** WARNING: Error in bitstream.");
		else
		{
			assert((uint)in <= m_readFrames);
			m_position += in;
			for (uint i = 0; i < (uint)m_channels; i++)
			{	BufferData d = output(i).makeScratchSamples(in/* / m_channels*/);
				if (!d.isNull())
					for (uint j = 0; j < (uint)in/* / m_channels*/; j++)
						d[j] = buffer[i][j];
				output(i).push(d);
			}
		}
		return DidWork;
	}
#endif
#ifdef HAVE_MAD
	if (m_mode == ModeMAD)
	{
		if (!m_stream.buffer || m_stream.error == MAD_ERROR_BUFLEN)
		{
			size_t readSize;
			size_t remaining;
			unsigned char* readStart;

			if (m_stream.next_frame!=NULL)
			{
				remaining = m_stream.bufend - m_stream.next_frame;
				memmove(m_inputBuffer.data(), m_stream.next_frame, remaining);
				readStart = m_inputBuffer.data() + remaining;
				readSize = m_inputBufferSize - remaining;
			}
			else
			{
				readSize = m_inputBufferSize;
				readStart = m_inputBuffer.data();
				remaining = 0;
			}

			readSize = BstdRead(readStart, 1, readSize, m_bStdFile);
			if (readSize <= 0)
			{
				if (ferror(m_madFile)) qDebug("*** WARNING: Read error on bit-stream from file %s (%s)", qPrintable(m_file.fileName()), strerror(errno));
				return DidWork;
			}

			if (BstdFileEofP(m_bStdFile))
			{
				m_guardPointer = readStart + readSize;
				memset(m_guardPointer, 0, MAD_BUFFER_GUARD);
				readSize += MAD_BUFFER_GUARD;
			}

			mad_stream_buffer(&m_stream, m_inputBuffer.data(), readSize + remaining);
			m_stream.error = (mad_error)0;
		}

		if (mad_frame_decode(&m_frame, &m_stream))
		{
			if (MAD_RECOVERABLE(m_stream.error))
			{
				if (m_stream.error != MAD_ERROR_LOSTSYNC || m_stream.this_frame != m_guardPointer)
					qDebug("*** WARNING: Recoverable frame level error in file %s (%s)", qPrintable(m_file.fileName()), mad_stream_errorstr(&m_stream));
				return DidWork;
			}
			else
				if (m_stream.error != MAD_ERROR_BUFLEN)
				{
					qDebug("*** WARNING: Unrecoverable frame level error in file %s (%s)", qPrintable(m_file.fileName()), mad_stream_errorstr(&m_stream));
					return WillNeverWork;
				}
				else
					return DidWork;
		}

		mad_timer_add(&m_timer, m_frame.header.duration);
		mad_synth_frame(&m_synth, &m_frame);
		assert(m_synth.pcm.length <= m_readFrames);
		m_position++;
		for (uint i = 0; i < m_channels; i++)
		{
			BufferData d = output(i).makeScratchSamples(m_synth.pcm.length);
			if (!d.isNull())
				for (uint j = 0; j < m_synth.pcm.length; j++)
					d[j] = float(MadFixedToSshort(m_synth.pcm.samples[i][j])) / 32768.f;
			output(i).push(d);
		}
		return DidWork;
	}
	return WillNeverWork;
#endif
}

void Player::processorStopped()
{
#ifdef HAVE_SNDFILE
	if (m_mode == ModeSF)
		sf_close(m_sndFile);
#endif
#ifdef HAVE_VORBISFILE
	if (m_mode == ModeVF)
	{
		ov_clear(&m_vorbisFile);
		m_file.close();
	}
#endif
#ifdef HAVE_MAD
	if (m_mode == ModeMAD)
	{
		BstdFileDestroy(m_bStdFile);
		mad_synth_finish(&m_synth);
		mad_frame_finish(&m_frame);
		mad_stream_finish(&m_stream);
		m_file.close();
	}
#endif
	m_position = 0;
}

EXPORT_CLASS(Player, 0,2,0, Processor);
