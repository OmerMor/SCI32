
extern "C" {
unsigned int	AviOpen(int movieNo,char * filename);   
unsigned int	AviClose(int movieNo);
unsigned int	AviPut(int movieNo,int x1, int y1, int x2, int y2);
unsigned int	AviPlay(int movieNo,int from, int to, int PlayStyle, int cue);
unsigned int	AviStop(int movieNo);
unsigned int	AviPause(int movieNo);
unsigned int	AviResume(int movieNo);
unsigned int	AviSetPalette(int movieNo);

extern	WORD	AVIDeviceID;
extern	WORD	avi_cue_who;
extern	BOOL	avi_playing;
extern	BOOL	avi_paused;
extern	BOOL	avi_fullscreen;
extern   short movieType;

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
	AVI_PALETTE_FAIL
};

enum avi_play_type {
	PLAY_DEFAULT = 0,
	PLAY_FULLSCREEN,
	SCI_WINDOW
};



enum avi_command_list {
      AVI_OPEN = 0,
      AVI_PUT,
      AVI_PLAY,
      AVI_STOP,
      AVI_PAUSE,
      AVI_RESUME,
      AVI_CLOSE,
	   AVI_SETPALETTE
};

}
