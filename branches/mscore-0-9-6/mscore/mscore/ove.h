//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importove.cpp 3763 2010-12-15 15:52:09Z vanferry $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef OVE_DATA_H
#define OVE_DATA_H

#ifdef WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

namespace OVE {

class OveSong;
class Track;
class Page;
class Voice;
class Line;
class Staff;
class Measure;
class MeasureData;
class MusicData;
class OffsetElement;
class LineElement;
class PairEnds;
class Note;
class NoteContainer;
class Beam;
class Tie;
class Tuplet;
class Harmony;
class Clef;
class Lyric;
class Slur;
class MeasureText;
class Articulation;
class Glissando;
class Decorator;
class MeasureRepeat;
class Dynamics;
class Wedge;
class WedgeEndPoint;
class Pedal;
class KuoHao;
class Expressions;
class HarpPedal;
class MultiMeasureRest;
class OctaveShift;
class OctaveShiftEndPoint;
class BarNumber;
class Tempo;
class Text;
class TimeSignature;
class Key;
class RepeatSymbol;
class NumericEnding;
class MidiData;
class MidiController;
class MidiProgramChange;
class MidiChannelPressure;
class MidiPitchWheel;

const int TWELVE_TONE = 12 ;
const int INVALID_NOTE = -1 ;
const int OCTAVE = 7 ;

enum CondType {
	Cond_Time_Parameters	= 0x09,	// size - 7, TimeSignature
	Cond_Bar_Number			= 0x0A,	// size, compatible with previous version
	Cond_Decorator			= 0x16,
	Cond_Tempo				= 0x1C,	// size - 7
	Cond_Text				= 0x1D,	// size - 7, Rehearsal | SystemText
	Cond_Expression			= 0x25,	// size - 7, if set playback parameters in Expression, will store in COND
	Cond_Barline_Parameters	= 0x30, // size - 7, include :|| repeat count
	Cond_Repeat				= 0x31,	//
	Cond_Numeric_Ending		= 0x32, //

	Cond_None
};

enum BdatType {
	Bdat_Raw_Note				= 0x70,
	Bdat_Rest					= 0x80,
	Bdat_Note					= 0x90,
	Bdat_Beam					= 0x10,
	Bdat_Harmony				= 0x11,
	Bdat_Clef					= 0x12,
	Bdat_Dynamics				= 0x13,
	Bdat_Wedge					= 0x14,	// cresendo, decresendo
	Bdat_Glissando				= 0x15,
	Bdat_Decorator				= 0x16,	// measure repeat | piano pedal | dotted barline
	Bdat_Key					= 0x17,
	Bdat_Lyric					= 0x18,
	Bdat_Octave_Shift			= 0x19,
	Bdat_Slur					= 0x1B,
	Bdat_Text					= 0x1D,
	Bdat_Tie					= 0x1E,
	Bdat_Tuplet					= 0x1F,
	Bdat_Guitar_Bend			= 0x21,	//
	Bdat_Guitar_Barre			= 0x22,	//
	Bdat_Pedal					= 0x23,
	Bdat_KuoHao					= 0x24,	// () [] {}
	Bdat_Expressions			= 0x25,
	Bdat_Harp_Pedal				= 0x26,
	Bdat_Multi_Measure_Rest		= 0x27,
	Bdat_Harmony_GuitarFrame	= 0x28,
	Bdat_Graphics_40			= 0x40,	// unknown
	Bdat_Graphics_RoundRect		= 0x41,
	Bdat_Graphics_Rect			= 0x42,
	Bdat_Graphics_Round			= 0x43,
	Bdat_Graphics_Line			= 0x44,
	Bdat_Graphics_Curve			= 0x45,
	Bdat_Graphics_WedgeSymbol	= 0x46,
	Bdat_Midi_Controller		= 0xAB,
	Bdat_Midi_Program_Change	= 0xAC,
	Bdat_Midi_Channel_Pressure	= 0xAD,
	Bdat_Midi_Pitch_Wheel		= 0xAE,
	Bdat_Bar_End				= 0xFF,

	Bdat_None
};

////////////////////////////////////////

enum MusicDataType {
	// attributes
	MusicData_Clef,
	MusicData_Key,
	MusicData_Measure_Repeat,

	// sound
	MusicData_Tempo,

	// direction
	MusicData_Dynamics,
	MusicData_Wedge,
	MusicData_Wedge_EndPoint,
	MusicData_OctaveShift,
	MusicData_OctaveShift_EndPoint,
	MusicData_Expressions,
	MusicData_Repeat,
	MusicData_Text,
	MusicData_Harp_Pedal,
	MusicData_Pedal,

	// note & harmony
	MusicData_Note_Container,
	MusicData_Harmony,

	// note's children
	MusicData_Beam,
	MusicData_Glissando,
	MusicData_Lyric,
	MusicData_Slur,
	MusicData_Tie,
	MusicData_Tuplet,

	// barline
	MusicData_Numeric_Ending,

	MusicData_KuoHao,
	MusicData_Bar_End,
	MusicData_Decorator,
	MusicData_Multi_Measure_Rest,

	MusicData_None
};

enum MidiType {
	Midi_Controller = 0,
	Midi_Program_Change,
	Midi_Channel_Pressure,
	Midi_Pitch_Wheel,

