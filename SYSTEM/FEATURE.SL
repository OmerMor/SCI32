
				Listing of feature.sc:	[script 64950]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$bc


Variables:
		    2	word	$0



Object: CueObj              
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $18  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $38  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $2a  	(-script-)
		   10	prop  $7   	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $9a  	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(client)
		   1a	prop  $ffff	(state)
		   1c	prop  $0   	(start)
		   1e	prop  $0   	(timer)
		   20	prop  $0   	(cycles)
		   22	prop  $0   	(seconds)
		   24	prop  $0   	(lastSeconds)
		   26	prop  $0   	(ticks)
		   28	prop  $0   	(lastTicks)
		   2a	prop  $0   	(register)
		   2c	prop  $0   	(script)
		   2e	prop  $0   	(caller)
		   30	prop  $0   	(next)
		   32	prop  $0   	(theVerb)

Object: Feature             
		(properties)
		   34	prop  $1234	(-objID-)
		   36	prop  $1e  	(-size-)
		   38	ofs   $3e  	(-propDict-)
		   3a	ofs   $7a  	(-methDict-)
		   3c	prop  $0   	(-classScript-)
		   3e	prop  $2b  	(-script-)
		   40	prop  $0   	(-super-)
		   42	prop  $8000	(-info-)
		   44	text  $a1  	(name)
		   46	prop  $0   	(scratch)
		   48	prop  $0   	(heading)
		   4a	prop  $0   	(noun)
		   4c	prop  $0   	(case)
		   4e	prop  $ffff	(modNum)
		   50	prop  $0   	(nsLeft)
		   52	prop  $0   	(nsTop)
		   54	prop  $0   	(nsRight)
		   56	prop  $0   	(nsBottom)
		   58	prop  $6789	(sightAngle)
		   5a	prop  $0   	(actions)
		   5c	prop  $0   	(onMeCheck)
		   5e	prop  $0   	(state)
		   60	prop  $0   	(approachX)
		   62	prop  $0   	(approachY)
		   64	prop  $0   	(approachDist)
		   66	prop  $0   	(_approachVerbs)
		   68	prop  $0   	(plane)
		   6a	prop  $0   	(x)
		   6c	prop  $0   	(y)
		   6e	prop  $0   	(z)

Object: dftDoVerb           
		(properties)
		   70	prop  $1234	(-objID-)
		   72	prop  $a   	(-size-)
		   74	ofs   $b0  	(-propDict-)
		   76	ofs   $b0  	(-methDict-)
		   78	prop  $0   	(-classScript-)
		   7a	prop  $ffff	(-script-)
		   7c	prop  $1   	(-super-)
		   7e	prop  $0   	(-info-)
		   80	text  $a9  	(name)
		   82	prop  $0   	(scratch)

Object: Actions             
		(properties)
		   84	prop  $1234	(-objID-)
		   86	prop  $a   	(-size-)
		   88	ofs   $b6  	(-propDict-)
		   8a	ofs   $ca  	(-methDict-)
		   8c	prop  $0   	(-classScript-)
		   8e	prop  $2c  	(-script-)
		   90	prop  $1   	(-super-)
		   92	prop  $8000	(-info-)
		   94	text  $b3  	(name)
		   96	prop  $0   	(scratch)
		   98	word	$0



		   9a	text
"CueObj"

		   a1	text
"Feature"

		   a9	text
"dftDoVerb"

		   b3	text
"Actions"

		   bb	byte	$0


Fixups:
		   bc	word	$4
		   be	word	$14
		   c0	word	$44
		   c2	word	$80
		   c4	word	$94




----------------------
-------- Hunk --------
----------------------

		    0	word	$892
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: CueObj              
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
		   1c	word	$36
		   1e	word	$29
		   20	word	$a8
		   22	word	$a9
		   24	word	$aa
		   26	word	$ab
		   28	word	$ac
		   2a	word	$ad
		   2c	word	$ae
		   2e	word	$af
		   30	word	$b0
		   32	word	$b1
		   34	word	$4c
		   36	word	$131
		(method dictionary)
		   38	word	$1
		   3a	word	$b2
		   3c	local $d0  	(changeState)

