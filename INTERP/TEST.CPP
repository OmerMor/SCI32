#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <direct.h>

#include "sol.hpp"
#include "sci.hpp"
#include "graphmgr.hpp"
#include "resource.hpp"
#include "msg.hpp"
#include "event.hpp"
#include "mouse.hpp"
#include "mono.hpp"
#include "cursor.hpp"
#include "time.hpp"
#include "config.hpp"

#include "kernel.hpp"
#include "audio.hpp"
#include "event.hpp"

const robotCount = 11;
static robotNumList[] = { 1802, 1804, 1807, 2522, 2923, 4180, 4236,
								  7290, 7291, 7302, 7313 };

const dirCount = 7;
static char* dirstr[] = {
  "f:\\ver45",
  "f:\\v5x8k",
  "f:\\v5x31k",
  "f:\\v5x60k",
  "f:\\300x10",
  "f:\\300x10a",
  "f:\\300x10b"
};



struct Data
{
	int tickCount;
	int framesSkipped;
	int maxSkipped;
};



class RobotTest
{
	public:
	RobotTest(int iterCount, int robotNum, int planeID);
	~RobotTest();

	Bool	DoTest();
	void	ResetAllTests();

	int	AverageTime();
	int	MaxTime();
	int	MinTime();

	int	AverageSkip();
	int	MaxSkip();
	int	MinSkip();

	int	AverageSkipRun();
	int	MaxSkipRun();
	int	MinSkipRun();


	Data* data;
	int	maxArraySize;
	int	arrayIndex;
	int	planeID;
	int	robotNum;
	int	frameCount;
};



void
Test()
{
	graphMgr->GCursor().Set(0, 0, 0);
	graphMgr->GCursor().SetDevice(*mouse);
	int robotNum = 300;
	if (configMgr->Get(configMgr->PlayAudio))
		robotNum = atoi(configMgr->Str(configMgr->PlayAudio));


	// Set up default plane
	Plane* defaultPlane = New Plane(SOL_Rect(0, 0, LOWRESX-1, LOWRESY-1));
	defaultPlane->SetColor(5);
	defaultPlane->SetPriority(2);
//	defaultPlane->SetPicNum(490);
//	defaultPlane->SetPicMirror(True);
	graphMgr->AddPlane(defaultPlane);
	int defaultPlaneID = defaultPlane->Id();

	Plane* backPlane = New Plane(SOL_Rect(0, LOWRESY - 62, LOWRESX - 1, LOWRESY - 1));
	backPlane->SetColor(0);
	backPlane->SetPriority(0);
	graphMgr->AddPlane(backPlane);
	graphMgr->FrameOut();
	int fd = Creat("c:\\results.txt", 0);
	char buffer[200];

	RobotTest* robotList[50];
	for (int i = 0; i < robotCount; i++)
		robotList[i] = New RobotTest(10, robotNumList[i], defaultPlaneID);

	int ch = 0;
	for (int j = 0; j < dirCount; j++)  {
		chdir(dirstr[j]);
		sprintf(buffer, "Directory: %s%c%c", dirstr[j], 0xd, 0xa);
		Write(fd, buffer, strlen(buffer));


		for (int q = 0; q < 3; q++)  {
			for (i = 0; i < robotCount; i++)  {
				robotList[i]->DoTest();
				if (kbhit()) {
					ch = toupper(getch());
					if (ch == 'Q')  {
						break;		  // quit
					}
				}
			}
			if (ch == 'Q')
				break;
		}
		if (ch == 'Q')
			break;

		for (i = 0; i < robotCount; i++)  {
			int frameCount = robotList[i]->frameCount;
			sprintf(buffer, "%d  skip: %d %d%%  \tspan: %d %d%% %c%c", robotNumList[i],
				robotList[i]->AverageSkip(),
				(robotList[i]->AverageSkip() * 100) / frameCount,
				robotList[i]->AverageSkipRun(),
				(robotList[i]->AverageSkipRun() * 100) / frameCount,
				0xd, 0xa);
			Write(fd, buffer, strlen(buffer));
			robotList[i]->ResetAllTests();
		}
	}

	Close(fd);

	for (i = 0; i < robotCount; i++)
		delete robotList[i];

	graphMgr->DeletePlane(defaultPlane);
	graphMgr->DeletePlane(backPlane);
}