	Midi_None
};

enum ClefType {
	Clef_Treble = 0x00,	//0x00
	Clef_Bass,			//0x01
	Clef_Alto,			//0x02
	Clef_UpAlto,		//0x03
	Clef_DownDownAlto,	//0x04
	Clef_DownAlto,		//0x05
	Clef_UpUpAlto,		//0x06
	Clef_Treble8va,		//0x07
	Clef_Bass8va,		//0x08
	Clef_Treble8vb,		//0x09
	Clef_Bass8vb,		//0x0A
	Clef_Percussion1,	//0x0B
	Clef_Percussion2,	//0x0C
	Clef_TAB			//0x0D
};

enum GroupType {
	Group_None = 0,
	Group_Brace,
	Group_Bracket
};

enum AccidentalType {
	Accidental_Normal				= 0x0,
	Accidental_Sharp				= 0x1,
	Accidental_Flat					= 0x2,
	Accidental_Natural				= 0x3,
	Accidental_DoubleSharp			= 0x4,
	Accidental_DoubleFlat			= 0x5,
	Accidental_Sharp_Caution		= 0x9,
	Accidental_Flat_Caution			= 0xA,
	Accidental_Natural_Caution		= 0xB,
	Accidental_DoubleSharp_Caution	= 0xC,
	Accidental_DoubleFlat_Caution	= 0xD
};

enum NoteHeadType {
	NoteHead_Standard	= 0x00,
	NoteHead_Invisible,
	NoteHead_Rhythmic_Slash,
	NoteHead_Percussion,
	NoteHead_Closed_Rhythm,
	NoteHead_Open_Rhythm,
	NoteHead_Closed_Slash,
	NoteHead_Open_Slash,
	NoteHead_Closed_Do,
	NoteHead_Open_Do,
	NoteHead_Closed_Re,
	NoteHead_Open_Re,
	NoteHead_Closed_Mi,
	NoteHead_Open_Mi,
	NoteHead_Closed_Fa,
	NoteHead_Open_Fa,
	NoteHead_Closed_Sol,
	NoteHead_Open_Sol,
	NoteHead_Closed_La,
	NoteHead_Open_La,
	NoteHead_Closed_Ti,
	NoteHead_Open_Ti
};

enum TiePos {
	Tie_None		= 0x0,
	Tie_LeftEnd		= 0x1,
	Tie_RightEnd	= 0x2
};

enum ArticulationType {
	Articulation_Major_Trill			= 0x00,
	Articulation_Minor_Trill			= 0x01,
	Articulation_Trill_Section			= 0x02,
	Articulation_Inverted_Short_Mordent	= 0x03,
	Articulation_Inverted_Long_Mordent	= 0x04,
	Articulation_Short_Mordent			= 0x05,
	Articulation_Turn					= 0x06,
	Articulation_Finger_1				= 0x07,
	Articulation_Finger_2				= 0x08,
	Articulation_Finger_3				= 0x09,
	Articulation_Finger_4				= 0x0A,
	Articulation_Finger_5				= 0x0B,
	Articulation_Flat_Accidental_For_Trill = 0x0C,
	Articulation_Sharp_Accidental_For_Trill = 0x0D,
	Articulation_Natural_Accidental_For_Trill = 0x0E,
	Articulation_Marcato				= 0x0F,
	Articulation_Marcato_Dot			= 0x10,
	Articulation_Heavy_Attack			= 0x11,
	Articulation_SForzando				= 0x12,
	Articulation_SForzando_Dot			= 0x13,
	Articulation_Heavier_Attack			= 0x14,
	Articulation_SForzando_Inverted		= 0x15,
	Articulation_SForzando_Dot_Inverted	= 0x16,
	Articulation_Staccatissimo			= 0x17,
	Articulation_Staccato				= 0x18,
	Articulation_Tenuto					= 0x19,
	Articulation_Up_Bow					= 0x1A,
	Articulation_Down_Bow				= 0x1B,
	Articulation_Up_Bow_Inverted		= 0x1C,
	Articulation_Down_Bow_Inverted		= 0x1D,
	Articulation_Arpeggio				= 0x1E,
	Articulation_Tremolo_Eighth			= 0x1F,
	Articulation_Tremolo_Sixteenth		= 0x20,
	Articulation_Tremolo_Thirty_Second	= 0x21,
	Articulation_Tremolo_Sixty_Fourth	= 0x22,
	Articulation_Natural_Harmonic		= 0x23,
	Articulation_Artificial_Harmonic	= 0x24,
	Articulation_Plus_Sign				= 0x25,
	Articulation_Fermata				= 0x26,
	Articulation_Fermata_Inverted		= 0x27,
	Articulation_Pedal_Down				= 0x28,
	Articulation_Pedal_Up				= 0x29,
	Articulation_Pause					= 0x2A,
	Articulation_Grand_Pause			= 0x2B,
	Articulation_Toe_Pedal				= 0x2C,
	Articulation_Heel_Pedal				= 0x2D,
	Articulation_Toe_To_Heel_Pedal		= 0x2E,
	Articulation_Heel_To_Toe_Pedal		= 0x2F,
	Articulation_Open_String			= 0x30,	// finger 0 in guitar
	Articulation_Guitar_Lift			= 0x46,
	Articulation_Guitar_Slide_Up		= 0x47,
	Articulation_Guitar_Rip				= 0x48,
	Articulation_Guitar_Fall_Off		= 0x49,
	Articulation_Guitar_Slide_Down		= 0x4A,
	Articulation_Guitar_Spill			= 0x4B,
	Articulation_Guitar_Flip			= 0x4C,
	Articulation_Guitar_Smear			= 0x4D,
	Articulation_Guitar_Bend			= 0x4E,
	Articulation_Guitar_Doit			= 0x4F,
	Articulation_Guitar_Plop			= 0x50,
	Articulation_Guitar_Wow_Wow			= 0x51,
	Articulation_Guitar_Thumb			= 0x64,
	Articulation_Guitar_Index_Finger	= 0x65,
	Articulation_Guitar_Middle_Finger	= 0x66,
	Articulation_Guitar_Ring_Finger		= 0x67,
	Articulation_Guitar_Pinky_Finger	= 0x68,
	Articulation_Guitar_Tap				= 0x69,
	Articulation_Guitar_Hammer			= 0x6A,
	Articulation_Guitar_Pluck			= 0x6B,

	Articulation_None

/*	Articulation_Detached_Legato,
	Articulation_Spiccato,
	Articulation_Scoop,
	Articulation_Plop,
	Articulation_Doit,
	Articulation_Falloff,
	Articulation_Breath_Mark,
	Articulation_Caesura,*/
};

enum NoteType {
	Note_DoubleWhole= 0x0,
	Note_Whole		= 0x1,
	Note_Half		= 0x2,
	Note_Quarter	= 0x3,
	Note_Eight		= 0x4,
	Note_Sixteen	= 0x5,
	Note_32			= 0x6,
	Note_64			= 0x7,
	Note_128		= 0x8,
	Note_256		= 0x9,

	Note_None
};

inline int NoteTypeToTick(NoteType type, int quarter) {
	int c = int(pow(2.0, (int)type)) ;
	return quarter * 4 * 2 / c ;
}

enum HarmonyType {
	Harmony_maj = 0,
	Harmony_min,
	Harmony_aug,
	Harmony_dim,
	Harmony_dim7,
	Harmony_sus2,
	Harmony_sus4,
	Harmony_sus24,
	Harmony_add2,
	Harmony_add9,
	Harmony_omit3,
	Harmony_omit5,
	Harmony_2,
	Harmony_5,
	Harmony_6,
	Harmony_69,
	Harmony_7,
	Harmony_7b5,
	Harmony_7b9,
	Harmony_7s9,
	Harmony_7s11,
	Harmony_7b5s9,
	Harmony_7b5b9,
	Harmony_7b9s9,
	Harmony_7b9s11,
	Harmony_7sus4,
	Harmony_9,
	Harmony_9b5,
	Harmony_9s11,
	Harmony_9sus4,
	Harmony_11,
	Harmony_13,
	Harmony_13b5,
	Harmony_13b9,
	Harmony_13s9,
	Harmony_13s11,
	Harmony_13sus4,
	Harmony_min_add2,
	Harmony_min_add9,
	Harmony_min_maj7,
	Harmony_min6,
	Harmony_min6_add9,
	Harmony_min7,
	Harmony_min7b5,
	Harmony_min7_add4,
	Harmony_min7_add11,
	Harmony_min9,
	Harmony_min9_b5,
	Harmony_min9_maj7,
	Harmony_min11,
	Harmony_min13,
	Harmony_maj7,
	Harmony_maj7_b5,
	Harmony_maj7_s5,
	Harmony_maj7_69,
	Harmony_maj7_add9,
	Harmony_maj7_s11,
	Harmony_maj9,
	Harmony_maj9_sus4,
	Harmony_maj9_b5,
	Harmony_maj9_s5,
	Harmony_maj9_s11,
	Harmony_maj13,
	Harmony_maj13_b5,
	Harmony_maj13_b9,
	Harmony_maj13_b9b5,
	Harmony_maj13_s11,
	Harmony_aug7,
	Harmony_aug7_b9,
	Harmony_aug7_s9,

	Harmony_None
};

enum DynamicsType {
	Dynamics_pppp = 0,
	Dynamics_ppp,
	Dynamics_pp,
	Dynamics_p,
	Dynamics_mp,
	Dynamics_mf,
	Dynamics_f,
	Dynamics_ff,
	Dynamics_fff,
	Dynamics_ffff,
	Dynamics_sf,
	Dynamics_fz,
	Dynamics_sfz,
	Dynamics_sffz,
	Dynamics_fp,
	Dynamics_sfp
};

enum WedgeType {
	Wedge_Cres_Line = 0,	// <
	Wedge_Double_Line,		// <>, not appear in xml
	Wedge_Decresc_Line,		// >
	Wedge_Cres,				// cresc., not appear in xml, will create Expression
	Wedge_Decresc			// decresc., not appear in xml, will create Expression
};

enum KuoHaoType {
	KuoHao_Parentheses = 0,
	KuoHao_Brace,
	KuoHao_Bracket
};

enum OctaveShiftType {
	OctaveShift_8 = 0,
	OctaveShift_Minus_8,
	OctaveShift_15,
	OctaveShift_Minus_15
};

enum OctaveShiftPosition {
	OctavePosition_Start = 0 ,
	OctavePosition_Continue,
	OctavePosition_Stop
};

enum RepeatType {
	Repeat_Segno = 0,
	Repeat_Coda,
	Repeat_ToCoda,
	Repeat_DSAlCoda,
	Repeat_DSAlFine,
	Repeat_DCAlCoda,
	Repeat_DCAlFine,
	Repeat_Fine,

