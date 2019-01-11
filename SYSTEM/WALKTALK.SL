
				Listing of walktalk.sc:	[script 64960]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$44


Variables:
		    2	word	$0



Object: WalkieTalkie        
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $18  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $38  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $24  	(-script-)
		   10	prop  $0   	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $36  	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(robot)
		   1a	prop  $0   	(x)
		   1c	prop  $0   	(y)
		   1e	prop  $ffff	(priority)
		   20	prop  $0   	(plane)
		   22	prop  $0   	(mode)
		   24	prop  $0   	(client)
		   26	prop  $0   	(signal)
		   28	prop  $0   	(caller)
		   2a	prop  $0   	(started)
		   2c	prop  $0   	(completed)
		   2e	prop  $0   	(holdOnLastCel)
		   30	prop  $1   	(disposeWhenDone)
		   32	prop  $1   	(hideClient)
		   34	word	$0



		   36	text
"WalkieTalkie"

		   43	byte	$0


Fixups:
		   44	word	$1
		   46	word	$14




----------------------
-------- Hunk --------
----------------------

		    0	word	$530
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: WalkieTalkie        
		(property dictionary)
		    8	word	$1000
		    a	word	$1001
		    c	word	$1002
		    e	word	$1003
		   10	word	$1004
		   12	word	$1005
		   14	word	$1006
		   16	word	$1007
		   18	word	$20
		   1a	word	$8c
		   1c	word	$112
		   1e	word	$1
		   20	word	$2
		   22	word	$7
		   24	word	$0
		   26	word	$27
		   28	word	$36
		   2a	word	$1a
		   2c	word	$b1
		   2e	word	$113
		   30	word	$fc
		   32	word	$114
		   34	word	$115
		   36	word	$116
		(method dictionary)
		   38	word	$a
		   3a	word	$8e
		   3c	local $62  	(init)
		   3e	word	$45
		   40	local $155 	(doit)
		   42	word	$8f
		   44	local $ce  	(dispose)
		   46	word	$a8
		   48	local $477 	(start)
		   4a	word	$117
		   4c	local $512 	(terminate)
		   4e	word	$118
		   50	local $1bb 	(doneYet)
		   52	word	$119
		   54	local $37e 	(robotDone)
		   56	word	$11a
		   58	local $2d4 	(killRobot)
		   5a	word	$11b
		   5c	local $3ec 	(showFrame)
		   5e	word	$11c
		   60	local $1da 	(doRobot)


Method: (WalkieTalkie init)

		   65	file
		   75	lap  	$0   	(argc)
		   77	bnt  	$8   	(.1)
		   79	lap  	$1   	(cl)
.0
		   7b	bnt  	$4   	(.1)
		   7d	lap  	$1   	(cl)
		   7f	aTop 	$20  	(client)
.1
		   84	lsp  	$0   	(argc)
		   86	ldi  	$1   
		   88	gt?
		   89	bnt  	$15  	(.3)
		   8e	lap  	$2   	(rb)
		   90	aTop 	$14  	(robot)
		   95	lsp  	$0   	(argc)
		   97	ldi  	$2   
		   99	gt?
		   9a	bnt  	$4   	(.2)
		   9c	lap  	$3   	(c)
		   9e	aTop 	$24  	(caller)
.2
.3
		   a3	pToa 	$1c  	(plane)
		   a5	not
		   a6	bnt  	$12  	(.6)
		   ab	pToa 	$20  	(client)
		   ad	bnt  	$7   	(.4)
		   af	push0
		   b0	push0
		   b1	send
		   b1	word	$4
		   b4	jmp  	$2   	(.5)
.4
		   b6	lag  	$3   	(thePlane)
.5
		   b8	aTop 	$1c  	(plane)
.6
		   bd	ldi  	$0   
		   bf	aTop 	$22  	(signal)
		   c1	aTop 	$28  	(completed)
		   c3	aTop 	$26  	(started)
		   c8	selfID
		   c9	ret
		   cd	ret


Method: (WalkieTalkie dispose)

		   d1	file
		   e4	pToa 	$26  	(started)
		   e6	bnt  	$23  	(.1)
		   eb	pToa 	$28  	(completed)
		   ed	not
		   ee	bnt  	$1b  	(.1)
		   f3	lap  	$0   	(argc)
		   f5	bnt  	$14  	(.1)
		   fa	lap  	$1   	(doDone)
.0
		   fc	bnt  	$d   	(.1)
		  104	pushi	$119 
		  107	push0
		  108	self
		  108	word	$4
