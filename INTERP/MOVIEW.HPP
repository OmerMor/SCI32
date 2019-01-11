#if !defined(MOVIEW_HPP)
#define MOVIEW_HPP

extern "C" {
unsigned int	AviOpen(int movieNo,char * filename);   
unsigned int	AviClose(int movieNo);
unsigned int	AviPut(int movieNo,int x1, int y1, int x2, int y2);
unsigned int	AviPutDouble(int movieNo,int x1, int y1);
unsigned int	AviPlay(int movieNo,int from, int to, int PlayStyle, int cue);
unsigned int	AviStop(int movieNo);
unsigned int	AviPause(int movieNo);
unsigned int	AviResume(int movieNo);
unsigned int	AviSetPalette(int movieNo);
unsigned int	AviGetLength(int movieNo);
unsigned int	AviGetPosition(int movieNo);
unsigned int	AviGetStatus(int movieNo);
unsigned int   AviCue(int movieNo,int frameNo);
unsigned int	AviSeek(int movieNo, int frameNo);
unsigned int   AviGetFramesSkipped(int movieNo);
unsigned int	AviWaitEvent(int,int events);
void			AviNotify(WPARAM,LPARAM);

enum avi_error {
	AVI_NO_ERROR = 0,
	AVI_DEVICE_NOT_OPEN,
	AVI_OPEN_FAIL,
	AVI_CLOSE_FAIL,
	AVI_WHERE_FAIL,
	AVI_PUT_FAIL,
	AVI_PLAY_FAIL,
	AVI_STOP_FAIL,
	AVI_PAUSE_FAIL,
	AVI_RESUME_FAIL,
	AVI_NOT_PLAYING,
	AVI_PALETTE_FAIL,
	AVI_CUE_FAIL
};

// the 3 consts following are bit flags for the AVI_
const WORD AVI_COMPLETED = 1;
const WORD AVI_ESC_PRESSED = 2;
const WORD	AVI_MOUSE_DOWN = 4;
const WORD AVI_HOT_RECTANGLE = 8;

enum avi_play_type {
	PLAY_DEFAULT = 0,
	PLAY_FULLSCREEN,
	SCI_WINDOW
};

enum avi_status {
	statusNotOpen,
	statusOpen,
	statusPlaying,
	statusPaused,
	statusStopped,
	statusDone,
};

enum avi_command_list {
      AVI_OPEN = 0,
      AVI_PUT,
      AVI_PLAY,
      AVI_STOP,
      AVI_PAUSE,
      AVI_RESUME,
      AVI_CLOSE,
	   AVI_SETPALETTE,
	   AVI_GET_LENGTH,
	   AVI_GET_POSITION,
	   AVI_GET_STATUS,
	   AVI_CUE,
	   AVI_SEEK,
	   AVI_FRAMES_SKIPPED,
	   AVI_WAIT_EVENT,
	   AVI_PUT_DOUBLE
};
extern short movieType;

}
#endif

