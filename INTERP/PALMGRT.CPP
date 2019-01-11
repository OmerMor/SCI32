#include <stdlib.h>
#include <string.h>

#include "sol.hpp"
#include "palmgrt.hpp"
#include "graphmgr.hpp"
#include "time.hpp"

Bool
PaletteMgrTrue::UpdateForFrame()
{
	ApplyAll();
	needUpdate = False;

	Bool paletteChanged = (current != next);
	return graphMgr->GRemapper().RemapAllTables(paletteChanged);
}

void
PaletteMgrTrue::ApplyAll()
{
	ApplyVary();
	ApplyCycles();
	ApplyFade();
}

void
PaletteMgrTrue::ApplyCycles()
{
	for (int i = 0; i < MAXCYCLES; i++) {
		if (cycle[i]) {

			if (cycle[i]->deltaTime && !(cycle[i]->pause)) {

				while (cycle[i]->lastUpdate + cycle[i]->deltaTime < timeMgr->GetTickCount()) {
					if (cycle[i]->forward)
						cycle[i]->current = (cycle[i]->current + 1) % cycle[i]->range;
					else
						cycle[i]->current = ((cycle[i]->current - 1 + cycle[i]->range) % cycle[i]->range);

					cycle[i]->lastUpdate += cycle[i]->deltaTime;
				}      
			}
		}
	}
}
	  

void    
PaletteMgrTrue::ApplyVary()
{
	while((int)(timeMgr->GetTickCount() - timeLastVary) > deltaTimeVary && varyDirection) {
		timeLastVary += deltaTimeVary;
		if (varyPercent == targetVaryPercent)
			varyDirection = 0;

		varyPercent += varyDirection;
	}

	for (int i = 0; i < MaxColors; i++)
		next[i] = source[i];
}


void
PaletteMgrTrue::ApplyFade()
{
}