.1
		  10e	ldi  	$0   
		  110	aTop 	$22  	(signal)
		  112	aTop 	$28  	(completed)
		  114	aTop 	$26  	(started)
		  119	pToa 	$20  	(client)
		  11b	bnt  	$8   	(.2)
		  11d	pushi	$112 
		  120	push1
		  121	push0
		  122	send
		  122	word	$6
.2
		  128	lsg  	$1c  	(autoRobot)
		  12a	selfID
		  12b	eq?
		  12c	bnt  	$4   	(.3)
		  12e	ldi  	$0   
		  130	sag  	$1c  	(autoRobot)
.3
		  135	pToa 	$24  	(caller)
		  137	bnt  	$d   	(.4)
		  139	pushi	$b3  
		  13c	push1
		  13d	pushi	$ffff
		  13f	send
		  13f	word	$6
		  142	ldi  	$0   
		  144	aTop 	$24  	(caller)
.4
		  149	pushi	$8f  
		  14c	push0
		  14d	super	$0   	(Object)
		  14d	word	$4
		  154	ret


Method: (WalkieTalkie doit)

		  158	file
		  165	link 	$1   
		  16a	pToa 	$26  	(started)
		  16c	bnt  	$49  	(.4)
		  171	push2
		  172	pushi	$8   
		  174	pushSelf
		  175	callk	$3a  	(Robot)
		  175	word	$4
		  17c	pToa 	$22  	(signal)
		  17e	bnt  	$37  	(.3)
		  183	sat  	$0   	(cueValue)
		  188	ldi  	$0   
		  18a	aTop 	$22  	(signal)
		  18f	pushi	$ffff
		  191	lat  	$0   	(cueValue)
		  193	eq?
		  194	bnt  	$e   	(.0)
		  199	pushi	$119 
		  19c	push1
		  19d	pTos 	$2c  	(disposeWhenDone)
		  19f	self
		  19f	word	$6
		  1a2	jmp  	$13  	(.2)
.0
		  1aa	pToa 	$24  	(caller)
		  1ac	bnt  	$9   	(.1)
		  1ae	pushi	$b3  
		  1b1	push1
		  1b2	lst  	$0   	(cueValue)
		  1b4	send
		  1b4	word	$6
.1
.2
.3
.4
		  1ba	ret


Method: (WalkieTalkie doneYet)

		  1be	file
		  1ce	push1
		  1cf	pushi	$5   
		  1d1	callk	$3a  	(Robot)
		  1d1	word	$2
		  1d5	ret
		  1d9	ret


Method: (WalkieTalkie doRobot)

		  1dd	file
		  1ea	link 	$9   
		  1f2	ldi  	$0   
		  1f4	sat  	$1   	(myX)
		  1f9	ldi  	$0   
		  1fb	sat  	$2   	(myY)
		  200	ldi  	$0   
		  202	sat  	$3   	(myCaller)
		  207	lag  	$0   	(ego)
		  209	sat  	$4   	(myObj)
		  20e	ldi  	$ffff
		  210	sat  	$5   	(myPri)
		  215	ldi  	$0   
		  217	sat  	$6   	(myHold)
		  21c	ldi  	$0   
		  21e	sat  	$7   	(alaMode)
		  223	ldi  	$1   
		  225	sat  	$8   	(i)
.0
		  227	lst  	$8   	(i)
		  229	lap  	$0   	(argc)
		  22b	lt?
		  22c	bnt  	$d   	(.1)
		  231	lat  	$8   	(i)
		  233	lspi 	$1   	()
		  235	sati 	$0   	()
		  237	+at  	$8   	(i)
		  239	jmp  	$ffec	(.0)
.1
		  241	lat  	$4   	(myObj)
		  243	bnt  	$12  	(.2)
		  248	pushi	$11d 
		  24b	pushi	$3   
		  24e	pushSelf
		  24f	push1
		  250	lsp  	$1   	(which)
		  252	send
		  252	word	$a
		  255	jmp  	$25  	(.5)
.2
		  260	pushi	$8e  
		  263	push2
		  264	push0
		  265	lsp  	$1   	(which)
		  267	pTos 	$e   	(-info-)
		  269	ldi  	$8000
		  26c	and
		  26d	bnt  	$9   	(.3)
		  26f	pushi	$8d  
		  272	push0
		  273	self
		  273	word	$4
		  276	jmp  	$1   	(.4)
.3
		  278	selfID
.4
		  279	send
		  279	word	$8
.5
		  27c	sat  	$0   	(theRobot)
		  281	lat  	$7   	(alaMode)
		  283	bt   	$2   	(.6)
		  285	lat  	$4   	(myObj)
