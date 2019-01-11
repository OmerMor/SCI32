#include <string.h>

#include "sol.hpp"
#include "graphmgr.hpp"
#include "msg.hpp"
#include "palmgr.hpp"
#include "remap.hpp"
#include "savegame.hpp"
#include "shiftpal.hpp"
	
int 			REMAP_START_COLOR = 236;
int			REMAP_END_COLOR = 244;
int			START_COLOR = 0;
int			END_COLOR = 235;

uchar			Remap::remapColor[REMAP_COLOR_RANGE][256];
uint			Remap::remapDepth[REMAP_COLOR_RANGE];
uchar			Remap::remapOn[256];
uint			Remap::remapCount;
uint			Remap::remapDepthOn;
int			Remap::forbiddenCount;
int			Remap::forbiddenStart;
Bool			Remap::updateFlag;


Remap::Remap()
{
	static Bool inited = False;

	if (!inited)  {
		REMAP_START_COLOR = AddPalShift(REMAP_START_COLOR);
		REMAP_END_COLOR = AddPalShift(REMAP_END_COLOR);
		START_COLOR = AddPalShift(START_COLOR);
		END_COLOR = AddPalShift(END_COLOR);
		inited = True;
	}

	remapCount = 0;
	remapDepthOn = 0;
	forbiddenCount = 0;
	forbiddenStart = 0;
	updateFlag = False;
	for (int i = 0; i < REMAP_COLOR_RANGE; i++)
		singleRemap[i] = NULL;

	for (i = 0; i < 256; i++)
		remapOn[i] = False;
}

Remap::~Remap()
{   
	for (int i = 0; i < REMAP_COLOR_RANGE; i++)
		delete singleRemap[i];
}

void
Remap::RemapOff(int color) 
{
   if (!color) {
	   for (int i = 0; i < REMAP_COLOR_RANGE; i++) {
			delete singleRemap[i];
			singleRemap[i] = NULL;
	   }

		for (i = 0; i < 256; i++)
			remapOn[i] = False;

      remapCount = 0;
		remapDepthOn = 0; 
		updateFlag = True;
      return;
   }

	//  If not in range or already off, return
	color = AddPalShift(color);
	if (!InRange(color))
		return;
	const int index = REMAP_END_COLOR - color;
	if (!singleRemap[index])
		return;

   if (singleRemap[index]->Type() == REMAP_OFF)
      msgMgr->Fatal(SrcLoc, "Remap Should not be off");

	//  Take down remapCount, remapDepth on if applicable
	remapCount--;
	remapOn[color] = False;
	if (remapDepth[index])
		remapDepthOn--;

	delete singleRemap[index];
	singleRemap[index] = NULL;
	updateFlag = True;
}

void
Remap::RemapByPercent(int color, int percent, int depth)
{  
	color = AddPalShift(color);
	if (!InRange(color))
		return;

   if (percent < 0)
      msgMgr->Alert("RemapByPercent percent out of range; percent = %d",percent);
							  
	const int index = REMAP_END_COLOR - color;
	if (!singleRemap[index])  {
		singleRemap[index] = New SingleRemap(remapColor[index]);
		remapCount++;
		remapOn[color] = True;
	}

   //  if remapDepth was on reduce the count, not to worry if depth is set again it will be added
   if (remapDepth[index])
		remapDepthOn--;

	remapDepth[index] = depth;
	if (depth)
		remapDepthOn++;

	singleRemap[index]->percent = percent;
	singleRemap[index]->type = REMAP_BY_PERCENT;
	updateFlag = True;
}


Bool
Remap::RemapAllTables(Bool palChanged)
{		            
	Bool changed = False;
	for (int j = 0; j < REMAP_COLOR_RANGE; j++)  {
		if (singleRemap[j])
			changed |= singleRemap[j]->Update(palChanged);
	}
	updateFlag = False;
	return changed;
}

void
Remap::DontMapToRange(int start, int count)
{
	start = AddPalShift(start);
	forbiddenStart = start;
	forbiddenCount = count;
}



/////////////////////////////////////////////////////////////


Remap::SingleRemap::SingleRemap(uchar* remapTable)
{
	type = REMAP_OFF;
	remap = remapTable;
	percent = 100;
   oldPercent = 100;
	gray = 0;
	oldGray = 0;
	for (int i = 0; i < 236; i++)  {
		Rgb24 color = graphMgr->GPalette().Current()[i];
		colorArray[i] = color;
		targetColor[i] = color;
		dist[i] = 0;
		remap[i] = i;
		colorChanged[i] = True;
	}
	for (i = 236; i < 246; i++)		// ****** ???????  WRONG
		remap[i] = i;
}




