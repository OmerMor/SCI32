;;;;
;;;;	BORDWIND.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Unknown
;;;;	Updated:	Brian K. Hughes
;;;;
;;;;	A bordered window.
;;;;
;;;;	Classes:
;;;;		BorderWindow
;;;;		InsetWindow
;;;;
;;;;	Procedures:
;;;;		DrawBeveledWindow
;;;;		PushOnScreen


(script# BORDWIND)

;¯gtp¯
;¯gtp¯	This entire module has been commented out.
;¯gtp¯	It is a relic from the days of sci16!
;¯gtp¯

#if 0

(procedure 
	DrawBeveledWindow
	PushOnScreen
)

(class BorderWindow kindof	SysWindow
	(properties
		fore				0		; foreground color
		back				5		; background color

		lftBordColor	6
		topBordColor	7
		rgtBordColor	4
		botBordColor	3
		
		bevelWid			3
		shadowWid		2
		
		lsLeft			0
		lsTop				0
		lsRight			0
		lsBottom			0
		eraseOnly		FALSE
	)
	
	(method (open &tmp savePort wMap)
		(SetPort 0)

		(= wMap VMAP)
		(if (!= priority -1)
			(|= wMap PMAP)
		)
;		(PushOnScreen self)
		
		(= lsLeft (- left bevelWid))
		(= lsTop (- top bevelWid))
		(= lsRight (+ right bevelWid shadowWid))
		(= lsBottom (+ bottom bevelWid shadowWid))
 		(= type $80)
		(super open:)
		(DrawBeveledWindow
			left top right bottom
			back lftBordColor topBordColor rgtBordColor botBordColor
			bevelWid
			shadowWid
			priority
			wMap
		)
		(= savePort (GetPort))
		(SetPort 0)
		(Graph GShowBits lsTop lsLeft lsBottom lsRight VMAP)
		(SetPort savePort)
 	)

	(method (dispose)
		(super dispose:)
		(SetPort 0)
	)
)



(class InsetWindow kindof BorderWindow
	(properties
		fore				0
		ck					3

		lftBordColor	4
		topBordColor	5
		rgtBordColor	2
		botBordColor	1

		;; colors for inset
		insideColor		2
		lftBordColor2	1
		topBordColor2	0
		rgtBordColor2	4
		botBordColor2	5

		topBordHgt		10
		botBordHgt		24
		sideBordWid		2

		shadWid			0
		bevWid			2

		bevelWid			3
		shadowWid		2
		
		xOffset			0	; offset of inset window from port
		yOffset			0
	)

	(method (open &tmp wMap savePort saveTop saveLeft saveBot saveRight)
		(= wMap VMAP)
		(if (!= priority -1)
			(|= wMap PMAP)
		)
		
		(= saveLeft left)
		(= saveTop top)
		(= saveRight right)
		(= saveBot bottom)
		
		(-= left (+ bevelWid sideBordWid))
		(-= top (+ bevelWid topBordHgt))
		(+= right (+ bevelWid sideBordWid))
		(+= bottom (+ bevelWid botBordHgt))
		
		(= xOffset (+ bevelWid sideBordWid))
		(= yOffset (+ bevelWid topBordHgt))
		
		(super open:)
		
		(DrawBeveledWindow
			saveLeft saveTop saveRight saveBot
			insideColor lftBordColor2 topBordColor2 rgtBordColor2 botBordColor2
			bevWid
			shadWid
			priority
			wMap
		)

		(= savePort (GetPort))
		(SetPort 0)
		(Graph GShowBits
			(- saveTop bevWid) (- saveLeft bevWid)
			(+ saveBot bevWid) (+ saveRight bevWid)
			VMAP
		)
		(SetPort savePort)
	)
)



(procedure (DrawBeveledWindow
						l t r b
						theColor leftColor topColor rightColor bottomColor
						theBevelWid theShadowWid thePri theMaps
						&tmp savePort i
			  )
	(= savePort (GetPort))
	(SetPort 0)
	(Graph GFillRect t l (+ b 1) (+ r 1) theMaps theColor thePri)

	(-= l theBevelWid)
	(-= t theBevelWid)
	(+= r theBevelWid)
	(+= b theBevelWid)

	; draw top and bottom bevels
	(Graph GFillRect
		t l (+ t theBevelWid) r
		theMaps topColor thePri
	)
	(Graph GFillRect
		(- b theBevelWid) l b r
		theMaps bottomColor thePri
	)
	(for ((= i 0)) (< i theBevelWid) ((++ i))
		(Graph GDrawLine
			(+ t i) (+ l i) (- b (+ i 1)) (+ l i)
			leftColor thePri -1
		)
		(Graph GDrawLine
			(+ t i) (- r (+ i 1)) (- b (+ i 1)) (- r (+ i 1))
			rightColor thePri	-1
		)
	)
	
	(if theShadowWid
		(Graph GFillRect
			(+ t theShadowWid) r
			(+ b theShadowWid) (+ r theShadowWid)
			theMaps 0 thePri
		)
		(Graph GFillRect
			b (+ l theShadowWid)
			(+ b theShadowWid) r
			theMaps 0 thePri
		)
	)
	
	(SetPort savePort)
)



(procedure (PushOnScreen wind &tmp dX dY)
	(define LEFTBORDER	5)
	(define TOPBORDER		10)
	(define RIGHTBORDER	315)
	(define BOTTOMBORDER	185)
	;; top
	(= dY 
		(cond
			((> (wind bottom?) BOTTOMBORDER)	
				(- BOTTOMBORDER (wind bottom?))
			)
			((< (wind top?) TOPBORDER)
				(- TOPBORDER (wind top?))
			)
			(else
				0
			)
		)
	)
	(= dX
		(cond
			((> (wind right?) RIGHTBORDER)
				(- RIGHTBORDER (wind right?))
			)
			((< (wind left?) LEFTBORDER)
				(- LEFTBORDER (wind left?))
			)
			(else
				0
			)
		)
	)
	(wind 
		left:		(+ (wind left?) 	dX),
		top:		(+ (wind top?) 	dY),
		right:	(+ (wind right?) 	dX),
		bottom:	(+ (wind bottom?) dY),
	)
)


#endif