.6
		  287	not
		  288	bnt  	$7   	(.7)
		  28d	lat  	$0   	(theRobot)
		  28f	sag  	$1c  	(autoRobot)
.7
		  297	pushi	$b1  
		  29a	push1
		  29b	lst  	$3   	(myCaller)
		  2a0	pushi	$7   
		  2a2	push1
		  2a3	lst  	$5   	(myPri)
		  2a8	pushi	$114 
		  2ab	push1
		  2ac	lst  	$6   	(myHold)
		  2b1	push1
		  2b2	push1
		  2b3	lst  	$1   	(myX)
		  2b8	push2
		  2b9	push1
		  2ba	lst  	$2   	(myY)
		  2bf	pushi	$27  
		  2c1	push1
		  2c2	lst  	$7   	(alaMode)
		  2c7	pushi	$a8  
		  2ca	push0
		  2cb	lat  	$0   	(theRobot)
		  2cd	send
		  2cd	word	$28
		  2d3	ret


Method: (WalkieTalkie killRobot)

		  2d7	file
		  2e4	link 	$2   
		  2ef	lsp  	$0   	(argc)
		  2f1	ldi  	$1   
		  2f3	gt?
		  2f4	bnt  	$4   	(.0)
		  2f6	lap  	$2   	(who)
		  2f8	jmp  	$10  	(.2)
.0
		  2fd	lag  	$1c  	(autoRobot)
		  2ff	bnt  	$4   	(.1)
		  301	ldi  	$0   
		  303	jmp  	$5   	(.2)
.1
		  308	lag  	$0   	(ego)
.2
		  30a	sat  	$0   	(myObj)
		  312	bnt  	$9   	(.3)
		  314	pushi	$112 
		  317	push0
		  318	send
		  318	word	$4
		  31b	jmp  	$2   	(.4)
.3
		  31d	lag  	$1c  	(autoRobot)
.4
		  31f	sat  	$1   	(theRobot)
		  324	bnt  	$47  	(.9)
		  329	pushi	$114 
		  32c	push1
		  32d	push0
		  32e	send
		  32e	word	$6
		  334	lap  	$0   	(argc)
		  336	bnt  	$2   	(.5)
		  338	lap  	$1   	(doICue)
.5
		  33a	not
		  33b	bnt  	$a   	(.6)
		  33d	pushi	$b1  
		  340	push1
		  341	push0
		  342	lat  	$1   	(theRobot)
		  344	send
		  344	word	$6
.6
		  34a	lat  	$0   	(myObj)
		  34c	bnt  	$d   	(.7)
		  351	pushi	$11d 
		  354	push1
		  355	push0
		  356	send
		  356	word	$6
		  359	jmp  	$1f  	(.10)
.7
		  361	pushi	$8f  
		  364	push1
		  365	push1
		  366	lat  	$1   	(theRobot)
		  368	send
		  368	word	$6
.8
		  36b	jmp  	$d   	(.10)
.9
		  373	push1
		  374	pushi	$7   
		  376	callk	$3a  	(Robot)
		  376	word	$2
.10
		  37d	ret


Method: (WalkieTalkie robotDone)

		  381	file
		  391	ldi  	$1   
		  393	aTop 	$28  	(completed)
		  398	pToa 	$2a  	(holdOnLastCel)
		  39a	not
		  39b	bnt  	$a   	(.0)
		  3a0	push1
		  3a1	pushi	$7   
		  3a3	callk	$3a  	(Robot)
		  3a3	word	$2
.0
		  3aa	pToa 	$2e  	(hideClient)
		  3ac	bnt  	$c   	(.2)
		  3ae	pToa 	$20  	(client)
.1
		  3b0	bnt  	$8   	(.2)
		  3b2	pushi	$75  
		  3b4	push0
		  3b5	pToa 	$20  	(client)
		  3b7	send
		  3b7	word	$4
.2
		  3bd	lap  	$0   	(argc)
		  3bf	bnt  	$10  	(.4)
		  3c1	lap  	$1   	(doDispose)
.3
		  3c3	bnt  	$c   	(.4)
		  3c8	pushi	$8f  
		  3cb	push0
		  3cc	self
		  3cc	word	$4
		  3cf	jmp  	$17  	(.6)
.4
		  3d7	pToa 	$24  	(caller)
		  3d9	bnt  	$d   	(.5)
		  3db	pushi	$b3  
		  3de	push1
		  3df	pushi	$ffff
		  3e1	send
		  3e1	word	$6
		  3e4	ldi  	$0   
		  3e6	aTop 	$24  	(caller)
