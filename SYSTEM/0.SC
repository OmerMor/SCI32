(script# 0)


(public
	Bench	0
)

(define vCURSORS 998)

(instance DummyEgo of Ego)

(class Bench of Game
	(method (init)
		(MonoOut "The game init:\n")
		(= theCursor theGameCursor)
		(= waitCursor theWaitCursor)
		(= normalCursor theArrowCursor) 
		(= userFont  999)
      (= bigFont   999)
      (= smallFont 999)

	 	(super init: &rest)

		;;; SCI32 SPECIFIC
		(= systemPlane Plane)
;**		((= systemPlane (Plane new:)) 
;**			init:
;**		)
		(thePlane setRect: 0 0 320 199)
		;;;

		(= ego DummyEgo)
		(User
			alterEgo:		ego,
			canControl:		TRUE,
			canInput:		FALSE,
		)	 

		(Print
			back: 74,
			fore:  0,
		)
		(= ego DummyEgo)

		(self newRoom: 1)
	)
)

(instance theGameCursor of Cursor
	(properties
		view				vCURSORS
		loop				0
		cel				7
	)
)

(instance theWaitCursor of Cursor
	(properties
		view				vCURSORS
		loop				0
		cel				8
	)
)

(instance theArrowCursor of Cursor
	(properties
		view			vCURSORS
		loop			0
		cel			7
	)				  
)