	Repeat_Null
};

enum BarlineType {
	Barline_Default = 0,	//0x00 will be | or final (at last measure)
	Barline_Double,			//0x01 ||
	Barline_RepeatLeft,		//0x02 ||:
	Barline_RepeatRight,	//0x03 :||
	Barline_Final,			//0x04
	Barline_Dashed,			//0x05
	Barline_Null			//0x06
} ;

enum NoteDuration {
	NoteDuration_256 = 15,
	NoteDuration_128 = NoteDuration_256 * 2,			// 30
	NoteDuration_64 = NoteDuration_128 * 2,				// 60
	NoteDuration_32 = NoteDuration_64 * 2,				// 120
	NoteDuration_16 = NoteDuration_32 * 2,				// 240
	NoteDuration_8 = NoteDuration_16 * 2,				// 480
	NoteDuration_4 = NoteDuration_8 * 2,				// 960
	NoteDuration_2 = NoteDuration_4 * 2,				// 1920
	NoteDuration_Whole = NoteDuration_2 * 2,			// 3840
	NoteDuration_Double_Whole = NoteDuration_Whole * 2	// 7680
};

enum ToneType {
	Tone_C = 0 ,
	Tone_D,
	Tone_E,
	Tone_F,
	Tone_G,
	Tone_A,
	Tone_B
};

enum KeyType {
	Key_C = 0,		// C
	Key_Bass_1,		// F
	Key_Bass_2,		// Bb
	Key_Bass_3,		// Eb
	Key_Bass_4,		// Ab
	Key_Bass_5,		// Db
	Key_Bass_6,		// Gb
	Key_Bass_7,		// Cb
	Key_Sharp_1,	// G
	Key_Sharp_2,	// D
	Key_Sharp_3,	// A
	Key_Sharp_4,	// E
	Key_Sharp_5,	// B
	Key_Sharp_6,	// F#
	Key_Sharp_7		// C#
};

// IOveNotify.h
class IOveNotify {
public:
	IOveNotify() {}
	virtual ~IOveNotify() {}

public:
	virtual void loadInfo(const QString& info) = 0;
	virtual void loadError() = 0;
	virtual void loadPosition(int currentMeasure, int totalMeasure, int currentTrack, int totalTrack) = 0;
};

class IOVEStreamLoader {
public:
	IOVEStreamLoader() {}
	virtual ~IOVEStreamLoader() {}

public:
	virtual void setNotify(IOveNotify* notify) = 0;
	virtual void setFileStream(unsigned char* buffer, unsigned int size) = 0;
	virtual void setOve(OveSong* ove) = 0;

	// read stream, set readed data to setOve(ove)
	virtual bool load() = 0;

	virtual void release() = 0;
};

DLL_EXPORT IOVEStreamLoader* createOveStreamLoader();

/////////////////////////////////////////////////////////////////////////////
// basic element
class TickElement {
public:
	TickElement();
	virtual ~TickElement() {}

public:
	void setTick(int tick);
	int getTick(void) const;

private:
	int tick_;
};

class MeasurePos {
public:
	MeasurePos();
	virtual ~MeasurePos() {}

public:
	void setMeasure(int measure);
	int getMeasure() const;

	void setOffset(int offset);
	int getOffset() const;

	MeasurePos shiftMeasure(int measure) const;
	MeasurePos shiftOffset(int offset) const; // ignore cross measure

	bool operator ==(const MeasurePos& mp) const;
	bool operator !=(const MeasurePos& mp) const;
	bool operator <(const MeasurePos& mp) const;
	bool operator <=(const MeasurePos& mp) const;
	bool operator >(const MeasurePos& mp) const;
	bool operator >=(const MeasurePos& mp) const;

private:
	int measure_;
	int offset_;
};

class PairElement {
public:
	PairElement();
	virtual ~PairElement();

public:
	MeasurePos* start() const;
	MeasurePos* stop() const;

private:
	MeasurePos* start_;
	MeasurePos* stop_;
};

class PairEnds {
public:
	PairEnds();
	virtual ~PairEnds();

public:
	LineElement* getLeftLine() const;
	LineElement* getRightLine() const;

	OffsetElement* getLeftShoulder() const;
	OffsetElement* getRightShoulder() const;

private:
	LineElement* leftLine_;
	LineElement* rightLine_;
	OffsetElement* leftShoulder_;
	OffsetElement* rightShoulder_;
};

class LineElement {
public:
	LineElement();
	virtual ~LineElement() {}

public:
	virtual void setLine(int line); // middle line (3rd line of each clef) is set 0
	virtual int getLine(void) const;

private:
	int line_;
};

class OffsetElement {
public:
	OffsetElement();
	virtual ~OffsetElement() {}

public:
	virtual void setXOffset(int offset);
	virtual int getXOffset() const;

	virtual void setYOffset(int offset);
	virtual int getYOffset() const;

private:
	int xOffset_;
	int yOffset_;
};

class LengthElement {
public:
	LengthElement();
	virtual ~LengthElement() {}

public:
	void setLength(int length);
	int getLength() const;

private:
	int length_; // tick
};

// base class of many ove music element
class MusicData: public TickElement, public PairElement, public OffsetElement {
public:
	MusicData();
	virtual ~MusicData() {}

public:
	MusicDataType getMusicDataType() const;

	enum XmlDataType {
		Attributes = 0, NoteBeam, Notations, Direction, None
	};
	static XmlDataType getXmlDataType(MusicDataType type);
	//	static bool get_is_pair_element(MusicDataType type) ;

	// show / hide
	void setShow(bool show);
	bool getShow() const;

	// color
	void setColor(unsigned int color); // not exists in ove 3
	unsigned int getColor() const;

	void setVoice(unsigned int voice);
	unsigned int getVoice() const;

	void copyCommonBlock(const MusicData& source);

protected:
	MusicDataType musicDataType_;

private:
	bool show_;
	unsigned int color_;
	unsigned int voice_;
};

class MidiData: public TickElement {
public:
	MidiData();
	virtual ~MidiData() {}

public:
	MidiType getMidiType() const;

protected:
	MidiType midiType_;
};


////////////////////////////////////////////////////////////////////////////////
class OveSong {
public:
	OveSong();
	~OveSong();

public:
	void setIsVersion4(bool version4 = true);
	bool getIsVersion4() const;

	void setQuarter(int tick);
	int getQuarter(void) const;

	void setShowPageMargin(bool show);
	bool getShowPageMargin() const;

	void setShowTransposeTrack(bool show);
	bool getShowTransposeTrack() const;

	void setShowLineBreak(bool show);
	bool getShowLineBreak() const;

	void setShowRuler(bool show);
	bool getShowRuler() const;

	void setShowColor(bool show);
	bool getShowColor() const;

	void setPlayRepeat(bool play);
	bool getPlayRepeat() const;

	enum PlayStyle{
		Record,	Swing, Notation
	};
	void setPlayStyle(PlayStyle style);
	PlayStyle getPlayStyle() const;

	void addTitle(const QString& str);
	QList<QString> getTitles(void) const;

	void addAnnotate(const QString& str);
	QList<QString> getAnnotates(void) const;

	void addWriter(const QString& str);
	QList<QString> getWriters(void) const;

	void addCopyright(const QString& str);
	QList<QString> getCopyrights(void) const;

	void addHeader(const QString& str);
	QList<QString> getHeaders(void) const;

	void addFooter(const QString& str);
	QList<QString> getFooters(void) const;

	void addTrack(Track* ptr);
	int getTrackCount(void) const;
	QList<Track*> getTracks() const;
	Track* getTrack(int part, int staff) const;

	void setTrackBarCount(int count);
	int getTrackBarCount() const;

	bool addPage(Page* page);
	int getPageCount() const;
	Page* getPage(int idx);

	void addLine(Line* ptr);
	int getLineCount() const;
	Line* getLine(int idx) const;

	void addMeasure(Measure* ptr);
	int getMeasureCount(void) const;
	Measure* getMeasure(int bar) const;

	void addMeasureData(MeasureData* ptr);
	int getMeasureDataCount(void) const;
	MeasureData* getMeasureData(int part, int staff/*=0*/, int bar) const;
	MeasureData* getMeasureData(int track, int bar) const;

	// tool
	void setPartStaffCounts(const QList<int>& partStaffCounts);
	int getPartCount() const;
	int getStaffCount(int part) const;
	int getPartBarCount() const;

	void clear(void);

	QPair<int, int> trackToPartStaff(int track) const;

	void setTextCodecName(const QString& codecName);
	QString getCodecString(const QByteArray& text);

private:
	int partStaffToTrack(int part, int staff) const;

private:
	bool version4_;
	int quarter_;

	bool showPageMargin_;
	bool showTransposeTrack;
	bool showLineBreak_;
	bool showRuler_;
	bool showColor_;
	bool playRepeat_;
	PlayStyle playStyle_;