Object: Feature             
		(property dictionary)
		   3e	word	$1000
		   40	word	$1001
		   42	word	$1002
		   44	word	$1003
		   46	word	$1004
		   48	word	$1005
		   4a	word	$1006
		   4c	word	$1007
		   4e	word	$20
		   50	word	$8c
		   52	word	$43
		   54	word	$124
		   56	word	$126
		   58	word	$123
		   5a	word	$12
		   5c	word	$13
		   5e	word	$14
		   60	word	$15
		   62	word	$134
		   64	word	$132
		   66	word	$135
		   68	word	$29
		   6a	word	$136
		   6c	word	$137
		   6e	word	$138
		   70	word	$139
		   72	word	$0
		   74	word	$1
		   76	word	$2
		   78	word	$3
		(method dictionary)
		   7a	word	$d
		   7c	word	$8e
		   7e	local $1a8 	(init)
		   80	word	$8f
		   82	local $7fa 	(dispose)
		   84	word	$13a
		   86	local $1fd 	(initialize)
		   88	word	$a6
		   8a	local $2b1 	(handleEvent)
		   8c	word	$133
		   8e	local $453 	(doVerb)
		   90	word	$13b
		   92	local $40c 	(notFacing)
		   94	word	$13c
		   96	local $4dd 	(facingMe)
		   98	word	$11e
		   9a	local $5e4 	(isNotHidden)
		   9c	word	$10e
		   9e	local $5fd 	(onMe)
		   a0	word	$13d
		   a2	local $23c 	(approachVerbs)
		   a4	word	$13e
		   a6	local $6d9 	(setName)
		   a8	word	$13f
		   aa	local $735 	(setPolygon)
		   ac	word	$140
		   ae	local $7ae 	(createPoly)

Object: dftDoVerb           
		(property dictionary)
		(method dictionary)
		   b0	word	$1
		   b2	word	$45
		   b4	local $85f 	(doit)

Object: Actions             
		(property dictionary)
		   b6	word	$1000
		   b8	word	$1001
		   ba	word	$1002
		   bc	word	$1003
		   be	word	$1004
		   c0	word	$1005
		   c2	word	$1006
		   c4	word	$1007
		   c6	word	$20
		   c8	word	$8c
		(method dictionary)
		   ca	word	$1
		   cc	word	$133
		   ce	local $878 	(doVerb)


Method: (CueObj changeState)

дцJ		   d3	file
дцJ		   e2	lap  	$1   	(newState)
		   e4	aTop 	$16  	(state)
		   e6	push
		   e7	dup
дцJ		   eb	ldi  	$1   
		   ed	eq?
		   ee	bnt  	$43  	(.0)
дцJ		   f3	pushi	$10a 
		   f6	push2
		   f7	pushi	$4   
		   f9	push1
		   fa	push0
		   fb	lag  	$0   	(ego)
		   fd	send
		   fd	word	$4
		  100	push
		  101	push2
		  102	push0
		  103	lag  	$0   	(ego)
		  105	send
		  105	word	$4
		  108	push
		  109	push1
		  10a	push0
		  10b	pToa 	$14  	(client)
		  10d	send
		  10d	word	$4
		  110	push
		  111	push2
		  112	push0
		  113	pToa 	$14  	(client)
		  115	send
		  115	word	$4
		  118	push
		  119	callk	$3f  	(GetAngle)
		  119	word	$8
		  11d	push
		  11e	pushSelf
		  11f	lag  	$0   	(ego)
		  121	send
		  121	word	$8
дцJ		  127	pushi	$96  
		  12a	push1
		  12b	pushSelf
		  12c	lag  	$4e  	(theDoits)
		  12e	send
		  12e	word	$6
		  131	jmp  	$70  	(.4)
.0
		  133	dup
дцJ		  137	ldi  	$2   
		  139	eq?
		  13a	bnt  	$9   	(.1)
дцJ		  13f	ldi  	$3   
		  141	aTop 	$1c  	(cycles)
		  143	jmp  	$5e  	(.4)
