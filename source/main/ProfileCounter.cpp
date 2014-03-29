#include "ProfileCounter.h"
#include "Material.h"

std::vector<ProfileCounter*> ProfileCounter::Counters;
bool ProfileCounter::Profiling=1;
unsigned ProfileCounter::Overhead=0;
unsigned ProfileCounter::FrameTime=0;

float Interval;
unsigned curPos, Samples, numValues;
float Update;
std::string Name;
float ScaleX, ScaleY;
float Seconds;
__int64 StartTicks, SumTicks;
unsigned Min, Max, Avg, Sum;

ProfileCounter::ProfileCounter(const char* name, float interval, float secs)
: Interval(interval), curPos(0), Samples(0), Update(0), Name(name),
  Seconds(secs), SumTicks(0), Min(0), Max(0), Avg(0), Sum(0)
{
	if (!Overhead) Init();
	numValues = unsigned(interval ? secs/interval : secs);
	Values.resize(numValues);
	for (unsigned i=0; i<numValues; ++i) Values[i]=0;
	UpdateTimer.Start();
	ScaleX=1.0f/(numValues-1);
	if (!Font::Initialized) Font::Init("data/font/verdanasmall.glf");

	Counters.push_back(this);
}

ProfileCounter::~ProfileCounter() 
{
	Counters.erase( find(Counters.begin(), Counters.end(), this) );
}

void ProfileCounter::Print(float x, float y, float scale) {
	glPushMatrix();
	glTranslatef(x,y, 0);
	glScalef(scale, scale, 0);
	glPushMatrix();
	glScalef(24, 15, 0);

	MaterialManager::SetColor(0,.3f,0,.75f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glVertex2f(0, 0); glVertex2f(0, 1);
	glVertex2f(1, 1); glVertex2f(1, 0);
	glEnd();
	MaterialManager::SetColor(0,.5f,0,.75f);
	glBegin(GL_LINES);
	glVertex2f(0.125f, 0); glVertex2f(0.125f, 1);
	glVertex2f(0.375f, 0); glVertex2f(0.375f, 1);
	glVertex2f(0.625f, 0); glVertex2f(0.625f, 1);
	glVertex2f(0.875f, 0); glVertex2f(0.875f, 1);
	glVertex2f(.25f, 0); glVertex2f(.25f, 1);
	glVertex2f(0, .25f); glVertex2f(1, .25f);
	glVertex2f(.5f, 0); glVertex2f(.5f, 1);
	glVertex2f(0, .5f); glVertex2f(1, .5f);
	glVertex2f(.75f, 0); glVertex2f(.75f, 1);
	glVertex2f(0, .75f); glVertex2f(1, .75f);
	glEnd();

	glTranslatef(0,1,0);
	glScalef(ScaleX, -ScaleY, 0);
	MaterialManager::SetColor(1,1,1,1.75f);
	glBegin(GL_LINE_STRIP);
	int offset=0;
	for (unsigned i=curPos; i<numValues; ++i, ++offset) 
		glVertex2f(float(offset), float(Values[i]));
	for (unsigned j=0; j<curPos; ++j, ++offset) 
		glVertex2f(float(offset), float(Values[j]));
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();

	//char* unit="t"; unsigned factor=1;
	//if (Max>1000) {unit="kt"; factor=1000;}
	//else if (Max>1000000) {unit="mt"; factor=1000000;}
        const char* unit="ns"; unsigned factor=1;
	if (Max>1000) {unit="ms"; factor=1000;}
	else if (Max>1000000) {unit="s"; factor=1000000;}
	

	MaterialManager::SetColor(1,1,1,.75f);
	Font::Print(Name, 0,0,2);
	char tmp[50];
	float RcpFT=100.0f/FrameTime;
	_snprintf(tmp, 50, "%d(%.1f%%)\n%d(%.1f%%)\n%d(%.1f%%)",
		Max/factor, RcpFT*Max, Avg/factor, RcpFT*Avg, Min/factor, RcpFT*Min);
	Font::Print(tmp, 0,4.5f,2);
	_snprintf(tmp, 50, "%.0f%s", 1.0f/(ScaleY*factor), unit);
	Font::Print(tmp, 24,0,2,1);
	if (Interval) _snprintf(tmp, 50, "-%.2fs", Seconds);
	else _snprintf(tmp, 50, "-%.0f", Seconds);
	Font::Print(tmp, 0,13,2);
	unsigned Val=Values[curPos ? curPos-1 : numValues-1];
	_snprintf(tmp, 50, "%d%s", Val/factor, unit);
	Font::Print(tmp, 24,13,2,1);
	glPopMatrix();
	MaterialManager::SetAlpha(1);
}

void ProfileCounter::DrawCounters() {
	static unsigned numFrames=0;
	if (!Profiling) return;
	if (++numFrames>50) {
		static unsigned NewTime = 0;
		static unsigned OldTime = 0;

		NewTime = (unsigned)Ticks();
		FrameTime=(NewTime-OldTime)/numFrames;
		OldTime=NewTime;
		numFrames=0;
	}
	int count=Counters.size();
	float scale=1.0f;//24 6*4
	if (count>77) scale=0.5f; //96 12*8
	else if (count>70) scale=.5454f; //77 11*7
	else if (count>60) scale=.57f; //70 10*7
	else if (count>54) scale=.6f; //60 10*6
	else if (count>40) scale=0.666f; //54 9*6
	else if (count>35) scale=0.75f; // 40 8*5
	else if (count>28) scale=0.8f; // 35 7*5
	else if (count>24) scale=0.857f; // 28 7*4
	unsigned num=(unsigned)(6.0/scale);
	for (int i=0; i<count; ++i)
		Counters[i]->Print(.5f + scale*(25*(i/num)), 3.5f + scale*(16*(i%num)), scale);
}