	QList<QString> titles_;
	QList<QString> annotates_;
	QList<QString> writers_;
	QList<QString> copyrights_;
	QList<QString> headers_;
	QList<QString> footers_;

	QList<Track*> tracks_;
	QList<Page*> pages_;
	QList<Line*> lines_;
	QList<Measure*> measures_;
	QList<MeasureData*> measureDatas_;
	int trackBarCount_;	//equal to measures_.size()

	QList<int> partStaffCounts_;
	QTextCodec* codec_;
};

class Voice {
public:
	Voice();
	~Voice(){}

public:
	void setChannel(int channel);
	int getChannel() const;

	void setVolume(int volume);
	int getVolume() const;

	void setPitchShift(int pitchShift);
	int getPitchShift() const;

	void setPan(int pan);
	int getPan() const;

	void setPatch(int patch);
	int getPatch() const;

	void setStemType(int stemType);
	int getStemType() const;

	static int getDefaultPatch();
	static int getDefaultVolume();

private:
	int channel_;		// [0, 15]
	int volume_;		// [-1, 127], -1 default
	int pitchShift_;	// [-36, 36]
	int pan_;			// [-64, 63]
	int patch_;			// [0, 127]
	int stemType_;		// 0, 1, 2
};

class Track {
public:
	Track();
	~Track();

public:
	void setName(const QString& str);
	QString getName(void) const;

	void setBriefName(const QString& str);
	QString getBriefName(void) const;

	void setPatch(unsigned int patch); // -1: percussion
	unsigned int getPatch() const;

	void setChannel(int channel);
	int getChannel() const;

	void setShowName(bool show);
	bool getShowName() const;

	void setShowBriefName(bool show);
	bool getShowBriefName() const;

	void setMute(bool mute);
	bool getMute() const;

	void setSolo(bool solo);
	bool getSolo() const;

	void setShowKeyEachLine(bool show);
	bool getShowKeyEachLine() const;

	void setVoiceCount(int voices);
	int getVoiceCount() const;

	void addVoice(Voice* voice);
	QList<Voice*> getVoices() const;

	void setShowTranspose(bool show);
	bool getShowTranspose() const;

	void setTranspose(int transpose);
	int getTranspose() const;

	void setNoteShift(int shift);
	int getNoteShift() const;

	void setStartClef(int clef/*in ClefType*/);
	ClefType getStartClef() const;

	void setTransposeClef(int clef);
	int getTansposeClef() const;

	void setStartKey(int key/*in KeyType*/);
	int getStartKey() const;

	void setDisplayPercent(unsigned int percent/*25~100*/);
	unsigned int getDisplayPercent() const;

	void setShowLegerLine(bool show);
	bool getShowLegerLine() const;

	void setShowClef(bool show);
	bool getShowClef() const;

	void setShowTimeSignature(bool show);
	bool getShowTimeSignature() const;

	void setShowKeySignature(bool show);
	bool getShowKeySignature() const;

	void setShowBarline(bool show);
	bool getShowBarline() const;

	void setFillWithRest(bool fill);
	bool getFillWithRest() const;

	void setFlatTail(bool flat);
	bool getFlatTail() const;

	void setShowClefEachLine(bool show);
	bool getShowClefEachLine() const;

	struct DrumNode {
		int line_;
		int headType_;
		int pitch_;
		int voice_;

	public:
		DrumNode():line_(0), headType_(0), pitch_(0), voice_(0){}
	};
	void addDrum(const DrumNode& node);
	QList<DrumNode> getDrumKit() const;

	void clear(void);

	/////////////////////////////////////////////////
	void setPart(int part);
	int getPart() const;

private:
	int number_;
	QString name_;
	QString briefName_;
	unsigned int patch_;
	int channel_;
	int transpose_;
	bool showTranspose_;
	int noteShift_;
	int startClef_;
	int transposeClef_;
	unsigned int displayPercent_;
	int startKey_;
	int voiceCount_;
	QList<Voice*> voices_;

	bool showName_;
	bool showBriefName_;
	bool showKeyEachLine_;
	bool showLegerLine_;
	bool showClef_;
	bool showTimeSignature_;
	bool showKeySignature_;
	bool showBarline_;
	bool showClefEachLine_;

	bool fillWithRest_;
	bool flatTail_;

	bool mute_;
	bool solo_;

	QList<DrumNode> drumKit_;

	//////////////////////////////
	int part_;
};

class Page {
public:
	Page();
	~Page(){}

public:
	void setBeginLine(int line);
	int getBeginLine() const;

	void setLineCount(int count);
	int getLineCount() const;

	void setLineInterval(int interval);	// between system
	int getLineInterval() const;

	void setStaffInterval(int interval);
	int getStaffInterval() const;

	void setStaffInlineInterval(int interval); // between treble-bass staff
	int getStaffInlineInterval() const;

	void setLineBarCount(int count);
	int getLineBarCount() const;

	void setPageLineCount(int count);
	int getPageLineCount() const;

	void setLeftMargin(int margin);
	int getLeftMargin() const;

	void setTopMargin(int margin);
	int getTopMargin() const;

	void setRightMargin(int margin);
	int getRightMargin() const;

	void setBottomMargin(int margin);
	int getBottomMargin() const;

	void setPageWidth(int width);
	int getPageWidth() const;

	void setPageHeight(int height);
	int getPageHeight() const;

private:
	int beginLine_;
	int lineCount_;

	int lineInterval_;
	int staffInterval_;
	int staffInlineInterval_;

	int lineBarCount_;
	int pageLineCount_;

	int leftMargin_;
	int topMargin_;
	int rightMargin_;
	int bottomMargin_;

	int pageWidth_;
	int pageHeight_;
};

class Line {
public:
	Line();
	~Line();

public:
	void addStaff(Staff* staff);
	int getStaffCount() const;
	Staff* getStaff(int idx) const;

	void setBeginBar(unsigned int bar);
	unsigned int getBeginBar() const;

	void setBarCount(unsigned int count);
	unsigned int getBarCount() const;

	void setYOffset(int offset);
	int getYOffset() const;

	void setLeftXOffset(int offset);
	int getLeftXOffset() const;

	void setRightXOffset(int offset);
	int getRightXOffset() const;

private:
	QList<Staff*> staffs_;
	unsigned int beginBar_;
	unsigned int barCount_;
	int yOffset_;
	int leftXOffset_;
	int rightXOffset_;
};

class Staff : public OffsetElement {
public:
	Staff();
	virtual ~Staff(){}

public:
	void setClefType(int clef);
	ClefType getClefType() const;

	void setKeyType(int key);
	int getKeyType() const;

	void setVisible(bool visible);
	bool setVisible() const;

	void setGroupType(GroupType type);
	GroupType getGroupType() const;

	void setGroupStaffCount(int count);
	int getGroupStaffCount() const;

private:
	ClefType clef_;
	int key_;
	bool visible_;
	GroupType groupType_;
	int groupStaffCount_;
};

///////////////////////////////////////////////////////////////////////////////

class Note : public LineElement {
public:
	Note();
	virtual ~Note(){}

public:
	void setIsRest(bool rest);
	bool getIsRest() const;

	void setNote(int note);
	int getNote() const;

	void setAccidental(int type);		//AccidentalType
	AccidentalType getAccidental() const;

	void setShowAccidental(bool show);
	bool getShowAccidental() const;

	void setOnVelocity(unsigned int velocity);
	unsigned int getOnVelocity() const;

	void setOffVelocity(unsigned int velocity);
	unsigned int getOffVelocity() const;

	void setHeadType(int type);		//NoteHeadType
	NoteHeadType getHeadType() const;

	void setTiePos(int tiePos);
	TiePos getTiePos() const;

	void setOffsetStaff(int offset);	// cross staff notes
	int getOffsetStaff() const;

	void setShow(bool show);
	bool getShow() const;

	void setOffsetTick(int offset);
	int getOffsetTick() const;

private:
	bool rest_;
	int note_;
	AccidentalType accidental_;
	bool showAccidental_;
	unsigned int onVelocity_;
	unsigned int offVelocity_;
	NoteHeadType headType_;
	TiePos tiePos_;
	int offsetStaff_;
	bool show_;
	int offsetTick_;//for playback
};

class Articulation : public OffsetElement {
public:
	Articulation();
	virtual ~Articulation(){}

public:
	void setArtType(int type);//ArticulationType
	ArticulationType getArtType() const;