.1
		  145	dup
дцJ		  149	ldi  	$3   
		  14b	eq?
		  14c	bnt  	$55  	(.4)
дцJ		  151	pushi	$58  
		  153	push1
		  154	pushSelf
		  155	lag  	$4e  	(theDoits)
		  157	send
		  157	word	$6
дцJдцJдцJ		  163	pToa 	$14  	(client)
		  165	bnt  	$21  	(.2)
дцJ		  16a	pushi	$132 
		  16d	push0
		  16e	send
		  16e	word	$4
		  171	bnt  	$15  	(.2)
дцJ		  176	pushi	$133 
		  179	push1
		  17a	pTos 	$2e  	(theVerb)
		  17c	pushi	$132 
		  17f	push0
		  180	pToa 	$14  	(client)
		  182	send
		  182	word	$4
		  185	send
		  185	word	$6
.2
		  188	not
		  189	bnt  	$11  	(.3)
дцJдцJ		  191	pushi	$133 
		  194	push1
		  195	pTos 	$2e  	(theVerb)
		  197	pToa 	$14  	(client)
		  199	send
		  199	word	$6
.3
дцJ		  19f	ldi  	$0   
		  1a1	aTop 	$16  	(state)
.4
		  1a3	toss
дцJ		  1a7	ret


Method: (Feature init)

дцJ		  1ab	file
дцJ		  1ba	pushi	$13a 
		  1bd	push1
		  1be	lap  	$0   	(argc)
		  1c0	bnt  	$4   	(.0)
		  1c2	lap  	$1   	(theInitializer)
		  1c4	jmp  	$2   	(.1)
.0
		  1c6	ldi  	$0   
.1
		  1c8	push
		  1c9	self
		  1c9	word	$6
дцJ		  1cf	pushi	$93  
		  1d2	push1
		  1d3	pushi	$e   
		  1d5	self
		  1d5	word	$6
		  1d8	bnt  	$f   	(.2)
дцJ		  1dd	pushi	$96  
		  1e0	push1
		  1e1	pushSelf
		  1e2	lag  	$5   	(cast)
		  1e4	send
		  1e4	word	$6
		  1e7	jmp  	$10  	(.3)
.2
дцJдцJ		  1ef	pushi	$96  
		  1f2	push1
		  1f3	pushSelf
		  1f4	lag  	$20  	(features)
		  1f6	send
		  1f6	word	$6
.3
дцJ		  1fc	ret


Method: (Feature initialize)

дцJ		  200	file
дцJдцJ		  212	lap  	$0   	(argc)
		  214	bnt  	$11  	(.1)
		  216	lap  	$1   	(theInitializer)
.0
		  218	bnt  	$d   	(.1)
дцJ		  21d	pushi	$63  
		  21f	push1
		  220	lsp  	$1   	(theInitializer)
		  222	self
		  222	word	$6
		  225	jmp  	$11  	(.2)
.1
дцJ		  22a	lag  	$40  	(ftrInitializer)
		  22c	bnt  	$a   	(.2)
дцJ		  231	pushi	$63  
		  233	push1
		  234	push
		  235	self
		  235	word	$6
.2
дцJ		  23b	ret


Method: (Feature approachVerbs)

дцJ		  23f	file
		  24b	link 	$2   
дцJ		  250	ldi  	$0   
		  252	aTop 	$32  	(_approachVerbs)
дцJ		  257	lap  	$0   	(argc)
		  259	bnt  	$52  	(.3)
дцJ		  25e	lag  	$42  	(approachCode)
		  260	bnt  	$4b  	(.3)
дцJ		  265	ldi  	$0   
		  267	lapi 	$1   	()
.0
		  269	bnt  	$42  	(.3)
дцJдцJ		  271	ldi  	$0   
		  273	sat  	$0   	(i)
.1
дцJ		  278	lst  	$0   	(i)
		  27a	lap  	$0   	(argc)
		  27c	lt?
		  27d	bnt  	$2e  	(.2)