Bool
Remap::SingleRemap::ByPercent()
{
	uchar targetChanged[236];

	memset(targetChanged, False, 236);

	const SOLPalette& palette = graphMgr->GPalette().Next();
	for (int i = 1; i < 236; i++ ) {
		Rgb24 color = palette[i];

		if (colorArray[i] != color)  {
			colorChanged[i] = True;
			colorArray[i] = color;
		}

		if (percent != oldPercent  || colorChanged[i])  {
			int red = color.red * percent / 100;
			int green = color.green * percent / 100;
			int blue = color.blue * percent / 100;

			if (percent > 100)  {
				if (red > 255)
					red = 255;
				if (green > 255)
					green = 255;
				if (blue > 255)
					blue = 255;
			}

			Rgb24 newcolor(red, green, blue);
			if (targetColor[i] != newcolor)  {
				targetChanged[i] = True;
				targetColor[i] = newcolor;
			}
		}
	}

	Bool remapChanged = Apply(targetChanged);
	memset(colorChanged, False, 236);
   oldPercent = percent;
	return remapChanged;
}



Bool
Remap::SingleRemap::Apply(uchar* targetChanged)
{
	uchar realIndex[236];
	uchar colorForbidden[236];

   const uchar* cycleMap = graphMgr->GPalette().CycleMap();
	memset(colorForbidden, False, 236);
	if (forbiddenCount)
		memset(colorForbidden + forbiddenStart, True, forbiddenCount);

	for (int i = 0; i < 236; i++)  {
		if (cycleMap[i])
			colorForbidden[i] = True;
	}


	//  Update the distances on everyone whose 
	Rgb24* newcolors = New Rgb24[236];
	int newCount = 0;
	for (i = 1; i < 236; i++)  {
		if (colorChanged[i]  &&  !colorForbidden[i])  {
			newcolors[newCount] = colorArray[i];
			realIndex[newCount] = i;
			newCount++;
		}
	}

	Bool changed = False;
#ifdef WIN32S
	if (!WING) {
#endif
	for (i = 1; i < 236; i++)  {

		int distance;
		//	 If the color and remap color didn't change, check new list only
		if (!targetChanged[i]  &&  !colorChanged[remap[i]])  {
			//  NEW **** Ignore search for better - has failed KQ7 - DF - 10/26/94
			continue;
		}

		//  If they both changed and within previous tolerance, don't search
		if (targetChanged[i]  &&  colorChanged[remap[i]])  {
			if (dist[i] < 100  &&  targetColor[i].Dist(colorArray[remap[i]]) <= dist[i])  {
				continue;
			}
		}

		int result = graphMgr->GPalette().Next().Match(targetColor[i], dist[i], &distance, colorForbidden);
		if (result != -1  &&  remap[i] != result)  {
			changed = True;
			remap[i] = result;
			dist[i] = distance;
		}
	}
#ifdef WIN32S
	} else {
	int	j;
	for (i = 1; i < 236; i++)  {
		j = i + PALSHIFT;

		int distance;
		//	 If the color and remap color didn't change, check new list only
		if (!targetChanged[i]  &&  !colorChanged[remap[j]])  {
			//  NEW **** Ignore search for better - has failed KQ7 - DF - 10/26/94
			continue;
		}

		//  If they both changed and within previous tolerance, don't search
		if (targetChanged[i]  &&  colorChanged[remap[j]])
			if (dist[i] < 100  &&  targetColor[i].Dist(colorArray[remap[j]]) <= dist[i])
				continue;

		int result = graphMgr->GPalette().Next().Match(targetColor[i], dist[i], &distance, colorForbidden);
		if (result != -1  &&  remap[j] != result)  {
			changed = True;
			remap[j] = result + PALSHIFT;
			dist[i] = distance;
		}
	}
	}
#endif

	delete [] newcolors;
	return changed;
}


inline int sqr(int i) {return i * i;}

int
Remap::SingleRemap::Match(Rgb24* colors, int count, const Rgb24& color, int diff, int& newDiff)
{
	int index = -1;
	int current;
	int red = color.red;
	int green = color.green;
	int blue = color.blue;

	for (int i = 0; i < count; i++) {
		current = sqr((int)(colors[i].red) - red);
		if (diff > current) {
			current += sqr((int)(colors[i].green) - green);
			if (diff > current) {
				current += sqr((int)(colors[i].blue) - blue);
				if (diff > current) {
					diff = current;
					index = i;
				}
			}
		}
	}

   newDiff = diff;
	return index;

}