	void setPlacementAbove(bool above);
	bool getPlacementAbove() const;

	// for midi
	bool willAffectNotes() const;

	static bool isTrill(ArticulationType type);

	// for xml
	enum XmlType {
		Xml_Articulation,
		Xml_Technical,
		Xml_Arpeggiate,
		Xml_Ornament,
		Xml_Fermata,
		Xml_Direction,

		Xml_Unknown
	};
	XmlType getXmlType() const;

	// sound setting
	bool getChangeSoundEffect() const;
	void setSoundEffect(int soundFrom, int soundTo);
	QPair<int, int> getSoundEffect() const;

	bool getChangeLength() const;
	void setLengthPercentage(int percentage);
	int getLengthPercentage() const;

	bool getChangeVelocity() const;
	enum VelocityType
	{
		Velocity_Offset,
		Velocity_SetValue,
		Velocity_Percentage
	};
	void setVelocityType(VelocityType type);
	VelocityType getVelocityType() const;

	void setVelocityValue(int value);
	int getVelocityValue() const;

	bool getChangeExtraLength() const;
	void setExtraLength(int length);
	int getExtraLength() const;

	// trill
	enum TrillInterval {
		TrillInterval_Diatonic = 0,
		TrillInterval_Chromatic,
		TrillInterval_Whole
	};
	void setTrillInterval(int interval);
	TrillInterval getTrillInterval() const;

	void setAuxiliaryFirst(bool first);
	bool getAuxiliaryFirst() const;

	void setTrillRate(NoteType rate);
	NoteType getTrillRate() const;

	void setTrillNoteLength(int length);
	int getTrillNoteLength() const;

	enum AccelerateType {
		Accelerate_None = 0 ,
		Accelerate_Slow,
		Accelerate_Normal,
		Accelerate_Fast
	};
	void setAccelerateType(int type);
	AccelerateType getAccelerateType() const;

private:
	ArticulationType type_;
	bool above_;

	bool changeSoundEffect_;
	QPair<int, int> soundEffect_;
	bool changeLength_;
	int lengthPercentage_;
	bool changeVelocity_;
	VelocityType velocityType_;
	int velocityValue_;
	bool changeExtraLength_;
	int extraLength_;

	// trill
	TrillInterval trillInterval_;
	bool auxiliaryFirst_;
	NoteType trillRate_;
	int trillNoteLength_;
	AccelerateType accelerateType_;
};

class NoteContainer : public MusicData, public LengthElement {
public:
	NoteContainer();
	virtual ~NoteContainer();

public:
	void setIsGrace(bool grace);
	bool getIsGrace() const;

	void setIsCue(bool cue);
	bool getIsCue() const;

	void setIsRest(bool rest/*or note*/);
	bool getIsRest() const;

	void setIsRaw(bool raw);
	bool getIsRaw() const;

	void setNoteType(NoteType type);
	NoteType getNoteType() const;

	void setDot(int dot);
	int getDot() const;

	void setGraceNoteType(NoteType type);
	NoteType getGraceNoteType() const;

	void setInBeam(bool in);
	bool getInBeam() const;

	void setStemUp(bool up);
	bool getStemUp(void) const;

	void setShowStem(bool show);
	bool getShowStem() const;

	void setStemLength(int line);
	int getStemLength() const;

	void setTuplet(int tuplet);
	int getTuplet() const;

	void setSpace(int space);
	int getSpace() const;

	void addNoteRest(Note* note);
	QList<Note*> getNotesRests() const;

	void addArticulation(Articulation* art);
	QList<Articulation*> getArticulations() const;

	void setNoteShift(int octave);
	int getNoteShift() const;

	int getOffsetStaff() const;

	int getDuration() const;

private:
	bool grace_;
	bool cue_;
	bool rest_;
	bool raw_;
	NoteType noteType_;
	int dot_;
	NoteType graceNoteType_;
	int tuplet_;
	int space_;
	bool inBeam_;
	bool stemUp_;
	bool showStem_;
	int stemLength_;	// line count span
	int noteShift_;

	QList<Note*> notes_;
	QList<Articulation*> articulations_;
};

class Beam : public MusicData, public PairEnds {
public:
	Beam();
	virtual ~Beam(){}

public:
	void setIsGrace(bool grace);
	bool getIsGrace() const;

	void addLine(const MeasurePos& startMp, const MeasurePos& endMp);
	const QList<QPair<MeasurePos, MeasurePos> > getLines() const;

private:
	bool grace_;
	QList<QPair<MeasurePos, MeasurePos> > lines_;
};

class Tie : public MusicData, public PairEnds {
public:
	Tie();
	virtual ~Tie(){}

public:
	void setShowOnTop(bool top);
	bool getShowOnTop() const;

	void setNote(int note);// note value tie point to
	int getNote() const;

	void setHeight(int height);
	int getHeight() const;

private:
	bool showOnTop_;
	int note_;
	int height_;
};

class Glissando : public MusicData, public PairEnds {
public:
	Glissando();
	virtual ~Glissando(){}

public:
	void setStraightWavy(bool straight);
	bool getStraightWavy() const;

	void setText(const QString& text);
	QString getText() const;

	void setLineThick(int thick);
	int getLineThick() const;

private:
	bool straight_;
	QString text_;
	int lineThick_;
};

class Decorator : public MusicData {
public:
	Decorator();
	virtual ~Decorator(){}

public:
	enum DecoratorType {
		Decorator_Dotted_Barline = 0,
		Decorator_Articulation
	};
	void setDecoratorType(DecoratorType type);
	DecoratorType getDecoratorType() const;

	void setArticulationType(ArticulationType type);
	ArticulationType getArticulationType() const;

private:
	DecoratorType decoratorType_;
	ArticulationType artType_;
};

class MeasureRepeat : public MusicData {
public:
	MeasureRepeat();
	virtual ~MeasureRepeat(){}

public:
	void setSingleRepeat(bool single); // false : double
	bool getSingleRepeat() const;

private:
	bool singleRepeat_;
};

class Tuplet : public MusicData, public PairEnds {
public:
	Tuplet();
	virtual ~Tuplet();

public:
	void setTuplet(int tuplet=3);
	int getTuplet() const;

	void setSpace(int space=2);
	int getSpace() const;

	void setHeight(int height);
	int getHeight() const;

	void setNoteType(NoteType type);
	NoteType getNoteType() const;

	OffsetElement* getMarkHandle() const;

private:
	int tuplet_;
	int space_;
	int height_;
	NoteType noteType_;
	OffsetElement* mark_;
};

class Harmony : public MusicData, public LengthElement {
public:
	Harmony();
	virtual ~Harmony(){}

public:
	void setHarmonyType(HarmonyType type);
	HarmonyType getHarmonyType() const;

	void setRoot(int root=0);//C
	int getRoot() const;

	void setBass(int bass);
	int getBass() const;

	void setBassOnBottom(bool on);
	bool getBassOnBottom() const;

	void setAngle(int angle);
	int getAngle() const;

private:
	HarmonyType harmonyType_;
	int root_;
	int bass_;
	bool bassOnBottom_;
	int angle_;
};

class Clef : public MusicData, public LineElement {
public:
	Clef();
	virtual ~Clef(){}

public:
	void setClefType(int type);	// ClefType
	ClefType getClefType() const;

private:
	ClefType clefType_;
};

class Lyric : public MusicData {
public:
	Lyric();
	virtual ~Lyric(){}

public:
	void setLyric(const QString& lyricText);
	QString getLyric() const;

	void setVerse(int verse);
	int getVerse() const;

private:
	QString lyric_;
	int verse_;
};

class Slur: public MusicData, public PairEnds {
public:
	Slur();
	virtual ~Slur();

public:
	void setContainerCount(int count); // span
	int getContainerCount() const;

	void setShowOnTop(bool top);
	bool getShowOnTop() const;

	OffsetElement* getHandle2() const;
	OffsetElement* getHandle3() const;

	void setNoteTimePercent(int percent); // 50% ~ 200%
	int getNoteTimePercent() const;

private:
	int containerCount_;
	bool showOnTop_;
	int noteTimePercent_;
	OffsetElement* handle_2_;
	OffsetElement* handle_3_;
};

class Dynamics: public MusicData {
public:
	Dynamics();
	virtual ~Dynamics() {}

public:
	void setDynamicsType(int type);//DynamicsType
	DynamicsType getDynamicsType() const;