дцJдцJ		  285	pushi	$45  
		  287	push1
		  288	lat  	$0   	(i)
		  28a	lspi 	$1   	()
		  28c	lag  	$42  	(approachCode)
		  28e	send
		  28e	word	$6
		  291	sat  	$1   	(theBits)
дцJ		  296	pushi	$139 
		  299	push1
		  29a	pushi	$139 
		  29d	push0
		  29e	self
		  29e	word	$4
		  2a1	push
		  2a2	lat  	$1   	(theBits)
		  2a4	or
		  2a5	push
		  2a6	self
		  2a6	word	$6
		  2a9	+at  	$0   	(i)
		  2ab	jmp  	$ffc8	(.1)
.2
.3
дцJ		  2b0	ret


Method: (Feature handleEvent)

дцJ		  2b4	file
		  2c0	link 	$1   
дцJдцJ		  2c8	pushi	$50  
		  2ca	push0
		  2cb	lap  	$1   	(event)
		  2cd	send
		  2cd	word	$4
		  2d0	bnt  	$9   	(.0)
дцJ		  2d5	ldi  	$1   
		  2d7	ret
		  2d8	jmp  	$127 	(.7)
.0
дцJдцJ		  2e1	pushi	$2b  
		  2e3	push0
		  2e4	lap  	$1   	(event)
		  2e6	send
		  2e6	word	$4
		  2e9	push
		  2ea	ldi  	$4000
		  2ed	and
		  2ee	bnt  	$111 	(.7)
дцJ		  2f4	pushi	$10e 
		  2f7	push1
		  2f8	lsp  	$1   	(event)
		  2fa	self
		  2fa	word	$6
		  2fd	bnt  	$102 	(.7)
дцJ		  303	pushi	$11e 
		  306	push0
		  307	self
		  307	word	$4
.1
		  30a	bnt  	$f5  	(.7)
дцJдцJдцJ		  316	pushi	$29  
		  318	push1
		  319	push0
дцJ		  31d	pushi	$aa  
		  320	push1
		  321	push0
дцJ		  325	pushi	$36  
		  327	push1
		  328	pushSelf
дцJ		  32c	pushi	$131 
		  32f	push1
		  330	pushi	$31  
		  332	push0
		  333	lap  	$1   	(event)
		  335	send
		  335	word	$4
		  338	push
		  339	class	$2a  	(CueObj)
		  33b	send
		  33b	word	$18
дцJ		  341	pushi	$50  
		  343	push1
		  344	push1
		  345	lap  	$1   	(event)
		  347	send
		  347	word	$6
		  34a	sat  	$0   	(ret)
дцJ		  34f	pushi	$141 
		  352	push0
		  353	lag  	$50  	(user)
		  355	send
		  355	word	$4
		  358	bnt  	$7d  	(.4)
дцJ		  35e	pushi	$29  
		  360	push0
		  361	lag  	$0   	(ego)
		  363	send
		  363	word	$4
		  366	push
		  367	ldi  	$2   
		  369	and
		  36a	bnt  	$6b  	(.4)
дцJ		  370	pushi	$4   
		  372	push1
		  373	push0
		  374	lag  	$0   	(ego)
		  376	send
		  376	word	$4
		  379	push
		  37a	push2
		  37b	push0
		  37c	lag  	$0   	(ego)
		  37e	send
		  37e	word	$4
		  381	push
		  382	pTos 	$2c  	(approachX)
		  384	pTos 	$2e  	(approachY)
		  386	callk	$40  	(GetDistance)
		  386	word	$8
		  38a	push
		  38b	pToa 	$30  	(approachDist)
		  38d	gt?
		  38e	bnt  	$48  	(.4)
дцJ		  393	lag  	$42  	(approachCode)
		  395	bnt  	$41  	(.4)
дцJ		  39a	pTos 	$32  	(_approachVerbs)
		  39c	pushi	$45  
		  39e	push1
		  39f	pushi	$31  
		  3a1	push0
		  3a2	lap  	$1   	(event)
		  3a4	send
		  3a4	word	$4
		  3a7	push
		  3a8	lag  	$42  	(approachCode)
		  3aa	send
		  3aa	word	$6
		  3ad	and
