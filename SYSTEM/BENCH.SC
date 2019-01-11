
(script# 1)


(public
	Benchmark	0
)

(local
	[act 20]
	runCount = 0
	startTime	
	endTime		
	timeArray	
	actorPosX  = [ 10  10  10  10  50  50 280 280 280 280 280 280]
	actorPosY  = [ 40 100 150 190  50 180  50 150  50  90 160 200]
	actorGotoX = [280 280 280 280 280 280  10  10  50  50  50  50]
	actorGotoY = [ 40 100 150 190 170  40 190  30  50  90 160 200]
)

(procedure
	NullProcedure
	StartTimer
	StopTimer
	GetTimer
)

(extern
	SysInfo	kernel		125
)

(define vCURSORS 998)

(instance Benchmark of Room
	(properties
		picture: 	-1
	)
	(method (init)
		(super init: &rest)

		(RemapColors RemapOff)

		(= timeArray   (IntArray new: 100))

		(Print
			back:	0,
			fore: 24,
		)
		(self setScript: RunBenchmarks)
;**		(self setScript: AnimationTest)
;**		(self setScript: NetAdvTest)
;**		(self setScript: PCodeTest)
;**		(self setScript: MemoryTest)
;**		(self setScript: ListTest)
;**		(self setScript: ArrayTest)
;**		(self setScript: NetAdvTest)
	)
)

(enum
	ANIM
	PCODE
	LISTPROC
	ARRAYHANDLE
	STRHANDLE
	MEMHANDLE
	NETADV
)

(class DummyActor of Actor
	(method (posn ax ay)
		(= x ax)
		(= y ay)
	)
	(method (setPri p)
		(= priority p)
	)
	(method (setLoop l)
		(= loop l)
	)
	(method (setCel c)  
		(= cel c)
	)
)

(procedure (StartTimer whichTest whichRun)
	(= startTime (GetTime))
)

(procedure (StopTimer whichTest whichRun)
	(= endTime (- (GetTime) startTime))
	(timeArray at: (+ (* whichTest 10) whichRun) endTime)
)

(procedure (GetTimer whichTest whichRun)
	(return (timeArray at: (+ (* whichTest 10) whichRun)))
)

(procedure (ChangeGraphics)

	(if (Print 
			addText: "Do you want graphics on?",
			addButton: 1 {Yes} 10 30,
			addButton: 0 {No}  90 30, 
			init:
		)
		(SysInfo TRUE)
	else
		(SysInfo FALSE)
	)
)

(instance RunBenchmarks of Script
	(method (changeState newState &tmp ret)
		(switch (= state newState)
			(0
			 	(= ret (Print 
							addText:   {Which benchmark?},
							addButton: 1 {Animation} 		  10 20,
							addButton: 2 {P-Code}	  		  10 35,
							addButton: 3 {List processing}  10 50,
							addButton: 4 {Array handling}   10 65,
							addButton: 5 {String handling}  10 80,
							addButton: 6 {Memory handling}  10 95,
							addButton: 7 {NetAdv Test}		  10 110,
							addButton: 0 {Cancel}			  10 140,
							init:
						)
				)
				(FrameOut)
				(switch ret
					(1
						(self setScript: AnimationTest self)
					)
					(2
						(self setScript: PCodeTest self)
					)
					(3
						(self setScript: ListTest self)
					)
					(4
						(self setScript: ArrayTest self)
					)
					(5
						(self setScript: StringTest self)
					)
					(6
						(self setScript: MemoryTest self)
					)
					(7
						(self setScript: NetAdvTest self)
					)
					(else
						(= quit TRUE)
						(return)
					)
				)
			)
			(1
				(self changeState: 0)
			)
		)
	)
)

(instance actorList of Set)

;;; create 1, 4, 16 Actors, put them in cast list, and move them
;;; across the screen
(instance AnimationTest of Script
	(method (changeState newState &tmp i j)
		(switch (= state newState)
			(0
				;;; Create 16 actors
				(for ((= i 0)) (< i 16) ((++ i))
					(actorList add: 
					  (= [act i] 
						((Actor new:)
							view:	555,
							setLoop: 0,
							cel:  0,
							setStep: 2 2,
							moveSpeed: 0,
							cycleSpeed: 0,
;;							init:,
							yourself:
						)
					  )
					)
				)
				(FrameOut)
				(Load RES_VIEW 555)
				(ChangeGraphics)
				(= cycles 1)
			)
;			(1
;				(for ((= runCount 0)) (< runCount 100) ((++ runCount))
;					(for ((= j 0)) (< j 100) ((++ j))
;						(for ((= i 0)) (< i 16) ((++ i))
;							([act i]
;								x: (+ 10 i (* 10 i) j),
;								y: (+ (* 5 i) 20),
;							)
;							(UpdateScreenItem [act i])
;						)
;						(FrameOut)
;					)
;				)
;				(= quit TRUE)
;			)

			(1
				(StartTimer ANIM 1)
				(for ((= i 0)) (< i 1) ((++ i))
					((actorList at: i)
						init:,
						posn: (* 10 i) (+ (* 5 i) 20),
						setMotion: MoveTo (+ 100 i (* 10 i)) (+ (* 5 i) 20) self,
					)
				)
			)
			(2
				(StopTimer ANIM 1)

				(StartTimer ANIM 2)
				(for ((= i 0)) (< i 4) ((++ i))
					((actorList at: i)
						init:,
						posn: (* 10 i) (+ (* 5 i) 20),
						setMotion: MoveTo (+ 100 i (* 10 i)) (+ (* 5 i) 20) self,
					)
				)
			)
			(3
				(StopTimer ANIM 2)

				(StartTimer ANIM 3)
				(for ((= i 0)) (< i 16) ((++ i))
					((actorList at: i)
						init:,
						posn: (* 10 i) (+ (* 5 i) 20),
						setMotion: MoveTo (+ 100 i (* 10 i)) (+ (* 5 i) 20) self,
					)
				)
			)
			(4
				(StopTimer ANIM 3)

				(SysInfo TRUE)
				(Print 
					addTextF: {Animation results:\n 
								  For 1 actor  : %3d ticks\n  
								  For 4 actors : %3d ticks\n  
								  For 16 actors: %3d ticks\n}
								  (GetTimer ANIM  1)
								  (GetTimer ANIM  2)
								  (GetTimer ANIM  3),
					init:
				)
				(actorList dispose:)
				(self dispose:)
			)
		)
	)
)

(define PCODE_ITER	25000)

(instance PCodeTest of Script
	(method (changeState newState &tmp i j obj testX testY)
		(switch (= state newState)
			(0
				((= obj (DummyActor new:))
					x: (Random 0 200),
					y: (Random 0 200),
					view:  555,
				)
				(= testX 100)
				(= testY 50)

				(StartTimer PCODE 1)
				(for ((= i 0)) (< i PCODE_ITER) ((++ i))
				 	(obj
						setPri:  5,
						setLoop: 0,
						setCel:  0,
					)
				)
				(StopTimer PCODE 1)

				(StartTimer PCODE 2)
				(for ((= i 0)) (< i PCODE_ITER) ((++ i))
				 	(obj 
						view: 0,
						loop: 1,
						cel:  2,
						x:    5,
						y:    10,
					)
				)
				(StopTimer PCODE 2)

				(StartTimer PCODE 3)
				(for ((= i 0)) (< i PCODE_ITER) ((++ i))
					(= testX (- (/ (* (+ testY 5) 22) (+ 4 testY)) 9))
				)
				(StopTimer PCODE 3)

				(StartTimer PCODE 4)
				(for ((= i 0)) (< i PCODE_ITER) ((++ i))
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
					(= testX testY)
					(+= testY testX)
					(-= testX testY)
				)
				(StopTimer PCODE 4)

				(StartTimer PCODE 5)
				(for ((= i 0)) (< i 10) ((++ i))
					(for ((= j 0)) (< j 32000) ((++ j))
						0
					)
				)
				(StopTimer  PCODE 5)

				(Print 
					addTextF: {PCode results for %d iterations:\n 
								  Method Calls:   %3d ticks\n
								  Property refs:  %3d ticks\n  
								  Arithmetic ops: %3d ticks\n 
								  Many Opcodes:   %3d ticks\n
								  Empty Loop:     %3d ticks\n
								 }
								  PCODE_ITER
								  (GetTimer PCODE 1)
								  (GetTimer PCODE 2)
								  (GetTimer PCODE 3)
								  (GetTimer PCODE 4)
								  (GetTimer PCODE 5)
								  ,
					init:
				)
				(self dispose:)

;**				(if (> (++ runCount) 3)
;**					(= quit TRUE)
;**				else
;**					(-- state)
;**					(= cycles 1)
;**				)
			)
		)
	)
)

(procedure (NullProcedure)
	(return)
)

(define LIST_ITER	   300)
(define ARRAY_ITER	100)
(define STR_ITER	  1000)
(define MEM_ITER	    50)

(instance ListTest of Script
	(method (changeState newState &tmp i j k l node)
		(switch (= state newState)
			(0

				(= l (List new:))
				(for ((= j 0)) (< j 255) ((++ j))
					(l add: Object)
				)

				(StartTimer LISTPROC 1)
				(for ((= i 0)) (< i LIST_ITER) ((++ i))
					(for ((= j 0)) (< j 255) ((++ j))
						(= k (l at: j))
					)
				)

;;				(for ((= i 0)) (< i LIST_ITER) ((++ i))
;;					(for	((= node (FirstNode (l elements?))))
;;							node
;;							((= node (l nextNode?)))
;;						(l nextNode: (NextNode node))
;;						(= k (NodeValue node))
;;					)
;;				)
				(StopTimer  LISTPROC 1)

				(StartTimer LISTPROC 2)
				(for ((= i 0)) (< i LIST_ITER) ((++ i))
					(l eachElementDo: #name: 0)
				)
				(StopTimer  LISTPROC 2)

				(l release:)

				(StartTimer LISTPROC 3)
				(for ((= i 0)) (< i LIST_ITER) ((++ i))
					(for ((= j 0)) (< j 100) ((++ j))
						(l add: Object)
					)
					(while (l size:)
						(l delete: (l at: 0))
					)
				)
				(StopTimer  LISTPROC 3)

				(StartTimer LISTPROC 4)
				(for ((= i 0)) (< i LIST_ITER) ((++ i))
					(for ((= j 0)) (< j 255) ((++ j))
						(l add: Object)
					)
					(while (l size:)
						(l delete: (l at: 0))
					)
				)
				(StopTimer  LISTPROC 4)

				(Print 
					addTextF: {List Processing of %d iterations:\n
									at: Ticks = %3d\n
									eachElem: Ticks = %3d\n
									add/del 100 Ticks = %3d\n
									add/del 255 Ticks = %3d\n
								 }
								 LIST_ITER
								 (GetTimer LISTPROC 1)
								 (GetTimer LISTPROC 2)
								 (GetTimer LISTPROC 3)
								 (GetTimer LISTPROC 4)
								 ,
					init:
				)
				(self dispose:)
			)
		)
	)
)


(instance ArrayTest of Script
	(method (changeState newState &tmp i j l)
		(switch (= state newState)
			(0
				(StartTimer ARRAYHANDLE 1)
				(for ((= i 0)) (< i ARRAY_ITER) ((++ i))
					(= l (IntArray new:))
					(for ((= j 0)) (< j 255) ((++ j))
						(l at: j (Random 1 255))
					)
					(l dispose:)
				)

				(StopTimer  ARRAYHANDLE 1)
				(Print 
					addTextF: {Array Handling for %d iterations:\n
									Ticks = %3d\n
								 }
								 ARRAY_ITER
								 (GetTimer ARRAYHANDLE 1)
								 ,
					init:
				)
				(self dispose:)
			)
		)
	)
)


(instance StringTest of Script
	(method (changeState newState &tmp i j k str)
		(switch (= state newState)
			(0
				(StartTimer STRHANDLE 1)
				(for ((= i 0)) (< i STR_ITER) ((++ i))
					(= str (String new:))
					(str format: "test%3dtest" (Random 1 255))
					(for ((= j 0)) (< j (str size:)) ((++ j))
						(= k (str at: j))
					)
					(str dispose:)
				)
				(StopTimer  STRHANDLE 1)
				(Print 
					addTextF: {String Handling for %d iterations:\n
									Ticks = %3d\n
								 }
								 STR_ITER
								 (GetTimer STRHANDLE 1)
								 ,
					init:
				)
				(self dispose:)
			)
		)
	)
)

(instance MemoryTest of Script
	(method (changeState newState &tmp i j l x)  
		(switch (= state newState)
			(0
				(StartTimer MEMHANDLE 1)
				(= l (List new:))
				(for ((= i 0)) (< i MEM_ITER) ((++ i))
					(for ((= j 0)) (< j 255) ((++ j))
						(l 
							add: (Object new:)
						)
					)
					(while (l size:)
						((l at: 0) dispose:)
						(l delete: (l at: 0))
					)
				)
				(StopTimer  MEMHANDLE 1)
				(l dispose:)

				(StartTimer MEMHANDLE 2)
				(= l (List new:))
				(for ((= i 0)) (< i MEM_ITER) ((++ i))
					(for ((= j 0)) (< j 255) ((++ j))
						(l 
							add: Object
						)
					)
					(while (l size:)
						(l delete: (l at: 0))
					)
				)
				(StopTimer  MEMHANDLE 2)
				(l dispose:)

;**				(= l (List new:))
;**				(for ((= i 0)) (< i 3000) ((++ i))
;**					(l add: (= x (Object new:)))
;**				)
;**				(l dispose:)

				(StartTimer MEMHANDLE 3)
				(for ((= i 0)) (< i 3000) ((++ i))
					(= x (Object new:))
					(x dispose:)
				)
				(StopTimer  MEMHANDLE 3)

				(Print 
					addTextF: {Memory Processing of %d iterations:\n
									Ticks: %3d-%3d = %3d\n
									QuickCreate/Free Ticks: %3d\n
								 }
								 MEM_ITER
								 (GetTimer MEMHANDLE 1)
							 	 (GetTimer MEMHANDLE 2)
								 (- 
								 	(GetTimer MEMHANDLE 1)
								 	(GetTimer MEMHANDLE 2)
								 )
								 (GetTimer MEMHANDLE 3)
								 ,
					init:
				)
				(self dispose:)
			)
		)
	)
)

(define NUM_ACTORS	12)
(instance NetAdvTest of Script
	(method (changeState newState &tmp i)
		(switch (= state newState)
			(0
;**				(curRoom drawPic: 100)

				;;; Create 12 actors
				(for ((= i 0)) (< i NUM_ACTORS) ((++ i))
					(actorList add: 
						((Actor new:)
							view:	   560,
							loop:      0,
							cel:       0,
							setStep: 3 2,
							moveSpeed:  0,
							cycleSpeed: 0,
							yourself:
						)
					)
				)
				(FrameOut)
				(Load RES_VIEW 560)
				(ChangeGraphics)
				(= cycles 1)
			)
			(1
				(StartTimer NETADV 1)
				(for ((= i 0)) (< i NUM_ACTORS) ((++ i))
					((actorList at: i)
						init:,
						posn: [actorPosX i] [actorPosY i],
						setCycle: Walk,
						ignoreActors: TRUE,
						setMotion: MoveTo [actorGotoX i] [actorGotoY i] (if (== i 0) self else 0),
					)
				)
			)
;**			(2
;**			 	(if (> (++ runCount) 10)
;**					(= quit TRUE)
;**				else
;**					(self changeState: 1)
;**				)
;**			)
			(2
				(StopTimer NETADV 1)
				(SysInfo TRUE)
				(Print 
					addTextF: {NetAdv results:\n
								  For %d actors: %3d ticks\n
								 }
								 NUM_ACTORS
								 (GetTimer NETADV  1),
					init:
				)
				(actorList dispose:)
				(self dispose:)
			)
		)
	)
)