RobotTest::RobotTest(int iterCount, int robotNum, int planeID) :
 robotNum(robotNum),
 planeID(planeID)

{
	maxArraySize = iterCount;
	data = New Data[maxArraySize];
	arrayIndex = 0;
	frameCount = 0;
}


RobotTest::~RobotTest()
{
	delete data;
}

void
RobotTest::ResetAllTests()
{
	arrayIndex = 0;
}

int
RobotTest::AverageTime()
{
	int total = 0;
	if (arrayIndex)  {
		for (int i = 0; i < arrayIndex; i++)
			total += data[i].tickCount;
		total /= arrayIndex;
	}
	return total;
}

int
RobotTest::MaxTime()
{
	int maxTime = data[0].tickCount;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].tickCount > maxTime)
			maxTime = data[i].tickCount;

	return maxTime;
}

int
RobotTest::MinTime()
{
	int minTime = data[0].tickCount;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].tickCount < minTime)
			minTime = data[i].tickCount;

	return minTime;
}

int
RobotTest::AverageSkip()
{
	int total = 0;
	if (arrayIndex)  {
		for (int i = 0; i < arrayIndex; i++)
			total += data[i].framesSkipped;
		total /= arrayIndex;
	}
	return total;
}

int
RobotTest::MaxSkip()
{
	int maxSkip = data[0].framesSkipped;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].framesSkipped > maxSkip)
			maxSkip = data[i].framesSkipped;

	return maxSkip;
}

int
RobotTest::MinSkip()
{
	int minSkip = data[0].framesSkipped;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].framesSkipped < minSkip)
			minSkip = data[i].framesSkipped;

	return minSkip;
}

int
RobotTest::AverageSkipRun()
{
	int total = 0;
	if (arrayIndex)  {
		for (int i = 0; i < arrayIndex; i++)
			total += data[i].maxSkipped;
		total /= arrayIndex;
	}
	return total;
}

int
RobotTest::MaxSkipRun()
{
	int maxSkip = data[0].maxSkipped;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].maxSkipped > maxSkip)
			maxSkip = data[i].maxSkipped;

	return maxSkip;
}

int
RobotTest::MinSkipRun()
{
	int minSkip = data[0].maxSkipped;
	for (int i = 1; i < arrayIndex; i++)
		if (data[i].maxSkipped < minSkip)
			minSkip = data[i].maxSkipped;

	return minSkip;
}

Bool
RobotTest::DoTest()
{
	graphMgr->InitRobot(robotNum, planeID, -1, 1, 1);
	graphMgr->GRobot().Resume();
	long startTick = timeMgr->GetTickCount();
	int state = 0;

	while (True)
	{
		graphMgr->FrameOut();
		if (!state  &&  (timeMgr->GetTickCount() - startTick) / 600)  {
			graphMgr->GRobot().Pause();
			state = 1;
		}

		if (state == 1  &&  (timeMgr->GetTickCount() - startTick) / 800)  {
			graphMgr->GRobot().Resume();
			state = 2;
		}


		if (graphMgr->RobotStatus() == ROBOT_FINAL)  {
			data[arrayIndex].tickCount = timeMgr->GetTickCount() - startTick;
			data[arrayIndex].framesSkipped = graphMgr->GRobot().FramesSkipped();
			data[arrayIndex].maxSkipped = graphMgr->GRobot().MaxFramesSeqSkipped();

			arrayIndex++;
			frameCount = graphMgr->GRobot().FrameCount();
			graphMgr->KillRobot();
			graphMgr->FrameOut();
			return True;
		}

		if (kbhit()) {
			int ch = toupper(getch());
			if (ch == 'Q')  {
				graphMgr->KillRobot();
				graphMgr->FrameOut();
				return False;
			}
		}
	}
}