.2
.3
		  3ae	bnt  	$28  	(.4)
дцJдцJ		  3b6	pushi	$142 
		  3b9	pushi	$4   
		  3bc	class	$23  	(PolyPath)
		  3be	push
		  3bf	pTos 	$2c  	(approachX)
		  3c1	pushi	$3   
		  3c3	push0
		  3c4	lag  	$0   	(ego)
		  3c6	send
		  3c6	word	$4
		  3c9	push
		  3ca	pToa 	$2e  	(approachY)
		  3cc	add
		  3cd	push
		  3ce	class	$2a  	(CueObj)
		  3d0	push
		  3d1	lag  	$0   	(ego)
		  3d3	send
		  3d3	word	$c
		  3d6	jmp  	$2a  	(.6)
.4
дцJдцJ		  3de	pushi	$142 
		  3e1	push1
		  3e2	push0
		  3e3	lag  	$0   	(ego)
		  3e5	send
		  3e5	word	$6
дцJ		  3eb	pushi	$13c 
		  3ee	push0
		  3ef	self
		  3ef	word	$4
		  3f2	bnt  	$e   	(.5)
дцJ		  3f7	pushi	$b2  
		  3fa	push1
		  3fb	pushi	$3   
		  3fd	class	$2a  	(CueObj)
		  3ff	send
		  3ff	word	$6
.5
.6
.7
дцJ		  405	lat  	$0   	(ret)
		  407	ret
дцJ		  40b	ret


Method: (Feature notFacing)

дцJ		  40f	file
		  41b	link 	$1   
дцJ		  420	pushi	$142 
		  423	push1
		  424	push0
		  425	lag  	$0   	(ego)
		  427	send
		  427	word	$6
дцJдцJ		  430	pushi	$36  
		  432	push1
		  433	pushSelf
дцJ		  437	pushi	$29  
		  439	push1
		  43a	push0
дцJ		  43e	pushi	$aa  
		  441	push1
		  442	push0
дцJ		  446	pushi	$b3  
		  449	push0
		  44a	class	$2a  	(CueObj)
		  44c	send
		  44c	word	$16
дцJ		  452	ret


Method: (Feature doVerb)

дцJ		  456	file
		  462	link 	$2   
дцJдцJ		  46a	lag  	$41  	(doVerbCode)
		  46c	bnt  	$5   	(.0)
дцJ		  471	jmp  	$9   	(.1)
.0
дцJдцJ		  479	lofsa	$70  	(dftDoVerb)
.1
		  47c	sat  	$0   	(defaultCode)
дцJ		  481	pTos 	$1a  	(modNum)
		  483	ldi  	$ffff
		  485	eq?
		  486	bnt  	$7   	(.2)
дцJ		  48b	lag  	$b   	(curRoomNum)
		  48d	aTop 	$1a  	(modNum)
.2
дцJ		  492	lag  	$5a  	(msgType)
		  494	bnt  	$32  	(.4)
дцJ		  499	pushi	$6   
		  49b	push0
		  49c	pTos 	$1a  	(modNum)
		  49e	pTos 	$16  	(noun)
		  4a0	lsp  	$1   	(theVerb)
		  4a2	pTos 	$18  	(case)
		  4a4	push1
		  4a5	callk	$48  	(Message)
		  4a5	word	$c
.3
		  4a9	bnt  	$1d  	(.4)
дцJдцJ		  4b1	pushi	$ee  
		  4b4	pushi	$6   
		  4b7	pTos 	$16  	(noun)
		  4b9	lsp  	$1   	(theVerb)
		  4bb	pTos 	$18  	(case)
		  4bd	push0
		  4be	push0
		  4bf	pTos 	$1a  	(modNum)
		  4c1	lag  	$5b  	(messager)
		  4c3	send
		  4c3	word	$10
		  4c6	jmp  	$11  	(.5)
.4
дцJдцJ		  4ce	pushi	$45  
		  4d0	push2
		  4d1	lsp  	$1   	(theVerb)
		  4d3	pushSelf
		  4d4	lat  	$0   	(defaultCode)
		  4d6	send
		  4d6	word	$8