	void setIsPlayback(bool play);
	bool getIsPlayback() const;

	void setVelocity(int vel);
	int getVelocity() const;

private:
	DynamicsType dynamicsType_;
	bool playback_;
	int velocity_;
};

class WedgeEndPoint: public MusicData {
public:
	WedgeEndPoint();
	virtual ~WedgeEndPoint() {}

public:
	void setWedgeType(WedgeType type);
	WedgeType getWedgeType() const;

	void setHeight(int height);
	int getHeight() const;

	void setWedgeStart(bool wedgeStart);
	bool getWedgeStart() const;

private:
	int height_;
	WedgeType wedgeType_;
	bool wedgeStart_;
};

class Wedge: public MusicData {
public:
	Wedge();
	virtual ~Wedge() {}

public:
	void setWedgeType(WedgeType type);
	WedgeType getWedgeType() const;

	void setHeight(int height);
	int getHeight() const;

private:
	int height_;
	WedgeType wedgeType_;
};

class Pedal: public MusicData, public PairEnds {
public:
	Pedal();
	virtual ~Pedal();

public:
	void setHalf(bool half);
	bool getHalf() const;

	void setIsPlayback(bool playback);
	bool getIsPlayback() const;

	void setPlayOffset(int offset); // -127~127
	int getPlayOffset() const;

	OffsetElement* getPedalHandle() const; //only on half pedal

private:
	bool half_;
	bool playback_;
	int playOffset_;
	OffsetElement* pedalHandle_;
};

class KuoHao: public MusicData, public PairEnds {
public:
	KuoHao();
	virtual ~KuoHao() {}

public:
	void setHeight(int height);
	int getHeight() const;

	void setKuohaoType(int type);// KuoHaoType
	KuoHaoType getKuohaoType() const;

private:
	int height_;
	KuoHaoType kuohaoType_;
};

class Expressions: public MusicData {
public:
	Expressions();
	virtual ~Expressions() {}

public:
	void setText(const QString& str);
	QString getText() const;

private:
	QString text_;
};

class HarpPedal: public MusicData {
public:
	HarpPedal();
	virtual ~HarpPedal() {}

public:
	void setShowType(int type);//0:graph, 1:char, 2:char cut, 3:change
	int getShowType() const;

	void setShowCharFlag(int flag);//each bit is a bool, total 7 bools
	int getShowCharFlag() const;

private:
	int showType_;
	int showCharFlag_;
};

class OctaveShift: public MusicData, public LengthElement {
public:
	OctaveShift();
	virtual ~OctaveShift() {}

public:
	void setOctaveShiftType(int type);
	OctaveShiftType getOctaveShiftType() const;

	int getNoteShift() const;

	void setEndTick(int tick);
	int getEndTick() const;

private:
	OctaveShiftType octaveShiftType_;
	OctaveShiftPosition octaveShiftPosition_;
	int endTick_;
};

class OctaveShiftEndPoint: public MusicData, public LengthElement {
public:
	OctaveShiftEndPoint();
	virtual ~OctaveShiftEndPoint() {}

public:
	void setOctaveShiftType(int type);
	OctaveShiftType getOctaveShiftType() const;

	void setOctaveShiftPosition(int position);
	OctaveShiftPosition getOctaveShiftPosition() const;

	void setEndTick(int tick);
	int getEndTick() const;

private:
	OctaveShiftType octaveShiftType_;
	OctaveShiftPosition octaveShiftPosition_;
	int endTick_;
};

class MultiMeasureRest: public MusicData {
public:
	MultiMeasureRest();
	virtual ~MultiMeasureRest() {}

public:
	void setMeasureCount(int count);
	int getMeasureCount() const;

private:
	int measureCount_;
};

class Tempo: public MusicData {
public:
	Tempo();
	virtual ~Tempo() {}

public:
	void setLeftNoteType(int type);//NoteType
	NoteType getLeftNoteType() const;

	void setShowMark(bool show);
	bool getShowMark() const;

	void setShowBeforeText(bool show);
	bool getShowBeforeText() const;

	void setShowParenthesis(bool show);
	bool getShowParenthesis() const;

	void setTypeTempo(int tempo); //0x2580 = 96.00
	int getTypeTempo() const;
	int getQuarterTempo() const;

	void setLeftText(const QString& str);// string at left of the mark
	QString getLeftText() const;

	void setRightText(const QString& str);
	QString getRightText() const;

	void setSwingEighth(bool swing);
	bool getSwingEighth() const;

	void setRightNoteType(int type);
	int getRightNoteType() const;

private:
	int leftNoteType_;
	bool showMark_;
	bool showText_;
	bool showParenthesis_;
	int typeTempo_;
	QString leftText_;
	QString rightText_;
	bool swingEighth_;
	int rightNoteType_;
};

class Text: public MusicData, public LengthElement {
public:
	Text();
	virtual ~Text() {}

public:
	enum TextType {
		Text_Rehearsal,
		Text_SystemText,
		Text_MeasureText
	};

	void setTextType(TextType type);
	TextType getTextType() const;

	void setHorizontalMargin(int margin);
	int getHorizontalMargin() const;

	void setVerticalMargin(int margin);
	int getVerticalMargin() const;

	void setLineThick(int thick);
	int getLineThick() const;

	void setText(const QString& text);
	QString getText() const;

	void setWidth(int width);
	int getWidth() const;

	void setHeight(int height);
	int getHeight() const;

private:
	TextType textType_;
	int horiMargin_;
	int vertMargin_;
	int lineThick_;
	QString text_;
	int width_;
	int height_;
};

///////////////////////////////////////////////////////////////////////////////

class TimeSignature: public MusicData {
public:
	TimeSignature();
	virtual ~TimeSignature() {}

public:
	void setNumerator(int numerator);
	int getNumerator() const;

	void setDenominator(int denominator);
	int getDenominator() const;

	void setIsSymbol(bool symbol); //4/4:common, 2/2:cut
	bool getIsSymbol() const;

	void setBeatLength(int length); // tick
	int getBeatLength() const;

	void setBarLength(int length); // tick
	int getBarLength() const;

	void addBeat(int startUnit, int lengthUnit, int startTick);
	void endAddBeat();
	int getUnits() const;

	void setReplaceFont(bool replace);
	bool getReplaceFont() const;

	void setShowBeatGroup(bool show);
	bool getShowBeatGroup() const;

	void setGroupNumerator1(int numerator);
	void setGroupNumerator2(int numerator);
	void setGroupNumerator3(int numerator);
	void setGroupDenominator1(int denominator);
	void setGroupDenominator2(int denominator);
	void setGroupDenominator3(int denominator);

	void setBeamGroup1(int count);
	void setBeamGroup2(int count);
	void setBeamGroup3(int count);
	void setBeamGroup4(int count);

	void set16thBeamCount(int count);
	void set32thBeamCount(int count);

private:
	int numerator_;
	int denominator_;
	bool isSymbol_;
	int beatLength_;
	int barLength_;

	struct BeatNode {
		int startUnit_;
		int lengthUnit_;
		int startTick_;

		BeatNode() :
			startUnit_(0),
			lengthUnit_(0),
			startTick_(0) {
		}
	};
	QList<BeatNode> beats_;
	int barLengthUnits_;

	bool replaceFont_;
	bool showBeatGroup_;

	int groupNumerator1_;
	int groupNumerator2_;
	int groupNumerator3_;
	int groupDenominator1_;
	int groupDenominator2_;
	int groupDenominator3_;

	int beamGroup1_;
	int beamGroup2_;
	int beamGroup3_;
	int beamGroup4_;

	int beamCount16th_;
	int beamCount32th_;
};

class Key: public MusicData {
public:
	Key();
	virtual ~Key() {}

public:
	void setKey(int key); //C=0x0, G=0x8, C#=0xE, F=0x1, Db=0x7
	int getKey() const;
	bool getSetKey() const;

	void setPreviousKey(int key);
	int getPreviousKey() const;

	void setSymbolCount(int count);
	int getSymbolCount() const;

private:
	int key_;
	bool set_;
	int previousKey_;
	int symbolCount_;
};

class RepeatSymbol: public MusicData {
public:
	RepeatSymbol();
	virtual ~RepeatSymbol() {}

public:
	void setText(const QString& text);
	QString getText() const;