.5
.6
		  3eb	ret


Method: (WalkieTalkie showFrame)

		  3ef	file
		  3fc	link 	$4   
		  407	lsp  	$0   	(argc)
		  409	ldi  	$2   
		  40b	gt?
		  40c	bnt  	$4   	(.0)
		  40e	lap  	$3   	(theX)
		  410	jmp  	$2   	(.1)
.0
		  412	ldi  	$0   
.1
		  414	sat  	$0   	(showX)
		  41c	lsp  	$0   	(argc)
		  41e	ldi  	$3   
		  420	gt?
		  421	bnt  	$4   	(.2)
		  423	lap  	$4   	(theY)
		  425	jmp  	$2   	(.3)
.2
		  427	ldi  	$0   
.3
		  429	sat  	$1   	(showY)
		  431	lsp  	$0   	(argc)
		  433	ldi  	$4   
		  435	gt?
		  436	bnt  	$4   	(.4)
		  438	lap  	$5   	(pri)
		  43a	jmp  	$2   	(.5)
.4
		  43c	ldi  	$ffff
.5
		  43e	sat  	$3   	(thatOldPri)
		  446	lsp  	$0   	(argc)
		  448	ldi  	$5   
		  44a	gt?
		  44b	bnt  	$4   	(.6)
		  44d	lap  	$6   	(whichPlane)
		  44f	jmp  	$2   	(.7)
.6
		  451	lag  	$3   	(thePlane)
.7
		  453	sat  	$2   	(thatOldPlane)
		  458	pushi	$6   
		  45a	push0
		  45b	lsp  	$1   	(theNum)
		  45d	push
		  45e	lst  	$3   	(thatOldPri)
		  460	lst  	$0   	(showX)
		  462	lst  	$1   	(showY)
		  464	callk	$3a  	(Robot)
		  464	word	$c
		  46b	push2
		  46c	push1
		  46d	lsp  	$2   	(theFrame)
		  46f	callk	$3a  	(Robot)
		  46f	word	$4
		  476	ret


Method: (WalkieTalkie start)

		  47a	file
		  48a	lap  	$0   	(argc)
		  48c	bnt  	$4   	(.0)
		  48e	lap  	$1   	(m)
		  490	aTop 	$1e  	(mode)
.0
		  495	pToa 	$2e  	(hideClient)
		  497	bnt  	$29  	(.4)
		  499	pToa 	$20  	(client)
.1
		  49b	bnt  	$25  	(.4)
		  4a0	pushi	$11e 
		  4a3	push0
		  4a4	pToa 	$20  	(client)
		  4a6	send
		  4a6	word	$4
		  4a9	bnt  	$d   	(.2)
		  4ae	pushi	$69  
		  4b0	push0
		  4b1	pToa 	$20  	(client)
		  4b3	send
		  4b3	word	$4
		  4b6	jmp  	$a   	(.3)
.2
		  4be	ldi  	$0   
		  4c0	aTop 	$2e  	(hideClient)
.3
.4
		  4c5	pushi	$6   
		  4c7	push0
		  4c8	pTos 	$14  	(robot)
		  4ca	pTos 	$1c  	(plane)
		  4cc	pTos 	$1a  	(priority)
		  4ce	pTos 	$16  	(x)
		  4d0	pTos 	$18  	(y)
		  4d2	callk	$3a  	(Robot)
		  4d2	word	$c
		  4d9	push1
		  4da	pushi	$4   
		  4dc	callk	$3a  	(Robot)
		  4dc	word	$2
		  4e3	ldi  	$1   
		  4e5	aTop 	$26  	(started)
		  4ea	pToa 	$1e  	(mode)
		  4ec	bnt  	$20  	(.7)
.5
		  4f1	push1
		  4f2	pushi	$5   
		  4f4	callk	$3a  	(Robot)
		  4f4	word	$2
		  4f8	not
		  4f9	bnt  	$7   	(.6)
		  4fb	push0
		  4fc	callk	$21  	(FrameOut)
		  4fc	word	$0
		  500	jmp  	$ffef	(.5)
.6
		  505	pushi	$119 
		  508	push1
		  509	pTos 	$2c  	(disposeWhenDone)
		  50b	self
		  50b	word	$6
.7
		  511	ret


Method: (WalkieTalkie terminate)

		  515	file
		  525	push1
		  526	pushi	$7   
		  528	callk	$3a  	(Robot)
		  528	word	$2
		  52f	ret


Fixups:
		  530	word	$0