.5
дцJ		  4dc	ret


Method: (Feature facingMe)

дцJ		  4e0	file
		  4ec	link 	$2   
дцJдцJ		  4f4	lap  	$0   	(argc)
		  4f6	bnt  	$9   	(.0)
дцJ		  4fb	lap  	$1   	(act)
		  4fd	sat  	$0   	(theActor)
		  4ff	jmp  	$22  	(.2)
.0
дцJ		  504	pushi	$9a  
		  507	push1
		  508	lsg  	$0   	(ego)
		  50a	lag  	$5   	(cast)
		  50c	send
		  50c	word	$6
		  50f	bnt  	$9   	(.1)
дцJ		  514	lag  	$0   	(ego)
		  516	sat  	$0   	(theActor)
		  518	jmp  	$9   	(.2)
.1
дцJдцJ		  520	ldi  	$1   
		  522	ret
.2
дцJдцJ		  529	push1
дцJдцJ		  530	pushi	$4   
		  532	push1
		  533	push0
		  534	lat  	$0   	(theActor)
		  536	send
		  536	word	$4
		  539	push
		  53a	push2
		  53b	push0
		  53c	lat  	$0   	(theActor)
		  53e	send
		  53e	word	$4
		  541	push
		  542	pTos 	$36  	(x)
		  544	pTos 	$38  	(y)
		  546	callk	$3f  	(GetAngle)
		  546	word	$8
		  54a	push
дцJ		  54e	pushi	$43  
		  550	push0
		  551	lat  	$0   	(theActor)
		  553	send
		  553	word	$4
		  556	sub
		  557	push
		  558	callk	$3d  	(Abs)
		  558	word	$2
		  55c	sat  	$1   	(theAngle)
дцJ		  561	push
		  562	ldi  	$b4  
		  565	gt?
		  566	bnt  	$b   	(.3)
дцJ		  56b	pushi	$168 
		  56e	lat  	$1   	(theAngle)
		  570	sub
		  571	sat  	$1   	(theAngle)
.3
дцJ		  576	lst  	$1   	(theAngle)
		  578	pToa 	$24  	(sightAngle)
		  57a	le?
		  57b	bnt  	$9   	(.4)
дцJ		  580	ldi  	$1   
		  582	ret
		  583	jmp  	$5a  	(.8)
.4
дцJдцJ		  58c	pTos 	$24  	(sightAngle)
		  58e	ldi  	$6789
		  591	ne?
		  592	bnt  	$46  	(.7)
дцJ		  597	pushi	$1a  
		  599	push0
		  59a	lat  	$0   	(theActor)
		  59c	send
		  59c	word	$4
		  59f	push
		  5a0	ldi  	$800 
		  5a3	and
		  5a4	bnt  	$27  	(.5)
дцJдцJ		  5ac	pushi	$36  
		  5ae	push1
		  5af	pushSelf
дцJ		  5b3	pushi	$29  
		  5b5	push1
		  5b6	push2
дцJ		  5ba	pushi	$aa  
		  5bd	push1
		  5be	push0
дцJ		  5c2	pushi	$b3  
		  5c5	push0
		  5c6	class	$2a  	(CueObj)
		  5c8	send
		  5c8	word	$16
		  5cb	jmp  	$d   	(.6)
.5
дцJдцJ		  5d3	pushi	$13b 
		  5d6	push0
		  5d7	self
		  5d7	word	$4
.6
.7
дцJ		  5dd	ldi  	$0   
		  5df	ret
.8
дцJ		  5e3	ret


Method: (Feature isNotHidden)

дцJ		  5e7	file
дцJ		  5f6	ldi  	$1   
		  5f8	ret
дцJ		  5fc	ret


Method: (Feature onMe)

дцJ		  600	file
		  60c	link 	$2   
дцJ		  611	lsp  	$0   	(argc)
		  613	ldi  	$1   
		  615	eq?
		  616	bnt  	$1a  	(.0)
дцJ		  61b	push1
		  61c	push0
		  61d	lap  	$1   	(theObjOrX)
		  61f	send
		  61f	word	$4
		  622	sat  	$0   	(oX)
