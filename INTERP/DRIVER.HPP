/* DRIVER.H
 * Definitions for drivers.
 */

/* Magic number to identify drivers.
 */
#define	DRIVER	0x87654321L

/* Driver types.
 */
#define	D_VIDEO		0
#define	D_MUSIC		1
#define	D_VOICE		2
#define	D_INPUT		3
#define	D_KEYBOARD	4
#define	D_COMM		5
#define	D_MOUSE		6
#define	D_MEMORY		7	/* type used for autodetection purposes only	*/
#define	D_PRINTER	8

/* Required driver functions.
 */
#define	D_DETECT		(0 * sizeof(void *))
#define	D_INIT		(1 * sizeof(void *))
#define	D_TERMINATE	(2 * sizeof(void *))

#define EXTDRV		0xFEDCBA98

/* extended driver bits	*/

/*	video	*/
#define MDA_BIT		0x0001
#define HERCULES_BIT	0x0002
#define CGA_BIT		0x0004
#define IBMPCJR_BIT	0x0008
#define TANDY_BIT		0x0010
#define EGA_BIT		0x0020
#define MCGA_BIT		0x0040
#define VGA_BIT		0x0080
#define CGAM_BIT		0x0100
#define CGAC_BIT		0x0200
#define EXPLORER_BIT	0x0400

/*	keyboard	*/
#define IBMKBD_BIT	1
#define TANDYKBD_BIT	2

/*	sound	*/
#define STD_SPEAKER	0x0001
#define ADLIB			0x0002
#define SBLAST_DAC	0x0004
#define CMS				0x0008
#define TANDY_3VOICE	0x0010
#define TANDY_DAC		0x0020
#define PS1_3VOICE	0x0040
#define PS1_DAC		0x0080
#define IMF				0x0100
#define MPU_MIDI		0x0200
#define DISNEY_DAC	0x0400
#define CDAUDIO_OUT	0x0800
