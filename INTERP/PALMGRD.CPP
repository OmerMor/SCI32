#include <dos.h>
#include "sol.hpp"
#include "graphmgr.hpp"
#include "palmgrd.hpp"
#include "video.hpp"




/*
 *
 *  UpdateHardware Method
 *  =====================
 *  Could be made faster if we look closer at differences between
 *  current and next palette.  Also, we will need to do assembly
 *  to wait for vblank before outputting palette
 *
 */

void
PaletteMgrDOS::UpdateHardware()
{
   if (current != next)  {
		current = next;
		uchar* buffer = New uchar[MaxColors * 3];
		uchar* ptr = buffer;
      if (gammaValue > -1) {
		   for (int i = 0; i < MaxColors - 1; i++)  {
			   Rgb24 rgb = current[i];
			   *ptr++ = (uchar) (gamma[gammaValue][rgb.red] >> 2);
			   *ptr++ = (uchar) (gamma[gammaValue][rgb.green] >> 2);
			   *ptr++ = (uchar) (gamma[gammaValue][rgb.blue] >> 2);
		   }
      }
		else {
		   for (int i = 0; i < MaxColors - 1; i++)  {
			   Rgb24 rgb = current[i];
			   *ptr++ = (uchar) (rgb.red >> 2);
			   *ptr++ = (uchar) (rgb.green >> 2);
			   *ptr++ = (uchar) (rgb.blue >> 2);
		   }
      }

		//  Ensure that last color is always white
		*ptr++ = 63;
		*ptr++ = 63;
		*ptr++ = 63;

		SetVGAPal(buffer);
		delete buffer;
	}
}