дцJ		  627	push2
		  628	push0
		  629	lap  	$1   	(theObjOrX)
		  62b	send
		  62b	word	$4
		  62e	sat  	$1   	(oY)
		  630	jmp  	$11  	(.1)
.0
дцJдцJ		  638	lap  	$1   	(theObjOrX)
		  63a	sat  	$0   	(oX)
дцJ		  63f	lap  	$2   	(theY)
		  641	sat  	$1   	(oY)
.1
дцJдцJдцJ		  64c	pToa 	$28  	(onMeCheck)
		  64e	not
		  64f	bnt  	$48  	(.9)
дцJ		  654	pToa 	$1c  	(nsLeft)
		  656	bt   	$a   	(.2)
		  658	pToa 	$20  	(nsRight)
		  65a	bt   	$6   	(.2)
		  65c	pToa 	$1e  	(nsTop)
		  65e	bt   	$2   	(.2)
		  660	pToa 	$22  	(nsBottom)
.2
		  662	not
		  663	bt   	$1e  	(.6)
дцJ		  668	pTos 	$1c  	(nsLeft)
		  66a	lat  	$0   	(oX)
		  66c	le?
		  66d	bnt  	$20  	(.7)
		  66f	pprev
		  670	pToa 	$20  	(nsRight)
		  672	le?
.3
		  673	bnt  	$1a  	(.7)
дцJ		  678	pTos 	$1e  	(nsTop)
		  67a	lat  	$1   	(oY)
		  67c	le?
		  67d	bnt  	$10  	(.7)
		  67f	pprev
		  680	pToa 	$22  	(nsBottom)
		  682	le?
.4
.5
.6
		  683	bnt  	$a   	(.7)
дцJдцJ		  68b	ldi  	$1   
		  68d	jmp  	$45  	(.11)
.7
дцJдцJ		  695	ldi  	$0   
.8
		  697	jmp  	$3b  	(.11)
.9
дцJ		  69c	pushi	$91  
		  69f	push1
		  6a0	class	$3   	(List)
		  6a2	push
		  6a3	pToa 	$28  	(onMeCheck)
		  6a5	send
		  6a5	word	$6
		  6a8	bnt  	$17  	(.10)
дцJ		  6ad	pushi	$98  
		  6b0	pushi	$3   
		  6b3	pushi	$10e 
		  6b6	lst  	$0   	(oX)
		  6b8	lst  	$1   	(oY)
		  6ba	pToa 	$28  	(onMeCheck)
		  6bc	send
		  6bc	word	$a
		  6bf	jmp  	$13  	(.11)
.10
дцJдцJ		  6c7	pushi	$10e 
		  6ca	push2
		  6cb	lst  	$0   	(oX)
		  6cd	lst  	$1   	(oY)
		  6cf	pToa 	$28  	(onMeCheck)
		  6d1	send
		  6d1	word	$8
.11
		  6d4	ret
дцJ		  6d8	ret


Method: (Feature setName)

дцJ		  6dc	file
		  6e8	link 	$1   
дцJ		  6ed	lsp  	$1   	(theName)
		  6ef	ldi  	$2   
		  6f1	eq?
		  6f2	bnt  	$33  	(.0)
дцJ		  6f7	pushi	$8d  
		  6fa	push0
		  6fb	lap  	$2   	(strObj)
		  6fd	send
		  6fd	word	$4
		  700	sat  	$0   	(locStrObj)
дцJ		  705	pushi	$71  
		  707	push0
		  708	send
		  708	word	$4
		  70b	aTop 	$10  	(name)
дцJ		  710	pushi	$71  
		  712	push1
		  713	push0
		  714	lat  	$0   	(locStrObj)
		  716	send
		  716	word	$6
дцJ		  71c	pushi	$8f  
		  71f	push0
		  720	lat  	$0   	(locStrObj)
		  722	send
		  722	word	$4
		  725	jmp  	$a   	(.1)
.0
дцJдцJ		  72d	lap  	$1   	(theName)
		  72f	aTop 	$10  	(name)