	void setRepeatType(int repeatType);
	RepeatType getRepeatType() const;

private:
	QString text_;
	RepeatType repeatType_;
};

class NumericEnding: public MusicData, public PairEnds {
public:
	NumericEnding();
	virtual ~NumericEnding();

public:
	OffsetElement* getNumericHandle() const;

	void setHeight(int height);
	int getHeight() const;

	void setText(const QString& text);
	QString getText() const;
	QList<int> getNumbers() const;
	int getJumpCount() const;

private:
	int height_;
	QString text_;
	OffsetElement* numericHandle_;
};

class BarNumber: public MusicData {
public:
	BarNumber();
	virtual ~BarNumber() {}

public:
	void setIndex(int index);
	int getIndex() const;

	void setShowOnParagraphStart(bool show);
	bool getShowOnParagraphStart() const;

	void setAlign(int align);// 0:left, 1:center, 2:right
	int getAlign() const;

	void setShowFlag(int flag); // 0:page, 1:staff, 2:bar, 3:none
	int getShowFlag() const;

	void setShowEveryBarCount(int count);
	int getShowEveryBarCount() const;

	void setPrefix(const QString& str);
	QString getPrefix() const;

private:
	int index_;
	bool showOnParagraphStart_;
	int align_;
	int showFlag_;
	int barRange_;
	QString prefix_;
};

///////////////////////////////////////////////////////////////////////////////
// MIDI
class MidiController: public MidiData {
public:
	MidiController();
	virtual ~MidiController() {}

public:
	void setController(int number);
	int getController() const;

	void setValue(int value);
	int getValue() const;

private:
	int controller_;
	int value_;
};

class MidiProgramChange: public MidiData {
public:
	MidiProgramChange();
	virtual ~MidiProgramChange() {}

public:
	void setPatch(int patch);
	int getPatch() const;

private:
	int patch_;
};

class MidiChannelPressure: public MidiData {
public:
	MidiChannelPressure();
	virtual ~MidiChannelPressure() {}

public:
	void setPressure(int pressure);
	int getPressure() const;

private:
	int pressure_;
};

class MidiPitchWheel: public MidiData {
public:
	MidiPitchWheel();
	virtual ~MidiPitchWheel() {}

public:
	void setValue(int value);
	int getValue() const;

private:
	int value_;
};

///////////////////////////////////////////////////////////////////////////////
class Measure: public LengthElement {
public:
	Measure(int index = 0);
	virtual ~Measure();

private:
	Measure();

public:
	BarNumber* getBarNumber() const;
	TimeSignature* getTime() const;

	void setLeftBarline(int barline/*in BarlineType*/);
	BarlineType getLeftBarline() const;

	void setRightBarline(int barline/*in BarlineType*/);
	BarlineType getRightBarline() const;

	// set when rightBarline == Baline_Backward
	void setBackwardRepeatCount(int repeatCount);
	int getBackwardRepeatCount() const;

	void setTypeTempo(double tempo);
	double getTypeTempo() const;

	void setIsPickup(bool pickup);
	bool getIsPickup() const;

	void setIsMultiMeasureRest(bool rest);
	bool getIsMultiMeasureRest() const;

	void setMultiMeasureRestCount(int count);
	int getMultiMeasureRestCount() const;

private:
	void clear();

	BarNumber* barNumber_;
	TimeSignature* time_;

	BarlineType leftBarline_;
	BarlineType rightBarline_;
	int repeatCount_;
	double typeTempo_; // based on some type
	bool pickup_;
	bool multiMeasureRest_;
	int multiMeasureRestCount_;
};

class MeasureData {
public:
	MeasureData();
	~MeasureData();

public:
	Clef* getClef() const;
	Key* getKey() const;

	void addNoteContainer(NoteContainer* ptr);
	QList<NoteContainer*> getNoteContainers() const;

	// put Tempo, Text, RepeatSymbol to MeasureData at part=0 && staff=0
	void addMusicData(MusicData* ptr);
	// if type==MusicData_None, return all
	QList<MusicData*> getMusicDatas(MusicDataType type);//MusicXml: note|direction|harmony

	// put NumericEnding to MeasureData at part=0 && staff=0
	void addCrossMeasureElement(MusicData* ptr, bool start);
	enum PairType {
		PairType_Start,
		PairType_Stop,
		PairType_All
	};
	QList<MusicData*> getCrossMeasureElements(MusicDataType type, PairType pairType);

	// for midi
	void addMidiData(MidiData* ptr);
	QList<MidiData*> getMidiDatas(MidiType type);

private:
	Key* key_;
	Clef* clef_;
	QList<MusicData*> musicDatas_;
	QList<NoteContainer*> noteContainers_;
	QList<QPair<MusicData*, bool> > crossMeasureElements_;
	QList<MidiData*> midiDatas_;
};

// StreamHandle
class StreamHandle {
public:
	StreamHandle(unsigned char* p, int size);
	virtual ~StreamHandle();

private:
	StreamHandle();

public:
	virtual bool read(char* buff, int size);
	virtual bool write(char* buff, int size);

private:
	int size_;
	int curPos_;
	unsigned char* point_;
};

// Block.h
// base block, or resizable block in ove to store data
class Block {
public:
	Block();
	explicit Block(unsigned int size);
	virtual ~Block() {
	}

public:
	// size > 0, check this in use code
	virtual void resize(unsigned int count);

	const unsigned char* data() const;
	unsigned char* data();
	unsigned int size() const;

	bool operator ==(const Block& block) const;
	bool operator !=(const Block& block) const;

	bool toBoolean() const;
	unsigned int toUnsignedInt() const;
	int toInt() const;
	QByteArray toStrByteArray() const;					// string
	QByteArray fixedSizeBufferToStrByteArray() const;	// string

private:
	void doResize(unsigned int count);

private:
	// char [-128, 127], unsigned char [0, 255]
	QList<unsigned char> data_;
};

class FixedBlock: public Block {
public:
	explicit FixedBlock(unsigned int count);
	virtual ~FixedBlock() {
	}

private:
	FixedBlock();

private:
	// can't resize
	virtual void resize(unsigned int count);
};

///////////////////////////////////////////////////////////////////////////////
// 4 byte block in ove to store size
class SizeBlock: public FixedBlock {
public:
	SizeBlock();
	virtual ~SizeBlock() {
	}

public:
	//	void fromUnsignedInt(unsigned int count) ;

	unsigned int toSize() const;
};

// 4 bytes block in ove to store name
class NameBlock: public FixedBlock {
public:
	NameBlock();
	virtual ~NameBlock() {
	}

public:
	// ingore data more than 4 bytes
	bool isEqual(const QString& name) const;
};

// 2 bytes block in ove to store count
class CountBlock: public FixedBlock {
public:
	CountBlock();
	virtual ~CountBlock() {
	}

public:
	//	void setValue(unsigned short count) ;

	unsigned short toCount() const;
};

// Chunk.h
// content : name
class Chunk {
public:
	Chunk();
	virtual ~Chunk() {
	}

public:
	const static QString TrackName;
	const static QString PageName;
	const static QString LineName;
	const static QString StaffName;
	const static QString MeasureName;
	const static QString ConductName;
	const static QString BdatName;

	NameBlock getName() const;

protected:
	NameBlock nameBlock_;
};

// content : name / size / data
class SizeChunk: public Chunk {
public:
	SizeChunk();
	virtual ~SizeChunk();

public:
	SizeBlock* getSizeBlock() const;
	Block* getDataBlock() const;

	const static unsigned int version3TrackSize;

protected:
	SizeBlock* sizeBlock_;
	Block* dataBlock_;
};

// content : name / count
class GroupChunk: public Chunk {
public:
	GroupChunk();
	virtual ~GroupChunk();

public:
	CountBlock* getCountBlock() const;

protected:
	CountBlock* childCount_;
};

// ChunkParse.h
class BasicParse {
public:
	BasicParse(OveSong* ove);
	virtual ~BasicParse();

private:
	BasicParse();

public:
	void setNotify(IOveNotify* notify);
	virtual bool parse();

protected:
	bool readBuffer(Block& placeHolder, unsigned int size);
	bool jump(int offset);

