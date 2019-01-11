#include <string.h>

#include "sol.hpp"
#include "graphmgr.hpp"
#include "msg.hpp"
#include "palmgr.hpp"
#include "remap.hpp"
#include "savegame.hpp"
#include "shiftpal.hpp"

int			REMAP_END_COLOR = 244;
int			START_COLOR = 0;
#ifdef MACINTOSH
	int 		REMAP_START_COLOR = 237;
	int			END_COLOR = 236;
#else
	int 		REMAP_START_COLOR = 236;
	int			END_COLOR = 235;
#endif

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
Remap::RemapByRange(int color, int start, int end, int delta, int depth)
{
	color = AddPalShift(color);
	start = AddPalShift(start);
	end   = AddPalShift(end);

	if (!InRange(color) || start < 0 || end > REMAP_START_COLOR - 1)
		return;
		 
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

   singleRemap[index]->start = start; 
   singleRemap[index]->end = end; 
   singleRemap[index]->delta = delta;
	singleRemap[index]->type = REMAP_BY_RANGE;
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

void
Remap::RemapToGray(int color, int gray, int depth)
{  
	color = AddPalShift(color);
	if (!InRange(color))
		return; 

   if (gray > 100 || gray < 0)
      msgMgr->Alert("RemapToGray percent out of range; gray = %d",gray); 
					 
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

	singleRemap[index]->gray = gray;
	singleRemap[index]->type = REMAP_TO_GRAY;
	updateFlag = True;
}

void
Remap::RemapToPercentGray(int color, int gray, int percent, int depth)
{        
	color = AddPalShift(color);
	if (!InRange(color))
		return;
							  
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
	singleRemap[index]->gray = gray;
	singleRemap[index]->type = REMAP_TO_PERCENT_GRAY;
	updateFlag = True;
}

	
void
Remap::SaveRestoreBefore(SaveGame*, Bool save)
{
	if (!save) {
   	for (int j = 0; j < REMAP_COLOR_RANGE; j++)  {
		   if (singleRemap[j]) {
			   delete singleRemap[j];
			   singleRemap[j] = NULL;
         }
	   }
		for (j = 0; j < 256; j++)
			remapOn[j] = False;
	}
}

 
void 
Remap::SaveRestore(SaveGame* saveGame, Bool save)
{ 
	saveGame->SaveRestore(remapDepth, sizeof remapDepth, save);
	saveGame->SaveRestore(&remapCount, sizeof remapCount, save);
	saveGame->SaveRestore(&remapDepthOn, sizeof remapDepthOn, save);
	saveGame->SaveRestore(&forbiddenCount, sizeof forbiddenCount, save);
	saveGame->SaveRestore(&forbiddenStart, sizeof forbiddenStart, save);
  	for (int j = 0; j < REMAP_COLOR_RANGE; j++)  {
	   saveGame->SaveRestore(&(singleRemap[j]), sizeof singleRemap[j], save);
	   if (singleRemap[j]) {
         if (!save)
            singleRemap[j] = New SingleRemap(remapColor[j]);
      	saveGame->SaveRestore(&(singleRemap[j]->type), sizeof singleRemap[j]->type, save);
      	saveGame->SaveRestore(&(singleRemap[j]->percent), sizeof singleRemap[j]->percent, save);
      	saveGame->SaveRestore(&(singleRemap[j]->gray), sizeof singleRemap[j]->gray, save);
      	saveGame->SaveRestore(&(singleRemap[j]->start), sizeof singleRemap[j]->start, save);
      	saveGame->SaveRestore(&(singleRemap[j]->end), sizeof singleRemap[j]->end, save);
      	saveGame->SaveRestore(&(singleRemap[j]->delta), sizeof singleRemap[j]->delta, save);
         if (!save)  {
				updateFlag = True;
				remapOn[REMAP_END_COLOR - j] = True;
			}
      }
   }
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


// Kernel Call

#include "kernel.hpp"

void
KRemapColors(argList)
{
	switch (arg(1)) {      
		case REMAP_OFF:
         if(argCount < 2)
			   graphMgr->GRemapper().RemapOff();
         else
			   graphMgr->GRemapper().RemapOff(arg(2));
			break;

		case REMAP_BY_RANGE:
			if(argCount < 6)     
				graphMgr->GRemapper().RemapByRange(arg(2), arg(3), arg(4), arg(5));
         else
				graphMgr->GRemapper().RemapByRange(arg(2), arg(3), arg(4), arg(5), arg(6));
			break;

		case REMAP_BY_PERCENT: 
			if(argCount < 4)     
				graphMgr->GRemapper().RemapByPercent(arg(2), arg(3));
         else
				graphMgr->GRemapper().RemapByPercent(arg(2), arg(3), arg(4));
			break;

		case REMAP_TO_GRAY: 
			if(argCount < 4)     
				graphMgr->GRemapper().RemapToGray(arg(2), arg(3));
         else
				graphMgr->GRemapper().RemapToGray(arg(2), arg(3), arg(4));
			break;

		case REMAP_TO_PERCENT_GRAY:
			if (argCount < 5)     
				graphMgr->GRemapper().RemapToPercentGray(arg(2), arg(3), arg(4));
         else
				graphMgr->GRemapper().RemapToPercentGray(arg(2), arg(3), arg(4), arg(5));
			break;

		case REMAP_SET_GLOBAL_NOMATCH_RANGE:
			graphMgr->GRemapper().DontMapToRange(arg(2), arg(3));
			break;
			
   }
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
Remap::SingleRemap::ByRange()
{
	Bool changed = False;

	for (int i = 0; i < 236; i++)  {
		int result;
      if (start <= i  &&  i <= end)
			result = i + delta;
      else
			result = i;

		if (remap[i] != result)  {
			changed = True;
			remap[i] = result;
		}
		colorChanged[i] = True; // keep color changed true to assure new remaps are
                              // clean
	}
	return changed;
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
Remap::SingleRemap::ToGray()
{
	uchar targetChanged[236];

	memset(targetChanged, False, 236);
	const SOLPalette& palette = graphMgr->GPalette().Current();
	for (int i = 1; i < 236; i++) {
		Rgb24 color = palette[i];
		if (colorArray[i] != color)  {
			colorChanged[i] = True;
			colorArray[i] = color;
		}

      if (gray != oldGray  || colorChanged[i]) {
   		int lum = color.Lum();
	   	int red = color.red - ((color.red - lum) * gray / 100);
		   int green = color.green - ((color.green - lum) * gray / 100);
   		int blue = color.blue - ((color.blue - lum) * gray / 100);

	   	Rgb24 newcolor(red, green, blue);
		   if (targetColor[i] != newcolor)  {
			   targetChanged[i] = True;
			   targetColor[i] = newcolor;
         }
      }
	}

	Bool remapChanged = Apply(targetChanged);
	memset(colorChanged, False, 236);
   oldGray = gray;
	return remapChanged;
}


Bool
Remap::SingleRemap::ToPercentGray()
{
	uchar targetChanged[236];

	memset(targetChanged, False, 236);
	const SOLPalette& palette = graphMgr->GPalette().Current();
	for (int i = 1; i < 236; i++) {
		Rgb24 color = palette[i];
		if (colorArray[i] != color)  {
			colorChanged[i] = True;
			colorArray[i] = color;
		}

      if (percent != oldPercent || gray != oldGray  || colorChanged[i]) {
   		int lum = color.Lum();
			lum = lum * percent / 100;
	   	int red = color.red - ((color.red - lum) * gray / 100);
		   int green = color.green - ((color.green - lum) * gray / 100);
   		int blue = color.blue - ((color.blue - lum) * gray / 100);

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
   oldGray = gray;
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
	if (!WING) {
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
	}
	else {
		int	j;
		for (i = 1; i < 236; i++)  {
			j = i + PALSHIFT;

			int distance;
			//	 If the color and remap color didn't change, check new list only
			if (!targetChanged[i]  &&  !colorChanged[remap[j] - PALSHIFT])  {
				//  NEW **** Ignore search for better - has failed KQ7 - DF - 10/26/94
				continue;
			}

			//  If they both changed and within previous tolerance, don't search
			if (targetChanged[i]  &&  colorChanged[remap[j] - PALSHIFT])
				if (dist[i] < 100  &&  targetColor[i].Dist(colorArray[remap[j] - PALSHIFT]) <= dist[i])
					continue;

			int result = graphMgr->GPalette().Next().Match(targetColor[i], dist[i], &distance, colorForbidden);
			if (result != -1  &&  remap[j] != result + PALSHIFT)  {
				changed = True;
				remap[j] = result + PALSHIFT;
				dist[i] = distance;
			}
		}
	}

	delete [] newcolors;
	return changed;
}


Bool
Remap::SingleRemap::Update(Bool palChanged)
{
	if (Remap::updateFlag  ||  palChanged)  {
		if (type == REMAP_OFF)
			return False;
		
		if (type == REMAP_BY_RANGE)
			return ByRange();

		if (type == REMAP_TO_GRAY)
			return ToGray();

		if (type == REMAP_BY_PERCENT)
			return ByPercent();

		if (type == REMAP_TO_PERCENT_GRAY)
			return ToPercentGray();

	   msgMgr->Fatal(SrcLoc, "Illegal remap type %d", type);
	}

   return False;
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