.1
дцJ		  734	ret


Method: (Feature setPolygon)

дцJ		  738	file
дцJ		  747	pToa 	$28  	(onMeCheck)
		  749	bnt  	$11  	(.0)
дцJ		  74e	pushi	$8f  
		  751	push0
		  752	send
		  752	word	$4
дцJ		  758	ldi  	$0   
		  75a	aTop 	$28  	(onMeCheck)
.0
дцJдцJ		  762	lap  	$0   	(argc)
		  764	not
		  765	bt   	$5   	(.1)
		  767	lsp  	$1   	(args)
		  769	ldi  	$0   
		  76b	eq?
.1
		  76c	bnt  	$6   	(.2)
дцJ		  771	ret
		  772	jmp  	$36  	(.4)
.2
дцJ		  777	lsp  	$0   	(argc)
		  779	ldi  	$1   
		  77b	eq?
		  77c	bnt  	$b   	(.3)
дцJ		  781	ldi  	$0   
		  783	lapi 	$1   	()
		  785	aTop 	$28  	(onMeCheck)
		  787	jmp  	$21  	(.4)
.3
дцJдцJ		  78f	pushi	$8d  
		  792	push0
		  793	class	$3   	(List)
		  795	send
		  795	word	$4
		  798	aTop 	$28  	(onMeCheck)
дцJ		  79d	pushi	$96  
		  7a0	push1
		  7a1	lsp  	$1   	(args)
		  7a3	&rest	$2   
		  7a5	pToa 	$28  	(onMeCheck)
		  7a7	send
		  7a7	word	$6
.4
дцJ		  7ad	ret


Method: (Feature createPoly)

дцJ		  7b1	file
дцJ		  7c0	pToa 	$28  	(onMeCheck)
		  7c2	bnt  	$a   	(.0)
дцJ		  7c7	pushi	$8f  
		  7ca	push0
		  7cb	send
		  7cb	word	$4
.0
дцJ		  7d1	pushi	$8d  
		  7d4	push0
		  7d5	class	$22  	(Polygon)
		  7d7	send
		  7d7	word	$4
		  7da	aTop 	$28  	(onMeCheck)
дцJдцJ		  7e2	pushi	$8e  
		  7e5	push1
		  7e6	lsp  	$1   	(args)
		  7e8	&rest	$2   
дцJ		  7ed	pushi	$2b  
		  7ef	push1
		  7f0	push0
		  7f1	pToa 	$28  	(onMeCheck)
		  7f3	send
		  7f3	word	$c
дцJ		  7f9	ret


Method: (Feature dispose)

дцJ		  7fd	file
дцJ		  80c	pushi	$13f 
		  80f	push1
		  810	push0
		  811	self
		  811	word	$6
дцJ		  817	pToa 	$26  	(actions)
		  819	bnt  	$11  	(.0)
дцJ		  81e	pushi	$8f  
		  821	push0
		  822	send
		  822	word	$4
дцJ		  828	ldi  	$0   
		  82a	aTop 	$26  	(actions)
.0
дцJ		  82f	pToa 	$28  	(onMeCheck)
		  831	bnt  	$11  	(.1)
дцJ		  836	pushi	$8f  
		  839	push0
		  83a	send
		  83a	word	$4
дцJ		  840	ldi  	$0   
		  842	aTop 	$28  	(onMeCheck)
.1
дцJ		  847	pushi	$58  
		  849	push1
		  84a	pushSelf
		  84b	lag  	$20  	(features)
		  84d	send
		  84d	word	$6
дцJ		  853	pushi	$8f  
		  856	push0
		  857	super	$0   	(Object)
		  857	word	$4
дцJ		  85e	ret


Method: (dftDoVerb doit)

дцJ		  862	file
дцJ		  871	ldi  	$1   
		  873	ret
дцJ		  877	ret


Method: (Actions doVerb)

дцJ		  87b	file
дцJ		  88a	ldi  	$0   
		  88c	ret
дцJ		  890	ret
		  891	byte	$0


Fixups:
		  892	word	$1
		  894	word	$47a
