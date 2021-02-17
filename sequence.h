/*
    Drumstick MIDI File Player Multiplatform Program
    Copyright (C) 2006-2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTextCodec>
#include <drumstick/qsmf.h>
#include <drumstick/qwrk.h>
#include <drumstick/rtmidioutput.h>
#include "events.h"

class Sequence : public QObject
{
    Q_OBJECT
public:
    /**
     * For Karaoke files, there may be additional metadata
     * FileType: @KMIDI KARAOKE FILE
     * Version: @V0100
     * Information: @I<text>
     * Language: @L<lang>
     * Title: @T<title, author, ...>
     * Warning?: @W<bla bla bla>
     */
    enum TextType {
        Text = 1, Copyright = 2, TrackName = 3,
        InstrumentName = 4, Lyric = 5, Marker = 6, Cue = 7,
        KarFileType = 8, KarVersion = 9, KarInformation = 10,
        KarLanguage = 11, KarTitles = 12, KarWarnings = 13,
        FIRST_TYPE = Text, LAST_TYPE = KarWarnings
    };

    Sequence(QObject* parent = 0);
    virtual ~Sequence();
    void clear();
    void appendEvent(MIDIEvent* ev);
    void loadPattern(QList<MIDIEvent*> pattern);
    void loadFile(const QString& fileName);
    qreal tempoFactor() const;
    void setTempoFactor(const qreal factor);
    MIDIEvent *nextEvent();
    int eventTime(MIDIEvent* ev) const;
    std::chrono::milliseconds deltaTimeOfEvent(MIDIEvent* ev) const;
    std::chrono::milliseconds timeOfTicks(const int ticks) const;
    bool hasMoreEvents();
    int getDivision() const { return m_division; }
    bool isEmpty();
    void resetPosition();
    void setTickPosition(long tick);
    void setTimePosition(long time);
    qreal currentTempo() const;
    QString getName() const { return m_lblName; }
    int songLengthTicks() const;
    void updateTempo(qreal newTempo);
    qreal ticks2millis() const { return m_ticks2millis; }

    bool channelUsed(int channel);
    QString channelLabel(int channel);
    int lowestMidiNote();
    int highestMidiNote();

signals:
    void loadingStart(int size);
    void loadingProgress(int pos);
    void loadingFinished();

public slots:
    /* SMF slots */
    void appendSMFEvent(MIDIEvent *ev);
    void smfHeaderEvent(int format, int ntrks, int division);
    void smfNoteOnEvent(int chan, int pitch, int vol);
    void smfNoteOffEvent(int chan, int pitch, int vol);
    void smfKeyPressEvent(int chan, int pitch, int press);
    void smfCtlChangeEvent(int chan, int ctl, int value);
    void smfPitchBendEvent(int chan, int value);
    void smfProgramEvent(int chan, int patch);
    void smfChanPressEvent(int chan, int press);
    void smfSysexEvent(const QByteArray& data);
    void smfMetaEvent(int type, const QByteArray &data);
    void smfTempoEvent(int tempo);
    void smfTrackStartEvent();
    void smfTrackEnd();
    void smfErrorHandler(const QString& errorStr);
    void smfUpdateLoadProgress();

    /* WRK slots */
    void appendWRKEvent(long ticks, MIDIEvent* ev);
    void wrkUpdateLoadProgress();
    void wrkErrorHandler(const QString& errorStr);
    void wrkFileHeader(int verh, int verl);
    void wrkEndOfFile();
    void wrkStreamEndEvent(long time);
    void wrkTrackHeader(const QString& name1, const QString& name2,
             int trackno, int channel, int pitch,
             int velocity, int port,
             bool selected, bool muted, bool loop);
    void wrkTimeBase(int timebase);
    void wrkNoteEvent(int track, long time, int chan, int pitch, int vol, int dur);
    void wrkKeyPressEvent(int track, long time, int chan, int pitch, int press);
    void wrkCtlChangeEvent(int track, long time, int chan, int ctl, int value);
    void wrkPitchBendEvent(int track, long time, int chan, int value);
    void wrkProgramEvent(int track, long time, int chan, int patch);
    void wrkChanPressEvent(int track, long time, int chan, int press);
    void wrkSysexEvent(int track, long time, int bank);
    void wrkSysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data);
    void wrkTempoEvent(long time, int tempo);
    void wrkTrackPatch(int track, int patch);
    void wrkNewTrackHeader(const QString& name,
            int trackno, int channel, int pitch,
            int velocity, int port,
            bool selected, bool muted, bool loop);
    void wrkTrackVol(int track, int vol);
    void wrkTrackBank(int track, int bank);

private: // methods
    void timeCalculations();
    void sort();

private: // members
    QList<MIDIEvent*> m_list;
    drumstick::File::QSmf* m_smf;
    drumstick::File::QWrk* m_wrk;
    QTextCodec *m_codec;

    int m_ticksDuration;
    int m_division;
    int m_pos;
    int m_track;
    qreal m_tempo;
    qreal m_tempoFactor;
    qreal m_ticks2millis;
    QString m_lblName;

    struct SysexEventRec {
        int track;
        long time;
        int bank;
    };
    QList<SysexEventRec> m_savedSysexEvents;

    struct TrackMapRec {
        int channel;
        int pitch;
        int velocity;
    };
    QMap<int,TrackMapRec> m_trackMap;

    QString m_encoding;
    qreal m_duration;
    qint64 m_lastBeat;
    qint64 m_beatLength;
    int m_beatMax;
    int m_barCount;
    int m_beatCount;
    int m_lowestMidiNote;
    int m_highestMidiNote;
    QByteArray m_trackLabel;
    bool m_channelUsed[drumstick::rt::MIDI_STD_CHANNELS];
    int m_channelEvents[drumstick::rt::MIDI_STD_CHANNELS];
    QByteArray m_channelLabel[drumstick::rt::MIDI_STD_CHANNELS];
    int m_channelPatches[drumstick::rt::MIDI_STD_CHANNELS];
};

#endif // SEQUENCE_H
