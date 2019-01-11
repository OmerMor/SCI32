;;;;
;;;;	WINDOW.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Unknown
;;;;	Updated:	
;;;;
;;;;	The Window class defines a visible, bordered, rectangular area of the
;;;;	screen.  A Window is used by Dialog to specify various aspects of its
;;;;	appearence.
;;;;
;;;;	Classes:
;;;;		SysWindow
;;;;		Window


(script# WINDOW)

;¯gtp¯
;¯gtp¯	This entire module has been commented out.
;¯gtp¯	It is a relic from the days of sci16!
;¯gtp¯


#if 0

(class SysWindow kindof	Object
	(properties
		left		0
		top		0
		right		0
		bottom	0

		fore		0			; foreground color
		back		15			; background color
		priority	15			; priority
		window	0			; handle/pointer to system window
		type	0				; generally	corresponds to system window types
		title		0			; text appearing in title bar if present

		;; this rectangle is the working area for X/Y centering
		;; these coordinates can define a subsection of the picture
		;; in which a window will be centered
		brLeft	0
		brTop		0
		brRight	320
		brBottom	190

		lsLeft	0
		lsTop		0
		lsRight	0
		lsBottom	0

		eraseOnly FALSE
	)

	(methods
		open
		dispose
	)

	;; Open corresponding system window structure
	;; Custom window type 0x81 indicates that system
	;; will NOT draw the window, only get a port and link into list
	(method (open)
		(= window 
			(NewWindow 
				;; port dimensions
				left
				top
				right
				bottom
				;; underBits dimensions
				lsLeft
				lsTop
				lsRight
				lsBottom

				title 
				type
				priority 
				fore
				back
			)
		)
	)
	(method (dispose)
		(if window
			(DisposeWindow window eraseOnly)
			(= window 0)
		)
		(super dispose:)
	)
)

(class Window kindof	SysWindow
	(properties
		left		0
		top		0
		right		0
		bottom	0

		fore		0			; foreground color
		back		15			; background color
		priority	-1			; priority
		underBits 0			; handle to saved region of opened window
		window	0			; handle/pointer to system window
		type	0				; generally	corresponds to system window types
		title		0			; text appearing in title bar if present

		;; this rectangle is the working area for X/Y centering
		;; these coordinates can define a subsection of the picture
		;; in which a window will be centered
		brLeft	0
		brTop		0
		brRight	320
		brBottom	190
	)

	(methods
		doit
		handleEvent
		setMapSet
		move
		moveTo
		draw
		save
		restore
		inset
		show
		draw
		open
		erase
		center
	)
	(method (center)
		;; Center the window in the working rectangle.

		(self moveTo:
			(/ (- (- brRight left) (- right left)) 2)	
			(/ (- (- brBottom top) (- bottom top)) 2)
		)	
	)

	(method (move h v)
		(+= left h)
		(+= right v)
		(+= right h)
		(+= bottom v)
	)

	(method (moveTo h v)
		(self move: (- h left) (- v top))
	)

	(method (inset h v)
		(+= left h)
		(+= top v)
		(-= right h)
		(-= bottom v)
	)
	
	(method (setMapSet &tmp mapSet)
		(= mapSet 0)
		(if (!= -1 fore)
			(|= mapSet VMAP)
		)
		(if (!= -1 priority)
			(|= mapSet PMAP)
		)
		(return mapSet)
	)

	(method (show)
		(Graph GShowBits top left bottom right (self setMapSet))
	)

;	(method (open)
;		(= lsLeft left)
;		(= lsTop top)
;		(= lsRight right)
;		(= lsBottom bottom)
;		(super open:)
;	)
	(method (draw v p)
		(if (>= argc 1)
			(= fore v)
		)
		(if (>= argc 2)
			(= priority p)
		)
		(Graph GFillRect top left bottom right (self setMapSet) fore priority)
	)
	(method (save)
		(= underBits (Graph GSaveBits top left bottom right (self setMapSet)))
	)
	(method (restore)
		(if underBits
			(Graph GRestoreBits underBits)
		)
	)

	(method (doit)
	)

	(method (handleEvent event)
		(return FALSE)
	)

	(method (dispose)
		(self restore:)
		(if window
			(DisposeWindow window)
			(= window 0)
		)
		(super dispose:)
	)

	(method (erase)
		(self draw back -1)
	)
)

#endif