	void messageOut(const QString& str);

protected:
	OveSong* ove_;
	StreamHandle* handle_;
	IOveNotify* notify_;
};

///////////////////////////////////////////////////////////////////////////////

class OvscParse: public BasicParse {
public:
	OvscParse(OveSong* ove);
	virtual ~OvscParse();

public:
	void setOvsc(SizeChunk* chunk);

	virtual bool parse();

private:
	SizeChunk* chunk_;
};

class TrackParse: public BasicParse {
public:
	TrackParse(OveSong* ove);
	virtual ~TrackParse();

public:
	void setTrack(SizeChunk* chunk);

	virtual bool parse();

private:
	SizeChunk* chunk_;
};

class GroupParse: BasicParse {
public:
	GroupParse(OveSong* ove);
	virtual ~GroupParse();

public:
	void addSizeChunk(SizeChunk* sizeChunk);

	virtual bool parse();

private:
	QList<SizeChunk*> sizeChunks_;
};

class PageGroupParse: public BasicParse {
public:
	PageGroupParse(OveSong* ove);
	virtual ~PageGroupParse();

public:
	void addPage(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parsePage(SizeChunk* chunk, Page* page);

private:
	QList<SizeChunk*> pageChunks_;
};

class StaffCountGetter: public BasicParse {
public:
	StaffCountGetter(OveSong* ove);
	virtual ~StaffCountGetter() {}

public:
	unsigned int getStaffCount(SizeChunk* chunk);
};

class LineGroupParse: public BasicParse {
public:
	LineGroupParse(OveSong* ove);
	virtual ~LineGroupParse();

public:
	void setLineGroup(GroupChunk* chunk);
	void addLine(SizeChunk* chunk);
	void addStaff(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parseLine(SizeChunk* chunk, Line* line);
	bool parseStaff(SizeChunk* chunk, Staff* staff);

private:
	GroupChunk* chunk_;
	QList<SizeChunk*> lineChunks_;
	QList<SizeChunk*> staffChunks_;
};

class BarsParse: public BasicParse {
public:
	BarsParse(OveSong* ove);
	virtual ~BarsParse();

public:
	void addMeasure(SizeChunk* chunk);
	void addConduct(SizeChunk* chunk);
	void addBdat(SizeChunk* chunk);

	virtual bool parse();

private:
	bool parseMeas(Measure* measure, SizeChunk* chunk);
	bool parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk);
	bool parseBdat(Measure* measure, MeasureData* measureData, SizeChunk* chunk);

	bool getCondElementType(unsigned int byteData, CondType& type);
	bool getBdatElementType(unsigned int byteData, BdatType& type);

	// COND
	bool parseTimeSignature(Measure* measure, int length);
	bool parseTimeSignatureParameters(Measure* measure, int length);
	bool parseRepeatSymbol(MeasureData* measureData, int length);
	bool parseNumericEndings(MeasureData* measureData, int length);
	bool parseTempo(MeasureData* measureData, int length);
	bool parseBarNumber(Measure* measure, int length);
	bool parseText(MeasureData* measureData, int length);
	bool parseBarlineParameters(Measure* measure, int length);

	// BDAT
	bool parseNoteRest(MeasureData* measureData, int length, BdatType type);
	bool parseBeam(MeasureData* measureData, int length);
	bool parseTie(MeasureData* measureData, int length);
	bool parseTuplet(MeasureData* measureData, int length);
	bool parseHarmony(MeasureData* measureData, int length);
	bool parseClef(MeasureData* measureData, int length);
	bool parseLyric(MeasureData* measureData, int length);
	bool parseSlur(MeasureData* measureData, int length);
	bool parseGlissando(MeasureData* measureData, int length);
	bool parseDecorators(MeasureData* measureData, int length);
	bool parseDynamics(MeasureData* measureData, int length);
	bool parseWedge(MeasureData* measureData, int length);
	bool parseKey(MeasureData* measureData, int length);
	bool parsePedal(MeasureData* measureData, int length);
	bool parseKuohao(MeasureData* measureData, int length);
	bool parseExpressions(MeasureData* measureData, int length);
	bool parseHarpPedal(MeasureData* measureData, int length);
	bool parseMultiMeasureRest(MeasureData* measureData, int length);
	bool parseHarmonyGuitarFrame(MeasureData* measureData, int length);
	bool parseOctaveShift(MeasureData* measureData, int length);
	bool parseMidiController(MeasureData* measureData, int length);
	bool parseMidiProgramChange(MeasureData* measureData, int length);
	bool parseMidiChannelPressure(MeasureData* measureData, int length);
	bool parseMidiPitchWheel(MeasureData* measureData, int length);

	bool parseSizeBlock(int length);
	bool parseMidiCommon(MidiData* ptr);
	bool parseCommonBlock(MusicData* ptr);
	bool parseOffsetCommonBlock(MusicData* ptr);
	bool parsePairLinesBlock(PairEnds* ptr); //size==2
	bool parseOffsetElement(OffsetElement* ptr);//size==2

private:
	QList<SizeChunk*> measureChunks_;
	QList<SizeChunk*> conductChunks_;
	QList<SizeChunk*> bdatChunks_;
};

class LyricChunkParse: public BasicParse {
public:
	LyricChunkParse(OveSong* ove);
	virtual ~LyricChunkParse() {}

public:
	void setLyricChunk(SizeChunk* chunk);

	virtual bool parse();

private:
	struct LyricInfo {
		int track_;
		int measure_;
		int verse_;
		int voice_;
		int wordCount_;
		int lyricSize_;
		QString name_;
		QString lyric_;
		int font_;
		int fontSize_;
		int fontStyle_;

		LyricInfo() :
			track_(0), measure_(0), verse_(0), voice_(0), wordCount_(0),
					lyricSize_(0), name_(QString()), lyric_(QString()),
					font_(0), fontSize_(12), fontStyle_(0) {}
	};

	void processLyricInfo(const LyricInfo& info);

private:
	SizeChunk* chunk_;
};

class TitleChunkParse: public BasicParse {
public:
	TitleChunkParse(OveSong* ove);
	virtual ~TitleChunkParse() {}

public:
	void setTitleChunk(SizeChunk* chunk);

	virtual bool parse();

private:
	void addToOve(const QString& str, unsigned int titleType);

private:
	unsigned int titleType_;
	unsigned int annotateType_;
	unsigned int writerType_;
	unsigned int copyrightType_;
	unsigned int headerType_;
	unsigned int footerType_;

	SizeChunk* chunk_;
};

// OveOrganizer.h
class OveOrganizer {
public:
	OveOrganizer(OveSong* ove) ;
	virtual ~OveOrganizer(){}

public:
	void organize() ;

private:
	void organizeAttributes() ;
	void organizeTracks() ;
	void organizeMeasures() ;
	void organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData) ;

	void organizeContainers(int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeMusicDatas(int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData) ;

	void organizePairElement(MusicData* data, int part, int track, Measure* measure, MeasureData* measureData) ;
	void organizeOctaveShift(OctaveShift* octave, Measure* measure, MeasureData* measureData) ;
	void organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData) ;

private:
	OveSong* ove_ ;
};

// OveSerialize.h
class StreamHandle;
class Block;
class NameBlock;
class Chunk;
class SizeChunk;
class GroupChunk;

class OveSerialize: public IOVEStreamLoader {
public:
	OveSerialize();
	virtual ~OveSerialize();

public:
	virtual void setOve(OveSong* ove);
	virtual void setFileStream(unsigned char* buffer, unsigned int size);
	virtual void setNotify(IOveNotify* notify);
	virtual bool load(void);

	virtual void release();

private:
	bool readNameBlock(NameBlock& nameBlock);
	bool readChunkName(Chunk* chunk, const QString& name);
	bool readSizeChunk(SizeChunk* sizeChunk); // contains a SizeChunk and data buffer
	bool readDataChunk(Block* block, unsigned int size);
	bool readGroupChunk(GroupChunk* groupChunk);

	bool readHeader();
	bool readHeadData(SizeChunk* ovscChunk);
	bool readTracksData();
	bool readPagesData();
	bool readLinesData();
	bool readBarsData();
	bool readOveEnd();

	void messageOutError();
	void messageOut(const QString& str);

private:
	OveSong* ove_;
	StreamHandle* streamHandle_;
	IOveNotify* notify_;
};

}

#endif