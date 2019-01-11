
				Listing of invent.sc:	[script 64995]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$1aa


Variables:
		    2	word	$0



Object: InvItem             
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $4e  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $a4  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $43  	(-script-)
		   10	prop  $40  	(-super-)
		   12	prop  $8004	(-info-)
		   14	text  $174 	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(heading)
		   1a	prop  $0   	(noun)
		   1c	prop  $0   	(case)
		   1e	prop  $ffff	(modNum)
		   20	prop  $0   	(nsLeft)
		   22	prop  $0   	(nsTop)
		   24	prop  $0   	(nsRight)
		   26	prop  $0   	(nsBottom)
		   28	prop  $6789	(sightAngle)
		   2a	prop  $0   	(actions)
		   2c	prop  $0   	(onMeCheck)
		   2e	prop  $0   	(state)
		   30	prop  $0   	(approachX)
		   32	prop  $0   	(approachY)
		   34	prop  $0   	(approachDist)
		   36	prop  $0   	(_approachVerbs)
		   38	prop  $0   	(plane)
		   3a	prop  $0   	(x)
		   3c	prop  $0   	(y)
		   3e	prop  $0   	(z)
		   40	prop  $80  	(scaleX)
		   42	prop  $80  	(scaleY)
		   44	prop  $80  	(maxScale)
		   46	prop  $0   	(scaleType)
		   48	prop  $0   	(priority)
		   4a	prop  $0   	(fixPriority)
		   4c	prop  $0   	(inLeft)
		   4e	prop  $0   	(inTop)
		   50	prop  $0   	(inRight)
		   52	prop  $0   	(inBottom)
		   54	prop  $0   	(useInsetRect)
		   56	prop  $0   	(view)
		   58	prop  $0   	(loop)
		   5a	prop  $0   	(cel)
		   5c	prop  $0   	(bitmap)
		   5e	prop  $2   	(yStep)
		   60	prop  $0   	(signal)
		   62	prop  $0   	(lsLeft)
		   64	prop  $0   	(lsTop)
		   66	prop  $0   	(lsRight)
		   68	prop  $0   	(lsBottom)
		   6a	prop  $0   	(brLeft)
		   6c	prop  $0   	(brTop)
		   6e	prop  $0   	(brRight)
		   70	prop  $0   	(brBottom)
		   72	prop  $0   	(scaleSignal)
		   74	prop  $0   	(magnifier)
		   76	prop  $80  	(oldScaleX)
		   78	prop  $4000	(type)
		   7a	prop  $0   	(message)
		   7c	prop  $0   	(modifiers)
		   7e	prop  $0   	(mainView)
		   80	prop  $0   	(mainLoop)
		   82	prop  $0   	(mainCel)
		   84	prop  $0   	(maskView)
		   86	prop  $0   	(maskLoop)
		   88	prop  $0   	(maskCel)
		   8a	prop  $ffff	(cursorView)
		   8c	prop  $ffff	(cursorLoop)
		   8e	prop  $ffff	(cursorCel)
		   90	prop  $0   	(highlightColor)
		   92	prop  $0   	(lowlightColor)
		   94	prop  $0   	(helpVerb)
		   96	prop  $0   	(object)
		   98	prop  $0   	(selector)
		   9a	prop  $0   	(owner)
		   9c	prop  $0   	(script)
		   9e	prop  $0   	(value)

Object: Inventory           
		(properties)
		   a0	prop  $1234	(-objID-)
		   a2	prop  $2e  	(-size-)
		   a4	ofs   $c2  	(-propDict-)
		   a6	ofs   $11e 	(-methDict-)
		   a8	prop  $0   	(-classScript-)
		   aa	prop  $44  	(-script-)
		   ac	prop  $41  	(-super-)
		   ae	prop  $8000	(-info-)
		   b0	text  $179 	(name)
		   b2	prop  $0   	(scratch)
		   b4	prop  $0   	(elements)
		   b6	prop  $0   	(size)
		   b8	prop  $0   	(nextNode)
		   ba	prop  $0   	(underBits)
		   bc	prop  $0   	(oldMouseX)
		   be	prop  $0   	(oldMouseY)
		   c0	prop  $0   	(curIcon)
		   c2	prop  $0   	(highlightedIcon)
		   c4	prop  $0   	(prevIcon)
		   c6	prop  $0   	(curInvIcon)
		   c8	prop  $0   	(useIconItem)
		   ca	prop  $0   	(helpIconItem)
		   cc	prop  $0   	(walkIconItem)
		   ce	prop  $0   	(plane)
		   d0	prop  $400 	(state)
		   d2	prop  $0   	(y)
		   d4	text  $17d 	(normalHeading)
		   d6	prop  $0   	(heading)
		   d8	text  $18f 	(empty)
		   da	prop  $0   	(okButton)
		   dc	prop  $0   	(selectIcon)
		   de	prop  $0   	(owner)
		   e0	prop  $0   	(iconBottom)
		   e2	prop  $0   	(iconRight)
		   e4	prop  $0   	(planeTop)
		   e6	prop  $0   	(planeBottom)
		   e8	prop  $0   	(planeLeft)
		   ea	prop  $0   	(planeRight)
		   ec	prop  $0   	(curIndex)
		   ee	prop  $0   	(numRow)
		   f0	prop  $0   	(numCol)
		   f2	prop  $0   	(rowMargin)
		   f4	prop  $0   	(colMargin)
		   f6	prop  $0   	(itemWide)
		   f8	prop  $0   	(itemHigh)
		   fa	prop  $0   	(numIcon)

Object: invCursor           
		(properties)
		   fc	prop  $1234	(-objID-)
		   fe	prop  $3b  	(-size-)
		  100	ofs   $154 	(-propDict-)
		  102	ofs   $154 	(-methDict-)
		  104	prop  $0   	(-classScript-)
		  106	prop  $ffff	(-script-)
		  108	prop  $42  	(-super-)
		  10a	prop  $0   	(-info-)
		  10c	text  $19f 	(name)
		  10e	prop  $0   	(scratch)
		  110	prop  $0   	(heading)
		  112	prop  $0   	(noun)
		  114	prop  $0   	(case)
		  116	prop  $ffff	(modNum)
		  118	prop  $0   	(nsLeft)
		  11a	prop  $0   	(nsTop)
		  11c	prop  $0   	(nsRight)
		  11e	prop  $0   	(nsBottom)
		  120	prop  $6789	(sightAngle)
		  122	prop  $0   	(actions)
		  124	prop  $0   	(onMeCheck)
		  126	prop  $0   	(state)
		  128	prop  $0   	(approachX)
		  12a	prop  $0   	(approachY)
		  12c	prop  $0   	(approachDist)
		  12e	prop  $0   	(_approachVerbs)
		  130	prop  $0   	(plane)
		  132	prop  $0   	(x)
		  134	prop  $0   	(y)
		  136	prop  $0   	(z)
		  138	prop  $80  	(scaleX)
		  13a	prop  $80  	(scaleY)
		  13c	prop  $80  	(maxScale)
		  13e	prop  $0   	(scaleType)
		  140	prop  $0   	(priority)
		  142	prop  $0   	(fixPriority)
		  144	prop  $0   	(inLeft)
		  146	prop  $0   	(inTop)
		  148	prop  $0   	(inRight)
		  14a	prop  $0   	(inBottom)
		  14c	prop  $0   	(useInsetRect)
		  14e	prop  $0   	(view)
		  150	prop  $0   	(loop)
		  152	prop  $0   	(cel)
		  154	prop  $0   	(bitmap)
		  156	prop  $2   	(yStep)
		  158	prop  $5021	(signal)
		  15a	prop  $0   	(lsLeft)
		  15c	prop  $0   	(lsTop)
		  15e	prop  $0   	(lsRight)
		  160	prop  $0   	(lsBottom)
		  162	prop  $0   	(brLeft)
		  164	prop  $0   	(brTop)
		  166	prop  $0   	(brRight)
		  168	prop  $0   	(brBottom)
		  16a	prop  $0   	(scaleSignal)
		  16c	prop  $0   	(magnifier)
		  16e	prop  $80  	(oldScaleX)
		  170	prop  $0   	(hidden)
		  172	word	$0



		  174	text
"InvI"

		  179	text
"Inv"

		  17d	text
"You are carrying:"

		  18f	text
"nothing!"

		  198	text
""

		  199	text
"%s %s"

		  19f	text
"invCursor"

		  1a9	byte	$0


Fixups:
		  1aa	word	$5
		  1ac	word	$14
		  1ae	word	$b0
		  1b0	word	$d4
		  1b2	word	$d8
		  1b4	word	$10c




----------------------
-------- Hunk --------
----------------------

		    0	word	$1512
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: InvItem             
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
		   1c	word	$43
		   1e	word	$124
		   20	word	$126
		   22	word	$123
		   24	word	$12
		   26	word	$13
		   28	word	$14
		   2a	word	$15
		   2c	word	$134
		   2e	word	$132
		   30	word	$135
		   32	word	$29
		   34	word	$136
		   36	word	$137
		   38	word	$138
		   3a	word	$139
		   3c	word	$0
		   3e	word	$1
		   40	word	$2
		   42	word	$3
		   44	word	$4
		   46	word	$5
		   48	word	$6
		   4a	word	$14a
		   4c	word	$7
		   4e	word	$8
		   50	word	$9
		   52	word	$a
		   54	word	$b
		   56	word	$c
		   58	word	$d
		   5a	word	$e
		   5c	word	$f
		   5e	word	$10
		   60	word	$11
		   62	word	$40
		   64	word	$1a
		   66	word	$16
		   68	word	$17
		   6a	word	$18
		   6c	word	$19
		   6e	word	$1c
		   70	word	$1d
		   72	word	$1e
		   74	word	$1f
		   76	word	$6a
		   78	word	$81
		   7a	word	$14b
		   7c	word	$2b
		   7e	word	$31
		   80	word	$48
		   82	word	$1b8
		   84	word	$1b9
		   86	word	$1ba
		   88	word	$1bb
		   8a	word	$1bc
		   8c	word	$1bd
		   8e	word	$1be
		   90	word	$1bf
		   92	word	$1c0
		   94	word	$1c1
		   96	word	$1c2
		   98	word	$1c3
		   9a	word	$172
		   9c	word	$173
		   9e	word	$f0
		   a0	word	$b0
		   a2	word	$51
		(method dictionary)
		   a4	word	$7
		   a6	word	$8e
		   a8	local $156 	(init)
		   aa	word	$133
		   ac	local $1c2 	(doVerb)
		   ae	word	$10e
		   b0	local $191 	(onMe)
		   b2	word	$1c5
		   b4	local $294 	(highlight)
		   b6	word	$1b7
		   b8	local $321 	(getCursor)
		   ba	word	$1dc
		   bc	local $2bc 	(ownedBy)
		   be	word	$c3
		   c0	local $2d7 	(moveTo)

Object: Inventory           
		(property dictionary)
		   c2	word	$1000
		   c4	word	$1001
		   c6	word	$1002
		   c8	word	$1003
		   ca	word	$1004
		   cc	word	$1005
		   ce	word	$1006
		   d0	word	$1007
		   d2	word	$20
		   d4	word	$8c
		   d6	word	$24
		   d8	word	$5a
		   da	word	$95
		   dc	word	$1c7
		   de	word	$1c8
		   e0	word	$1c9
		   e2	word	$1ca
		   e4	word	$1cb
		   e6	word	$1cc
		   e8	word	$1cd
		   ea	word	$1ce
		   ec	word	$1c6
		   ee	word	$1cf
		   f0	word	$0
		   f2	word	$29
		   f4	word	$2
		   f6	word	$1e0
		   f8	word	$43
		   fa	word	$1e1
		   fc	word	$1e2
		   fe	word	$1e3
		  100	word	$f0
		  102	word	$1e4
		  104	word	$1e5
		  106	word	$1e6
		  108	word	$1e7
		  10a	word	$1e8
		  10c	word	$1e9
		  10e	word	$1ea
		  110	word	$1eb
		  112	word	$1ec
		  114	word	$1ed
		  116	word	$1ee
		  118	word	$1ef
		  11a	word	$1f0
		  11c	word	$1f1
		(method dictionary)
		  11e	word	$d
		  120	word	$8e
		  122	local $363 	(init)
		  124	word	$45
		  126	local $d3a 	(doit)
		  128	word	$75
		  12a	local $951 	(show)
		  12c	word	$69
		  12e	local $9b0 	(hide)
		  130	word	$19d
		  132	local $b39 	(advance)
		  134	word	$19e
		  136	local $bed 	(retreat)
		  138	word	$1b3
		  13a	local $8ab 	(showSelf)
		  13c	word	$1dc
		  13e	local $82e 	(ownedBy)
		  140	word	$1f2
		  142	local $3f4 	(drawIcons)
		  144	word	$1f3
		  146	local $532 	(drawInvItems)
		  148	word	$1f4
		  14a	local $13c1	(carryingNothing)
		  14c	word	$1f5
		  14e	local $13f9	(setCurIndex)
		  150	word	$1f6
		  152	local $14d0	(setCurIndexTo)

Object: invCursor           
		(property dictionary)
		(method dictionary)
		  154	word	$0


Method: (InvItem init)

		  159	file
		  167	pushi	$8e  
		  16a	push0
		  16b	&rest	$1   
		  16d	super	$40  	(IconItem)
		  16d	word	$4
		  174	pTos 	$86  	(cursorView)
		  176	ldi  	$ffff
		  178	eq?
		  179	bnt  	$12  	(.0)
		  17e	pushi	$19f 
		  181	pushi	$3   
		  184	pTos 	$7a  	(mainView)
		  186	pTos 	$7c  	(mainLoop)
		  188	pTos 	$7e  	(mainCel)
		  18a	self
		  18a	word	$a
.0
		  190	ret


Method: (InvItem onMe)

		  194	file
		  1a8	pTos 	$5c  	(signal)
		  1aa	ldi  	$4   
		  1ac	and
		  1ad	not
		  1ae	bnt  	$d   	(.0)
		  1b3	pushi	$10e 
		  1b6	push1
		  1b7	lsp  	$1   	(event)
		  1b9	super	$40  	(IconItem)
		  1b9	word	$6
.0
		  1bd	ret
		  1c1	ret


Method: (InvItem doVerb)

		  1c5	file
		  1d0	link 	$1   
		  1d5	pTos 	$1a  	(modNum)
		  1d7	ldi  	$ffff
		  1d9	eq?
		  1da	bnt  	$7   	(.0)
		  1df	lag  	$b   	(curRoomNum)
		  1e1	aTop 	$1a  	(modNum)
.0
		  1e6	lag  	$5a  	(msgType)
		  1e8	bnt  	$2e  	(.2)
		  1ed	pushi	$6   
		  1ef	push0
		  1f0	pTos 	$1a  	(modNum)
		  1f2	pTos 	$16  	(noun)
		  1f4	lsp  	$1   	(theVerb)
		  1f6	push0
		  1f7	push1
		  1f8	callk	$48  	(Message)
		  1f8	word	$c
.1
		  1fc	bnt  	$1a  	(.2)
		  204	pushi	$ee  
		  207	pushi	$6   
		  20a	pTos 	$16  	(noun)
		  20c	lsp  	$1   	(theVerb)
		  20e	push0
		  20f	push0
		  210	push0
		  211	pTos 	$1a  	(modNum)
		  213	lag  	$5b  	(messager)
		  215	send
		  215	word	$10
.2
		  21b	pushi	$b0  
		  21e	push0
		  21f	lag  	$1   	(theGame)
		  221	send
		  221	word	$4
		  224	sat  	$0   	(tut)
		  226	bnt  	$68  	(.7)
		  22b	pushi	$91  
		  22e	push1
		  22f	class	$12  	(Tutorial)
		  231	push
		  232	lat  	$0   	(tut)
		  234	send
		  234	word	$6
.3
		  237	bnt  	$57  	(.7)
		  242	pushi	$e9  
		  245	push0
		  246	lat  	$0   	(tut)
		  248	send
		  248	word	$4
		  24b	push
		  24c	selfID
		  24d	ne?
		  24e	bnt  	$f   	(.4)
		  253	pushi	$ed  
		  256	push1
		  257	pushSelf
		  258	lat  	$0   	(tut)
		  25a	send
		  25a	word	$6
		  25d	jmp  	$31  	(.6)
.4
		  262	pushi	$ea  
		  265	push0
		  266	lat  	$0   	(tut)
		  268	send
		  268	word	$4
		  26b	push
		  26c	lap  	$1   	(theVerb)
		  26e	ne?
		  26f	bnt  	$10  	(.5)
		  274	pushi	$ed  
		  277	push1
		  278	lsp  	$1   	(theVerb)
		  27a	lat  	$0   	(tut)
		  27c	send
		  27c	word	$6
		  27f	jmp  	$f   	(.6)
.5
		  287	pushi	$b3  
		  28a	push0
		  28b	lat  	$0   	(tut)
		  28d	send
		  28d	word	$4
.6
.7
		  293	ret


Method: (InvItem highlight)

		  297	file
		  2a5	push2
		  2a6	lap  	$0   	(argc)
		  2a8	bnt  	$2   	(.0)
		  2aa	lap  	$1   	(tOrF)
.0
		  2ac	mul
		  2ad	aTop 	$56  	(cel)
		  2b2	push1
		  2b3	pushSelf
		  2b4	callk	$20  	(UpdateScreenItem)
		  2b4	word	$2
		  2bb	ret


Method: (InvItem ownedBy)

		  2bf	file
		  2cd	pTos 	$96  	(owner)
		  2cf	lap  	$1   	(who)
		  2d1	eq?
		  2d2	ret
		  2d6	ret


Method: (InvItem moveTo)

		  2da	file
		  2e8	lap  	$1   	(who)
		  2ea	aTop 	$96  	(owner)
		  2f2	pToa 	$9a  	(value)
		  2f4	bnt  	$22  	(.1)
		  2f9	lsp  	$1   	(who)
		  2fb	lag  	$0   	(ego)
		  2fd	eq?
.0
		  2fe	bnt  	$18  	(.1)
		  306	pushi	$1df 
		  309	push1
		  30a	pTos 	$9a  	(value)
		  30c	lag  	$1   	(theGame)
		  30e	send
		  30e	word	$6
		  314	ldi  	$0   
		  316	aTop 	$9a  	(value)
.1
		  31b	selfID
		  31c	ret
		  320	ret


Method: (InvItem getCursor)

		  324	file
		  335	pTos 	$88  	(cursorLoop)
		  337	ldi  	$ffff
		  339	eq?
		  33a	bnt  	$7   	(.0)
		  33f	pToa 	$86  	(cursorView)
		  341	jmp  	$1b  	(.1)
.0
		  34c	pushi	$13a 
		  34f	push1
		  350	pushSelf
		  354	pushi	$94  
		  357	push0
		  358	lofsa	$fc  	(invCursor)
		  35b	send
		  35b	word	$a
.1
		  35e	ret
		  362	ret


Method: (Inventory init)

		  366	file
		  371	link 	$2   
		  376	pushi	$8e  
		  379	push0
		  37a	super	$41  	(IconBar)
		  37a	word	$4
		  381	pushi	$9c  
		  384	push0
		  385	self
		  385	word	$4
		  388	sat  	$0   	(node)
.0
		  38d	lat  	$0   	(node)
		  38f	bnt  	$55  	(.3)
		  397	pushi	$91  
		  39a	push1
		  39b	class	$43  	(InvItem)
		  39d	push
		  39e	push2
		  39f	pushi	$8   
		  3a1	lst  	$0   	(node)
		  3a3	callk	$5a  	(KList)
		  3a3	word	$4
		  3a7	sat  	$1   	(obj)
		  3a9	send
		  3a9	word	$6
		  3ac	bnt  	$24  	(.1)
		  3b1	push1
		  3b2	lst  	$1   	(obj)
		  3b4	callk	$1f  	(DeleteScreenItem)
		  3b4	word	$2
		  3bb	pushi	$1a  
		  3bd	push1
		  3be	pushi	$1a  
		  3c0	push0
		  3c1	lat  	$1   	(obj)
		  3c3	send
		  3c3	word	$4
		  3c6	push
		  3c7	ldi  	$4   
		  3c9	or
		  3ca	push
		  3cb	lat  	$1   	(obj)
		  3cd	send
		  3cd	word	$6
		  3d0	jmp  	$8   	(.2)
.1
		  3d8	ipToa	$5a  	(numIcon)
.2
		  3da	pushi	$4c  
		  3dc	push1
		  3dd	lst  	$0   	(node)
		  3df	self
		  3df	word	$6
		  3e2	sat  	$0   	(node)
		  3e4	jmp  	$ffa4	(.0)
.3
		  3e9	pushi	$1f2 
		  3ec	push0
		  3ed	self
		  3ed	word	$4
		  3f3	ret


Method: (Inventory drawIcons)

		  3f7	file
		  402	link 	$5   
		  407	ldi  	$0   
		  409	sat  	$4   	(cWide)
		  40b	sat  	$3   	(cHigh)
		  40d	sat  	$0   	(obj)
		  40f	sat  	$2   	(node)
		  411	sat  	$1   	(atX)
		  416	pushi	$9c  
		  419	push0
		  41a	self
		  41a	word	$4
		  41d	sat  	$2   	(node)
.0
		  422	lat  	$2   	(node)
		  424	bnt  	$f2  	(.4)
		  42d	pushi	$91  
		  430	push1
		  431	class	$43  	(InvItem)
		  433	push
		  434	push2
		  435	pushi	$8   
		  437	lst  	$2   	(node)
		  439	callk	$5a  	(KList)
		  439	word	$4
		  43d	sat  	$0   	(obj)
		  43f	send
		  43f	word	$6
		  442	not
		  443	bnt  	$c6  	(.3)
		  449	pushi	$3   
		  44b	pushi	$e   
		  44d	push0
		  44e	lat  	$0   	(obj)
		  450	send
		  450	word	$4
		  453	push
		  454	pushi	$f   
		  456	push0
		  457	lat  	$0   	(obj)
		  459	send
		  459	word	$4
		  45c	push
		  45d	pushi	$10  
		  45f	push0
		  460	lat  	$0   	(obj)
		  462	send
		  462	word	$4
		  465	push
		  466	callk	$1c  	(CelWide)
		  466	word	$6
		  46a	sat  	$4   	(cWide)
		  46f	pTos 	$42  	(iconRight)
		  471	add
		  472	aTop 	$42  	(iconRight)
		  477	pushi	$3   
		  479	pushi	$e   
		  47b	push0
		  47c	lat  	$0   	(obj)
		  47e	send
		  47e	word	$4
		  481	push
		  482	pushi	$f   
		  484	push0
		  485	lat  	$0   	(obj)
		  487	send
		  487	word	$4
		  48a	push
		  48b	pushi	$10  
		  48d	push0
		  48e	lat  	$0   	(obj)
		  490	send
		  490	word	$4
		  493	push
		  494	callk	$1d  	(CelHigh)
		  494	word	$6
		  498	sat  	$3   	(cHigh)
		  49a	push
		  49b	pToa 	$40  	(iconBottom)
		  49d	gt?
		  49e	bnt  	$7   	(.1)
		  4a3	lat  	$3   	(cHigh)
		  4a5	aTop 	$40  	(iconBottom)
.1
		  4aa	pushi	$1a  
		  4ac	push0
		  4ad	lat  	$0   	(obj)
		  4af	send
		  4af	word	$4
		  4b2	push
		  4b3	ldi  	$80  
		  4b6	and
		  4b7	not
		  4b8	bnt  	$38  	(.2)
		  4c0	push1
		  4c1	push1
		  4c2	lst  	$1   	(atX)
		  4c7	push2
		  4c8	push1
		  4c9	push0
		  4cd	pushi	$12  
		  4cf	push1
		  4d0	lst  	$1   	(atX)
		  4d5	pushi	$13  
		  4d7	push1
		  4d8	push0
		  4dc	pushi	$14  
		  4de	push1
		  4df	lst  	$1   	(atX)
		  4e1	lat  	$4   	(cWide)
		  4e3	add
		  4e4	push
		  4e8	pushi	$15  
		  4ea	push1
		  4eb	lst  	$3   	(cHigh)
		  4ed	lat  	$0   	(obj)
		  4ef	send
		  4ef	word	$24
.2
		  4f5	push1
		  4f6	push0
		  4f7	lat  	$0   	(obj)
		  4f9	send
		  4f9	word	$4
		  4fc	push
		  4fd	lat  	$4   	(cWide)
		  4ff	add
		  500	sat  	$1   	(atX)
		  505	push1
		  506	lst  	$0   	(obj)
		  508	callk	$20  	(UpdateScreenItem)
		  508	word	$2
.3
		  50c	pushi	$4c  
		  50e	push1
		  50f	lst  	$2   	(node)
		  511	self
		  511	word	$6
		  514	sat  	$2   	(node)
		  516	jmp  	$ff06	(.0)
.4
		  51c	pToa 	$50  	(numCol)
		  51e	bnt  	$e   	(.5)
		  523	pTos 	$56  	(itemWide)
		  525	pToa 	$54  	(colMargin)
		  527	add
		  528	push
		  529	pToa 	$50  	(numCol)
		  52b	mul
		  52c	aTop 	$42  	(iconRight)
.5
		  531	ret


Method: (Inventory drawInvItems)

		  535	file
		  540	link 	$b   
		  545	ldi  	$0   
		  547	sat  	$4   	(itemBottom)
		  54c	ldi  	$a   
		  54e	sat  	$0   	(atX)
		  553	pTos 	$40  	(iconBottom)
		  555	ldi  	$f   
		  557	add
		  558	sat  	$1   	(atY)
		  55d	ldi  	$0   
		  55f	sat  	$a   	(curRow)
		  564	pushi	$9c  
		  567	push0
		  568	self
		  568	word	$4
		  56b	sat  	$3   	(node)
		  56d	ldi  	$0   
		  56f	sat  	$9   	(si)
.0
		  571	lst  	$9   	(si)
		  573	pToa 	$4c  	(curIndex)
		  575	lt?
		  576	bnt  	$5f  	(.3)
		  57b	pushi	$91  
		  57e	push1
		  57f	class	$43  	(InvItem)
		  581	push
		  582	push2
		  583	pushi	$8   
		  585	lst  	$3   	(node)
		  587	callk	$5a  	(KList)
		  587	word	$4
		  58b	sat  	$2   	(obj)
		  58d	send
		  58d	word	$6
		  590	bnt  	$37  	(.2)
		  595	pushi	$1a  
		  597	push0
		  598	lat  	$2   	(obj)
		  59a	send
		  59a	word	$4
		  59d	push
		  59e	ldi  	$4   
		  5a0	and
		  5a1	not
.1
		  5a2	bnt  	$25  	(.2)
		  5aa	push1
		  5ab	lst  	$2   	(obj)
		  5ad	callk	$1f  	(DeleteScreenItem)
		  5ad	word	$2
		  5b4	pushi	$1a  
		  5b6	push1
		  5b7	pushi	$1a  
		  5b9	push0
		  5ba	lat  	$2   	(obj)
		  5bc	send
		  5bc	word	$4
		  5bf	push
		  5c0	ldi  	$4   
		  5c2	or
		  5c3	push
		  5c4	lat  	$2   	(obj)
		  5c6	send
		  5c6	word	$6
.2
		  5c9	+at  	$9   	(si)
		  5cb	pushi	$4c  
		  5cd	push1
		  5ce	lst  	$3   	(node)
		  5d0	self
		  5d0	word	$6
		  5d3	sat  	$3   	(node)
		  5d5	jmp  	$ff9a	(.0)
.3
		  5da	lat  	$3   	(node)
		  5dc	sat  	$3   	(node)
.4
		  5de	lat  	$3   	(node)
		  5e0	bnt  	$177 	(.12)
		  5e9	pushi	$91  
		  5ec	push1
		  5ed	class	$43  	(InvItem)
		  5ef	push
		  5f0	push2
		  5f1	pushi	$8   
		  5f3	lst  	$3   	(node)
		  5f5	callk	$5a  	(KList)
		  5f5	word	$4
		  5f9	sat  	$2   	(obj)
		  5fb	send
		  5fb	word	$6
		  5fe	bnt  	$14c 	(.11)
		  604	pushi	$f0  
		  607	push0
		  608	lat  	$2   	(obj)
		  60a	send
		  60a	word	$4
		  60d	push
		  60e	pToa 	$3e  	(owner)
		  610	eq?
.5
		  611	bnt  	$139 	(.11)
		  61a	pushi	$3   
		  61c	pushi	$e   
		  61e	push0
		  61f	lat  	$2   	(obj)
		  621	send
		  621	word	$4
		  624	push
		  625	pushi	$f   
		  627	push0
		  628	lat  	$2   	(obj)
		  62a	send
		  62a	word	$4
		  62d	push
		  62e	pushi	$10  
		  630	push0
		  631	lat  	$2   	(obj)
		  633	send
		  633	word	$4
		  636	push
		  637	callk	$1d  	(CelHigh)
		  637	word	$6
		  63b	sat  	$5   	(cHigh)
		  63d	push
		  63e	lat  	$4   	(itemBottom)
		  640	gt?
		  641	bnt  	$7   	(.6)
		  646	lat  	$5   	(cHigh)
		  648	sat  	$4   	(itemBottom)
.6
		  650	push1
		  651	push1
		  652	lst  	$0   	(atX)
		  657	push2
		  658	push1
		  659	lst  	$1   	(atY)
		  65e	pushi	$13  
		  660	push1
		  661	lst  	$1   	(atY)
		  666	pushi	$12  
		  668	push1
		  669	lst  	$0   	(atX)
		  66e	pushi	$14  
		  670	push1
		  671	lst  	$0   	(atX)
		  673	pushi	$3   
		  675	pushi	$e   
		  677	push0
		  678	lat  	$2   	(obj)
		  67a	send
		  67a	word	$4
		  67d	push
		  67e	pushi	$f   
		  680	push0
		  681	lat  	$2   	(obj)
		  683	send
		  683	word	$4
		  686	push
		  687	pushi	$10  
		  689	push0
		  68a	lat  	$2   	(obj)
		  68c	send
		  68c	word	$4
		  68f	push
		  690	callk	$1c  	(CelWide)
		  690	word	$6
		  694	add
		  695	push
		  699	pushi	$15  
		  69b	push1
		  69c	lst  	$1   	(atY)
		  69e	lat  	$5   	(cHigh)
		  6a0	add
		  6a1	push
		  6a2	lat  	$2   	(obj)
		  6a4	send
		  6a4	word	$24
		  6aa	pushi	$1a  
		  6ac	push0
		  6ad	lat  	$2   	(obj)
		  6af	send
		  6af	word	$4
		  6b2	push
		  6b3	ldi  	$4   
		  6b5	and
		  6b6	bnt  	$24  	(.7)
		  6bb	pushi	$1a  
		  6bd	push1
		  6be	pushi	$1a  
		  6c0	push0
		  6c1	lat  	$2   	(obj)
		  6c3	send
		  6c3	word	$4
		  6c6	push
		  6c7	ldi  	$fffb
		  6c9	and
		  6ca	push
		  6cb	lat  	$2   	(obj)
		  6cd	send
		  6cd	word	$6
		  6d3	push1
		  6d4	lst  	$2   	(obj)
		  6d6	callk	$1e  	(AddScreenItem)
		  6d6	word	$2
		  6da	jmp  	$d   	(.8)
.7
		  6e2	push1
		  6e3	lst  	$2   	(obj)
		  6e5	callk	$20  	(UpdateScreenItem)
		  6e5	word	$2
.8
		  6f2	lst  	$0   	(atX)
		  6f4	pushi	$3   
		  6f6	pushi	$e   
		  6f8	push0
		  6f9	lat  	$2   	(obj)
		  6fb	send
		  6fb	word	$4
		  6fe	push
		  6ff	pushi	$f   
		  701	push0
		  702	lat  	$2   	(obj)
		  704	send
		  704	word	$4
		  707	push
		  708	pushi	$10  
		  70a	push0
		  70b	lat  	$2   	(obj)
		  70d	send
		  70d	word	$4
		  710	push
		  711	callk	$1c  	(CelWide)
		  711	word	$6
		  715	push
		  716	pToa 	$54  	(colMargin)
		  718	add
		  719	add
		  71a	sat  	$0   	(atX)
		  71c	push
		  720	pToa 	$42  	(iconRight)
		  722	gt?
		  723	bnt  	$28  	(.10)
		  72b	ldi  	$a   
		  72d	sat  	$0   	(atX)
		  732	lst  	$1   	(atY)
		  734	pToa 	$58  	(itemHigh)
		  736	add
		  737	push
		  738	pToa 	$52  	(rowMargin)
		  73a	add
		  73b	sat  	$1   	(atY)
		  740	+at  	$a   	(curRow)
		  742	push
		  743	pToa 	$4e  	(numRow)
		  745	ge?
		  746	bnt  	$5   	(.9)
		  74b	jmp  	$d   	(.12)
.9
.10
.11
		  74d	pushi	$4c  
		  74f	push1
		  750	lst  	$3   	(node)
		  752	self
		  752	word	$6
		  755	sat  	$3   	(node)
		  757	jmp  	$fe84	(.4)
.12
		  75d	lat  	$3   	(node)
		  75f	bnt  	$71  	(.17)
		  765	pushi	$4c  
		  767	push1
		  768	push
		  769	self
		  769	word	$6
		  76c	sat  	$3   	(node)
.13
		  76e	lat  	$3   	(node)
		  770	bnt  	$60  	(.16)
		  779	pushi	$91  
		  77c	push1
		  77d	class	$43  	(InvItem)
		  77f	push
		  780	push2
		  781	pushi	$8   
		  783	lst  	$3   	(node)
		  785	callk	$5a  	(KList)
		  785	word	$4
		  789	sat  	$2   	(obj)
		  78b	send
		  78b	word	$6
		  78e	bnt  	$37  	(.15)
		  793	pushi	$1a  
		  795	push0
		  796	lat  	$2   	(obj)
		  798	send
		  798	word	$4
		  79b	push
		  79c	ldi  	$4   
		  79e	and
		  79f	not
.14
		  7a0	bnt  	$25  	(.15)
		  7a8	push1
		  7a9	lst  	$2   	(obj)
		  7ab	callk	$1f  	(DeleteScreenItem)
		  7ab	word	$2
		  7b2	pushi	$1a  
		  7b4	push1
		  7b5	pushi	$1a  
		  7b7	push0
		  7b8	lat  	$2   	(obj)
		  7ba	send
		  7ba	word	$4
		  7bd	push
		  7be	ldi  	$4   
		  7c0	or
		  7c1	push
		  7c2	lat  	$2   	(obj)
		  7c4	send
		  7c4	word	$6
.15
		  7c7	pushi	$4c  
		  7c9	push1
		  7ca	lst  	$3   	(node)
		  7cc	self
		  7cc	word	$6
		  7cf	sat  	$3   	(node)
		  7d1	jmp  	$ff9b	(.13)
.16
.17
		  7d6	pToa 	$4e  	(numRow)
		  7d8	bnt  	$26  	(.18)
		  7e0	pushi	$16b 
		  7e3	pushi	$5   
		  7e6	pushi	$32  
		  7e8	dup
		  7e9	dup
		  7ea	pToa 	$42  	(iconRight)
		  7ec	add
		  7ed	push
		  7ee	pTos 	$56  	(itemWide)
		  7f0	pToa 	$54  	(colMargin)
		  7f2	add
		  7f3	push
		  7f4	pToa 	$50  	(numCol)
		  7f6	mul
		  7f7	push
		  7f8	push1
		  7f9	pToa 	$2e  	(plane)
		  7fb	send
		  7fb	word	$e
		  7fe	jmp  	$2a  	(.19)
.18
		  809	pushi	$16b 
		  80c	pushi	$5   
		  80f	pushi	$32  
		  811	dup
		  812	dup
		  813	pToa 	$42  	(iconRight)
		  815	add
		  816	push
		  817	pushi	$a   
		  819	lst  	$1   	(atY)
		  81b	pToa 	$40  	(iconBottom)
		  81d	add
		  81e	push
		  81f	ldi  	$5   
		  821	add
		  822	add
		  823	push
		  824	push1
		  825	pToa 	$2e  	(plane)
		  827	send
		  827	word	$e
.19
		  82d	ret


Method: (Inventory ownedBy)

		  831	file
		  83c	link 	$2   
		  841	push2
		  842	pushi	$3   
		  844	pTos 	$14  	(elements)
		  846	callk	$5a  	(KList)
		  846	word	$4
		  84a	sat  	$0   	(node)
.0
		  84f	lat  	$0   	(node)
		  851	bnt  	$4e  	(.3)
		  859	push2
		  85a	pushi	$6   
		  85c	push
		  85d	callk	$5a  	(KList)
		  85d	word	$4
		  861	aTop 	$18  	(nextNode)
		  866	push2
		  867	pushi	$8   
		  869	lst  	$0   	(node)
		  86b	callk	$5a  	(KList)
		  86b	word	$4
		  86f	sat  	$1   	(obj)
		  874	pushi	$91  
		  877	push1
		  878	class	$43  	(InvItem)
		  87a	push
		  87b	lat  	$1   	(obj)
		  87d	send
		  87d	word	$6
		  880	bnt  	$19  	(.2)
		  885	pushi	$1dc 
		  888	push1
		  889	lsp  	$1   	(whom)
		  88b	lat  	$1   	(obj)
		  88d	send
		  88d	word	$6
.1
		  890	bnt  	$9   	(.2)
		  898	ldi  	$1   
		  89a	ret
.2
		  89b	pToa 	$18  	(nextNode)
		  89d	sat  	$0   	(node)
		  89f	jmp  	$ffab	(.0)
.3
		  8a4	ldi  	$0   
		  8a6	ret
		  8aa	ret


Method: (Inventory showSelf)

		  8ae	file
		  8bc	pushi	$f2  
		  8bf	push0
		  8c0	lag  	$8   	(sounds)
		  8c2	send
		  8c2	word	$4
		  8c8	lag  	$4d  	(pMouse)
		  8ca	bnt  	$18  	(.1)
		  8cc	pushi	$93  
		  8cf	push1
		  8d0	pushi	$f1  
		  8d3	send
		  8d3	word	$6
.0
		  8d6	bnt  	$c   	(.1)
		  8db	pushi	$f1  
		  8de	push0
		  8df	lag  	$4d  	(pMouse)
		  8e1	send
		  8e1	word	$4
.1
		  8e7	pToa 	$3a  	(okButton)
		  8e9	not
		  8ea	bnt  	$14  	(.2)
		  8ef	push2
		  8f0	pushi	$8   
		  8f2	pushi	$9c  
		  8f5	push0
		  8f6	self
		  8f6	word	$4
		  8f9	push
		  8fa	callk	$5a  	(KList)
		  8fa	word	$4
		  8fe	aTop 	$3a  	(okButton)
.2
		  903	ldi  	$0   
		  905	aTop 	$20  	(curIcon)
		  90a	lap  	$0   	(argc)
		  90c	bnt  	$4   	(.3)
		  90e	lap  	$1   	(who)
		  910	jmp  	$2   	(.4)
.3
		  912	lag  	$0   	(ego)
.4
		  914	aTop 	$3e  	(owner)
		  919	pushi	$1dc 
		  91c	push1
		  91d	pTos 	$3e  	(owner)
		  91f	self
		  91f	word	$6
		  922	not
		  923	bnt  	$10  	(.5)
		  928	pushi	$1f4 
		  92b	push0
		  92c	self
		  92c	word	$4
		  932	ldi  	$0   
		  934	ret
.5
		  93b	pushi	$75  
		  93d	push0
		  941	pushi	$45  
		  943	push0
		  947	pushi	$69  
		  949	push0
		  94a	self
		  94a	word	$c
		  950	ret


Method: (Inventory show)

		  954	file
		  962	pTos 	$30  	(state)
		  964	ldi  	$20  
		  966	or
		  967	aTop 	$30  	(state)
		  96c	pushi	$19f 
		  96f	push1
		  970	pushi	$1b7 
		  973	push0
		  974	self
		  974	word	$4
		  977	push
		  978	lag  	$1   	(theGame)
		  97a	send
		  97a	word	$6
		  980	pushi	$7   
		  982	push1
		  983	push0
		  984	callk	$2c  	(GetHighPlanePri)
		  984	word	$0
		  988	push
		  989	ldi  	$1   
		  98b	add
		  98c	push
		  98d	pToa 	$2e  	(plane)
		  98f	send
		  98f	word	$6
		  995	push1
		  996	pTos 	$2e  	(plane)
		  998	callk	$2a  	(UpdatePlane)
		  998	word	$2
		  99f	pushi	$1f3 
		  9a2	push0
		  9a3	self
		  9a3	word	$4
		  9a9	ldi  	$1   
		  9ab	ret
		  9af	ret


Method: (Inventory hide)

		  9b3	file
		  9be	link 	$3   
		  9c3	pTos 	$30  	(state)
		  9c5	ldi  	$20  
		  9c7	and
		  9c8	bnt  	$17  	(.0)
		  9cd	pushi	$f2  
		  9d0	push1
		  9d1	push0
		  9d2	lag  	$8   	(sounds)
		  9d4	send
		  9d4	word	$6
		  9da	pTos 	$30  	(state)
		  9dc	ldi  	$ffdf
		  9de	and
		  9df	aTop 	$30  	(state)
.0
		  9e4	push2
		  9e5	pushi	$3   
		  9e7	pTos 	$14  	(elements)
		  9e9	callk	$5a  	(KList)
		  9e9	word	$4
		  9ed	sat  	$1   	(node)
.1
		  9f2	lat  	$1   	(node)
		  9f4	bnt  	$83  	(.4)
		  9fd	push2
		  9fe	pushi	$6   
		  a00	push
		  a01	callk	$5a  	(KList)
		  a01	word	$4
		  a05	aTop 	$18  	(nextNode)
		  a0a	push2
		  a0b	pushi	$8   
		  a0d	lst  	$1   	(node)
		  a0f	callk	$5a  	(KList)
		  a0f	word	$4
		  a13	sat  	$2   	(obj)
		  a18	pushi	$1a  
		  a1a	push1
		  a1b	pushi	$1a  
		  a1d	push0
		  a1e	send
		  a1e	word	$4
		  a21	push
		  a22	ldi  	$ffdf
		  a24	and
		  a25	push
		  a26	lat  	$2   	(obj)
		  a28	send
		  a28	word	$6
		  a2e	pushi	$91  
		  a31	push1
		  a32	class	$43  	(InvItem)
		  a34	push
		  a35	lat  	$2   	(obj)
		  a37	send
		  a37	word	$6
		  a3a	bnt  	$37  	(.3)
		  a3f	pushi	$1a  
		  a41	push0
		  a42	lat  	$2   	(obj)
		  a44	send
		  a44	word	$4
		  a47	push
		  a48	ldi  	$4   
		  a4a	and
		  a4b	not
.2
		  a4c	bnt  	$25  	(.3)
		  a54	pushi	$1a  
		  a56	push1
		  a57	pushi	$1a  
		  a59	push0
		  a5a	lat  	$2   	(obj)
		  a5c	send
		  a5c	word	$4
		  a5f	push
		  a60	ldi  	$4   
		  a62	or
		  a63	push
		  a64	lat  	$2   	(obj)
		  a66	send
		  a66	word	$6
		  a6c	push1
		  a6d	lst  	$2   	(obj)
		  a6f	callk	$1f  	(DeleteScreenItem)
		  a6f	word	$2
.3
		  a73	pToa 	$18  	(nextNode)
		  a75	sat  	$1   	(node)
		  a77	jmp  	$ff75	(.1)
.4
		  a7d	pushi	$7   
		  a7f	push1
		  a80	pushi	$ffff
		  a82	pToa 	$2e  	(plane)
		  a84	send
		  a84	word	$6
		  a8a	push1
		  a8b	pTos 	$2e  	(plane)
		  a8d	callk	$2a  	(UpdatePlane)
		  a8d	word	$2
		  a94	pToa 	$20  	(curIcon)
		  a96	bnt  	$9c  	(.7)
		  a9c	pushi	$91  
		  a9f	push1
		  aa0	class	$43  	(InvItem)
		  aa2	push
		  aa3	pToa 	$20  	(curIcon)
		  aa5	send
		  aa5	word	$6
.5
		  aa8	bnt  	$8a  	(.7)
		  ab1	pushi	$1cd 
		  ab4	push0
		  ab5	lag  	$45  	(theIconBar)
		  ab7	send
		  ab7	word	$4
		  aba	not
		  abb	bnt  	$16  	(.6)
		  ac0	pushi	$1d5 
		  ac3	push1
		  ac4	pushi	$1ce 
		  ac7	push0
		  ac8	lag  	$45  	(theIconBar)
		  aca	send
		  aca	word	$4
		  acd	push
		  ace	lag  	$45  	(theIconBar)
		  ad0	send
		  ad0	word	$6
.6
		  ad9	pushi	$1ca 
		  adc	push1
		  ae3	pushi	$1be 
		  ae6	push1
		  ae7	pushi	$e   
		  ae9	push0
		  aea	lofsa	$fc  	(invCursor)
		  aed	send
		  aed	word	$4
		  af0	push
		  af4	pushi	$1bf 
		  af7	push1
		  af8	pushi	$f   
		  afa	push0
		  afb	lofsa	$fc  	(invCursor)
		  afe	send
		  afe	word	$4
		  b01	push
		  b05	pushi	$1c0 
		  b08	push1
		  b09	pushi	$10  
		  b0b	push0
		  b0c	lofsa	$fc  	(invCursor)
		  b0f	send
		  b0f	word	$4
		  b12	push
		  b16	pushi	$94  
		  b19	push0
		  b1a	pushi	$1ce 
		  b1d	push0
		  b1e	lag  	$45  	(theIconBar)
		  b20	send
		  b20	word	$4
		  b23	send
		  b23	word	$16
		  b26	push
		  b2a	pushi	$1cd 
		  b2d	push1
		  b2e	pTos 	$20  	(curIcon)
		  b30	lag  	$45  	(theIconBar)
		  b32	send
		  b32	word	$c
.7
		  b38	ret


Method: (Inventory advance)

		  b3c	file
		  b47	link 	$4   
		  b4c	lap  	$0   	(argc)
		  b4e	bnt  	$4   	(.0)
		  b50	lap  	$1   	(amount)
		  b52	jmp  	$2   	(.1)
.0
		  b54	ldi  	$1   
.1
		  b56	sat  	$1   	(toMove)
		  b5b	pushi	$a4  
		  b5e	push1
		  b5f	pTos 	$22  	(highlightedIcon)
		  b61	self
		  b61	word	$6
		  b64	sat  	$2   	(highlightedNo)
		  b69	lst  	$1   	(toMove)
		  b6b	add
		  b6c	sat  	$3   	(nextIcon)
.2
		  b77	pushi	$4b  
		  b79	push1
		  b7d	lst  	$3   	(nextIcon)
		  b7f	pToa 	$16  	(size)
		  b81	le?
		  b82	bnt  	$7   	(.3)
		  b87	lat  	$3   	(nextIcon)
		  b89	jmp  	$e   	(.4)
.3
		  b91	lst  	$3   	(nextIcon)
		  b93	pTos 	$16  	(size)
		  b95	ldi  	$1   
		  b97	sub
		  b98	mod
.4
		  b99	push
		  b9a	self
		  b9a	word	$6
		  b9d	sat  	$0   	(theIcon)
		  ba2	not
		  ba3	bnt  	$14  	(.5)
		  ba8	push2
		  ba9	pushi	$8   
		  bab	pushi	$9c  
		  bae	push0
		  baf	self
		  baf	word	$4
		  bb2	push
		  bb3	callk	$5a  	(KList)
		  bb3	word	$4
		  bb7	sat  	$0   	(theIcon)
.5
		  bbc	pushi	$1a  
		  bbe	push0
		  bbf	lat  	$0   	(theIcon)
		  bc1	send
		  bc1	word	$4
		  bc4	push
		  bc5	ldi  	$4   
		  bc7	and
		  bc8	not
		  bc9	bnt  	$7   	(.6)
		  bce	jmp  	$c   	(.8)
		  bd0	jmp  	$ff9f	(.2)
.6
		  bd8	+at  	$3   	(nextIcon)
.7
		  bda	jmp  	$ff95	(.2)
.8
		  bdf	pushi	$1c5 
		  be2	push2
		  be3	lst  	$0   	(theIcon)
		  be5	push1
		  be6	self
		  be6	word	$8
		  bec	ret


Method: (Inventory retreat)

		  bf0	file
		  bfb	link 	$4   
		  c00	lap  	$0   	(argc)
		  c02	bnt  	$4   	(.0)
		  c04	lap  	$1   	(amount)
		  c06	jmp  	$2   	(.1)
.0
		  c08	ldi  	$1   
.1
		  c0a	sat  	$1   	(toMove)
		  c0f	pushi	$a4  
		  c12	push1
		  c13	pTos 	$22  	(highlightedIcon)
		  c15	self
		  c15	word	$6
		  c18	sat  	$2   	(highlightedNo)
		  c1d	push
		  c1e	lat  	$1   	(toMove)
		  c20	sub
		  c21	sat  	$3   	(nextIcon)
.2
		  c2c	pushi	$4b  
		  c2e	push1
		  c2f	lst  	$3   	(nextIcon)
		  c31	self
		  c31	word	$6
		  c34	sat  	$0   	(theIcon)
		  c39	not
		  c3a	bnt  	$14  	(.3)
		  c3f	push2
		  c40	pushi	$8   
		  c42	pushi	$9f  
		  c45	push0
		  c46	self
		  c46	word	$4
		  c49	push
		  c4a	callk	$5a  	(KList)
		  c4a	word	$4
		  c4e	sat  	$0   	(theIcon)
.3
		  c53	pushi	$1a  
		  c55	push0
		  c56	lat  	$0   	(theIcon)
		  c58	send
		  c58	word	$4
		  c5b	push
		  c5c	ldi  	$4   
		  c5e	and
		  c5f	not
		  c60	bnt  	$7   	(.4)
		  c65	jmp  	$c   	(.6)
		  c67	jmp  	$ffbd	(.2)
.4
		  c6f	-at  	$3   	(nextIcon)
.5
		  c71	jmp  	$ffb3	(.2)
.6
		  c76	pushi	$1c5 
		  c79	push2
		  c7a	lst  	$0   	(theIcon)
		  c7c	push1
		  c7d	self
		  c7d	word	$8
		  c83	ret


Procedure: (FindIcon)

		  c87	file
		  c92	link 	$3   
		  c97	pushi	$14  
		  c99	push0
		  c9a	lap  	$1   	(hIcon)
		  c9c	send
		  c9c	word	$4
		  c9f	push
		  ca0	push1
		  ca1	push0
		  ca2	lap  	$1   	(hIcon)
		  ca4	send
		  ca4	word	$4
		  ca7	sub
		  ca8	push
		  ca9	ldi  	$2   
		  cab	div
		  cac	push
		  cad	push1
		  cae	push0
		  caf	lap  	$1   	(hIcon)
		  cb1	send
		  cb1	word	$4
		  cb4	add
		  cb5	sat  	$2   	(theX)
		  cba	lap  	$2   	(y1)
		  cbc	sat  	$1   	(theY)
.0
		  cc1	push1
		  cc2	lst  	$1   	(theY)
		  cc4	lap  	$3   	(y2)
		  cc6	sub
		  cc7	push
		  cc8	callk	$3d  	(Abs)
		  cc8	word	$2
		  ccc	push
		  ccd	ldi  	$4   
		  ccf	ge?
		  cd0	bnt  	$63  	(.4)
		  cdc	pushi	$98  
		  cdf	push2
		  ce0	pushi	$10e 
		  ce9	push1
		  cea	push1
		  ceb	lst  	$2   	(theX)
		  cf0	push2
		  cf1	push1
		  cf2	lst  	$1   	(theY)
		  cf7	pushi	$94  
		  cfa	push0
		  cfb	pushi	$8d  
		  cfe	push0
		  cff	pushi	$1da 
		  d02	push0
		  d03	lag  	$50  	(user)
		  d05	send
		  d05	word	$4
		  d08	send
		  d08	word	$4
		  d0b	send
		  d0b	word	$10
		  d0e	push
		  d0f	self
		  d0f	word	$8
		  d12	sat  	$0   	(thisIcon)
		  d14	bnt  	$7   	(.1)
		  d1c	ret
.1
		  d1d	lsp  	$2   	(y1)
		  d1f	lap  	$3   	(y2)
		  d21	lt?
		  d22	bnt  	$9   	(.2)
		  d24	lst  	$1   	(theY)
		  d26	ldi  	$4   
		  d28	add
		  d29	sat  	$1   	(theY)
		  d2b	jmp  	$ff91	(.0)
.2
		  d2d	lst  	$1   	(theY)
		  d2f	ldi  	$4   
		  d31	sub
		  d32	sat  	$1   	(theY)
.3
		  d34	jmp  	$ff88	(.0)
.4
		  d39	ret


Method: (Inventory doit)

		  d3d	file
		  d48	link 	$c   
		  d50	pushi	$ba  
		  d53	push2
		  d54	pushi	$64  
		  d56	lofsa	$198 
		  d59	push
		  d5a	class	$f   	(String)
		  d5c	send
		  d5c	word	$8
		  d5f	sat  	$a   	(buffer)
.0
		  d64	pushi	$2b  
		  d66	push0
		  d67	pushi	$8d  
		  d6a	push0
		  d6b	pushi	$1da 
		  d6e	push0
		  d6f	lag  	$50  	(user)
		  d71	send
		  d71	word	$4
		  d74	send
		  d74	word	$4
		  d77	sat  	$1   	(event)
		  d79	send
		  d79	word	$4
		  d7c	bnt  	$2   	(.1)
		  d7e	jmp  	$ffe4	(.0)
.1
.2
		  d83	pTos 	$30  	(state)
		  d85	ldi  	$20  
		  d87	and
		  d88	bnt  	$626 	(.60)
		  d8e	pushi	$8d  
		  d91	push0
		  d92	pushi	$1da 
		  d95	push0
		  d96	lag  	$50  	(user)
		  d98	send
		  d98	word	$4
		  d9b	send
		  d9b	word	$4
		  d9e	sat  	$1   	(event)
		  da3	push1
		  da4	push0
		  da5	send
		  da5	word	$4
		  da8	sag  	$46  	(mouseX)
		  dad	push2
		  dae	push0
		  daf	lat  	$1   	(event)
		  db1	send
		  db1	word	$4
		  db4	sag  	$47  	(mouseY)
		  db9	pushi	$2b  
		  dbb	push0
		  dbc	lat  	$1   	(event)
		  dbe	send
		  dbe	word	$4
		  dc1	sat  	$2   	(eType)
		  dc6	pushi	$31  
		  dc8	push0
		  dc9	lat  	$1   	(event)
		  dcb	send
		  dcb	word	$4
		  dce	sat  	$3   	(eMsg)
		  dd3	pushi	$48  
		  dd5	push0
		  dd6	lat  	$1   	(event)
		  dd8	send
		  dd8	word	$4
		  ddb	sat  	$4   	(eMod)
		  de0	ldi  	$0   
		  de2	sat  	$9   	(keyInvoked)
		  de7	lsg  	$56  	(tickOffset)
		  de9	push0
		  dea	callk	$79  	(GetTime)
		  dea	word	$0
		  dee	add
		  def	sag  	$58  	(gameTime)
		  df4	push0
		  df5	callk	$21  	(FrameOut)
		  df5	word	$0
		  dfc	pushi	$b5  
		  dff	push1
		  e00	pTos 	$2e  	(plane)
		  e02	lat  	$1   	(event)
		  e04	send
		  e04	word	$6
		  e0a	pushi	$1f7 
		  e0d	push0
		  e0e	lag  	$59  	(narrator)
		  e10	send
		  e10	word	$4
		  e13	bnt  	$1f  	(.3)
		  e18	pushi	$45  
		  e1a	push0
		  e1b	lag  	$59  	(narrator)
		  e1d	send
		  e1d	word	$4
		  e23	pushi	$a6  
		  e26	push1
		  e27	lst  	$1   	(event)
		  e29	lag  	$59  	(narrator)
		  e2b	send
		  e2b	word	$6
		  e31	jmp  	$ff4f	(.2)
.3
		  e3a	pToa 	$20  	(curIcon)
		  e3c	bnt  	$9d  	(.10)
		  e42	lat  	$4   	(eMod)
		  e44	not
		  e45	bnt  	$94  	(.10)
		  e4b	pTos 	$20  	(curIcon)
		  e4d	pToa 	$3c  	(selectIcon)
		  e4f	ne?
		  e50	bnt  	$89  	(.10)
		  e59	lst  	$2   	(eType)
		  e5b	ldi  	$1   
		  e5d	eq?
		  e5e	bt   	$35  	(.6)
		  e66	lst  	$2   	(eType)
		  e68	ldi  	$4   
		  e6a	eq?
		  e6b	bnt  	$11  	(.4)
		  e70	lst  	$3   	(eMsg)
		  e72	ldi  	$d   
		  e74	eq?
		  e75	bnt  	$7   	(.4)
		  e7a	ldi  	$1   
		  e7c	sat  	$9   	(keyInvoked)
.4
		  e7e	bt   	$15  	(.6)
		  e86	lst  	$2   	(eType)
		  e88	ldi  	$20  
		  e8a	eq?
		  e8b	bnt  	$4e  	(.10)
		  e91	ldi  	$1   
		  e93	sat  	$9   	(keyInvoked)
.5
.6
.7
		  e95	bnt  	$44  	(.10)
		  ea1	pTos 	$20  	(curIcon)
		  ea3	pToa 	$2a  	(helpIconItem)
		  ea5	ne?
		  ea6	bt   	$f   	(.8)
		  eab	pushi	$1a  
		  ead	push0
		  eae	pToa 	$2a  	(helpIconItem)
		  eb0	send
		  eb0	word	$4
		  eb3	push
		  eb4	ldi  	$10  
		  eb6	and
.8
		  eb7	bnt  	$23  	(.9)
		  ec2	pushi	$2b  
		  ec4	push1
		  ec5	pushi	$4000
		  ecb	pushi	$31  
		  ecd	push1
		  ece	pushi	$31  
		  ed0	push0
		  ed1	pToa 	$20  	(curIcon)
		  ed3	send
		  ed3	word	$4
		  ed6	push
		  ed7	lat  	$1   	(event)
		  ed9	send
		  ed9	word	$c
.9
.10
		  edf	push1
		  ee0	lst  	$1   	(event)
		  ee2	callk	$53  	(MapKeyToDir)
		  ee2	word	$2
		  ee9	pushi	$2b  
		  eeb	push0
		  eec	lat  	$1   	(event)
		  eee	send
		  eee	word	$4
		  ef1	sat  	$2   	(eType)
		  ef6	pushi	$31  
		  ef8	push0
		  ef9	lat  	$1   	(event)
		  efb	send
		  efb	word	$4
		  efe	sat  	$3   	(eMsg)
		  f03	lag  	$12  	(cuees)
		  f05	bnt  	$c   	(.11)
		  f0a	pushi	$97  
		  f0d	push1
		  f0e	pushi	$45  
		  f10	send
		  f10	word	$6
.11
		  f16	pushi	$b0  
		  f19	push0
		  f1a	lag  	$1   	(theGame)
		  f1c	send
		  f1c	word	$4
		  f1f	sat  	$b   	(tut)
		  f21	bnt  	$1f  	(.13)
		  f26	pushi	$91  
		  f29	push1
		  f2a	class	$12  	(Tutorial)
		  f2c	push
		  f2d	lat  	$b   	(tut)
		  f2f	send
		  f2f	word	$6
.12
		  f32	bnt  	$e   	(.13)
		  f3a	pushi	$45  
		  f3c	push0
		  f3d	lat  	$b   	(tut)
		  f3f	send
		  f3f	word	$4
.13
		  f48	lst  	$2   	(eType)
		  f4a	ldi  	$1   
		  f4c	eq?
		  f4d	bnt  	$1d  	(.15)
		  f4f	lat  	$4   	(eMod)
.14
		  f51	bnt  	$19  	(.15)
		  f56	pushi	$1d1 
		  f59	push0
		  f5a	self
		  f5a	word	$4
		  f60	pushi	$50  
		  f62	push1
		  f63	push1
		  f64	lat  	$1   	(event)
		  f66	send
		  f66	word	$6
		  f69	jmp  	$fe17	(.2)
.15
		  f72	lst  	$2   	(eType)
		  f74	ldi  	$0   
		  f76	eq?
		  f77	bnt  	$2e  	(.17)
		  f7c	pushi	$98  
		  f7f	push2
		  f80	pushi	$10e 
		  f83	lst  	$1   	(event)
		  f85	self
		  f85	word	$8
		  f88	sat  	$0   	(thisIcon)
		  f8a	bnt  	$1b  	(.17)
		  f8f	push
		  f90	pToa 	$22  	(highlightedIcon)
		  f92	ne?
.16
		  f93	bnt  	$12  	(.17)
		  f9b	pushi	$1c5 
		  f9e	push1
		  f9f	lst  	$0   	(thisIcon)
		  fa1	self
		  fa1	word	$6
		  fa4	jmp  	$fddc	(.2)
.17
		  fad	lst  	$2   	(eType)
		  faf	ldi  	$1   
		  fb1	eq?
		  fb2	bt   	$1f  	(.19)
		  fba	lst  	$2   	(eType)
		  fbc	ldi  	$4   
		  fbe	eq?
		  fbf	bnt  	$8   	(.18)
		  fc4	lst  	$3   	(eMsg)
		  fc6	ldi  	$d   
		  fc8	eq?
.18
		  fc9	bt   	$8   	(.19)
		  fce	lst  	$2   	(eType)
		  fd0	ldi  	$20  
		  fd2	eq?
.19
		  fd3	bnt  	$cd  	(.27)
		  fe2	pToa 	$22  	(highlightedIcon)
		  fe4	bnt  	$fd9c	(.2)
		  fea	pushi	$1c4 
		  fed	push2
		  fee	push
		  fef	lst  	$2   	(eType)
		  ff1	ldi  	$1   
		  ff3	eq?
		  ff4	push
		  ff5	self
		  ff5	word	$8
.20
		  ff8	bnt  	$fd88	(.2)
		 1004	pTos 	$22  	(highlightedIcon)
		 1006	pToa 	$3a  	(okButton)
		 1008	eq?
		 1009	bnt  	$15  	(.21)
		 100e	pushi	$50  
		 1010	push1
		 1011	push1
		 1012	lat  	$1   	(event)
		 1014	send
		 1014	word	$6
		 101a	jmp  	$394 	(.60)
		 101d	jmp  	$fd63	(.2)
.21
		 1023	pTos 	$22  	(highlightedIcon)
		 1025	pToa 	$2a  	(helpIconItem)
		 1027	eq?
		 1028	bnt  	$57  	(.24)
		 102e	pToa 	$22  	(highlightedIcon)
		 1030	aTop 	$20  	(curIcon)
		 1035	pushi	$19f 
		 1038	push1
		 1039	pushi	$1b7 
		 103c	push0
		 103d	self
		 103d	word	$4
		 1040	push
		 1041	lag  	$1   	(theGame)
		 1043	send
		 1043	word	$6
		 104c	pTos 	$30  	(state)
		 104e	ldi  	$800 
		 1051	and
		 1052	bnt  	$d   	(.22)
		 1057	pushi	$1d6 
		 105a	push0
		 105b	self
		 105b	word	$4
		 105e	jmp  	$fd22	(.2)
.22
		 1064	pToa 	$2a  	(helpIconItem)
		 1066	bnt  	$fd1a	(.2)
		 106c	pushi	$1a  
		 106e	push1
		 106f	pushi	$1a  
		 1071	push0
		 1072	send
		 1072	word	$4
		 1075	push
		 1076	ldi  	$10  
		 1078	or
		 1079	push
		 107a	pToa 	$2a  	(helpIconItem)
		 107c	send
		 107c	word	$6
.23
		 107f	jmp  	$fd01	(.2)
.24
		 1088	pToa 	$22  	(highlightedIcon)
		 108a	aTop 	$20  	(curIcon)
		 108f	pushi	$19f 
		 1092	push1
		 1093	pushi	$1b7 
		 1096	push0
		 1097	self
		 1097	word	$4
		 109a	push
		 109b	lag  	$1   	(theGame)
		 109d	send
		 109d	word	$6
.25
.26
		 10a0	jmp  	$fce0	(.2)
.27
		 10a6	lst  	$2   	(eType)
		 10a8	ldi  	$10  
		 10aa	and
		 10ab	bnt  	$106 	(.40)
		 10b1	lst  	$3   	(eMsg)
		 10b3	dup
		 10b7	ldi  	$3   
		 10b9	eq?
		 10ba	bnt  	$d   	(.28)
		 10bf	pushi	$19d 
		 10c2	push0
		 10c3	self
		 10c3	word	$4
		 10c6	jmp  	$e7  	(.39)
.28
		 10c9	dup
		 10cd	ldi  	$7   
		 10cf	eq?
		 10d0	bnt  	$d   	(.29)
		 10d5	pushi	$19e 
		 10d8	push0
		 10d9	self
		 10d9	word	$4
		 10dc	jmp  	$d1  	(.39)
.29
		 10df	dup
		 10e3	ldi  	$1   
		 10e5	eq?
		 10e6	bnt  	$4c  	(.33)
		 10ef	pToa 	$22  	(highlightedIcon)
		 10f1	bnt  	$31  	(.31)
		 10fa	pushi	$3   
		 10fc	push
		 10fd	pushi	$13  
		 10ff	push0
		 1100	send
		 1100	word	$4
		 1103	push
		 1104	ldi  	$1   
		 1106	sub
		 1107	push
		 1108	push0
		 1109	call 	$fb76	(FindIcon)
		 1109	word	$6
		 110e	sat  	$0   	(thisIcon)
.30
		 1110	bnt  	$13  	(.31)
		 1118	pushi	$1c5 
		 111b	push2
		 111c	lst  	$0   	(thisIcon)
		 111e	push1
		 111f	self
		 111f	word	$8
		 1122	jmp  	$8b  	(.39)
.31
		 112b	pushi	$19e 
		 112e	push0
		 112f	self
		 112f	word	$4
.32
		 1132	jmp  	$7b  	(.39)
.33
		 1135	dup
		 1139	ldi  	$5   
		 113b	eq?
		 113c	bnt  	$54  	(.37)
		 1145	pToa 	$22  	(highlightedIcon)
		 1147	bnt  	$3a  	(.35)
		 1150	pushi	$3   
		 1152	push
		 1153	pushi	$15  
		 1155	push0
		 1156	send
		 1156	word	$4
		 1159	push
		 115a	ldi  	$1   
		 115c	add
		 115d	push
		 115e	pushi	$8a  
		 1161	push0
		 1162	pToa 	$2e  	(plane)
		 1164	send
		 1164	word	$4
		 1167	push
		 1168	call 	$fb17	(FindIcon)
		 1168	word	$6
		 116d	sat  	$0   	(thisIcon)
.34
		 116f	bnt  	$13  	(.35)
		 1177	pushi	$1c5 
		 117a	push2
		 117b	lst  	$0   	(thisIcon)
		 117d	push1
		 117e	self
		 117e	word	$8
		 1181	jmp  	$2c  	(.39)
.35
		 118a	pushi	$19d 
		 118d	push0
		 118e	self
		 118e	word	$4
.36
		 1191	jmp  	$1d  	(.39)
.37
		 1193	dup
		 1197	ldi  	$0   
		 1199	eq?
		 119a	bnt  	$14  	(.39)
		 119f	lst  	$2   	(eType)
		 11a1	ldi  	$4   
		 11a3	and
		 11a4	bnt  	$a   	(.38)
		 11a9	pushi	$1d1 
		 11ac	push0
		 11ad	self
		 11ad	word	$4
.38
.39
		 11b0	toss
		 11b1	jmp  	$fbcf	(.2)
.40
		 11b7	lst  	$2   	(eType)
		 11b9	ldi  	$4   
		 11bb	eq?
		 11bc	bnt  	$44  	(.44)
		 11c2	lst  	$3   	(eMsg)
		 11c4	dup
		 11c8	ldi  	$9   
		 11ca	eq?
		 11cb	bnt  	$d   	(.41)
		 11d0	pushi	$19d 
		 11d3	push0
		 11d4	self
		 11d4	word	$4
		 11d7	jmp  	$25  	(.43)
.41
		 11da	dup
		 11de	ldi  	$f00 
		 11e1	eq?
		 11e2	bnt  	$c   	(.42)
		 11e7	pushi	$19e 
		 11ea	push0
		 11eb	self
		 11eb	word	$4
		 11ee	jmp  	$f   	(.43)
.42
		 11f0	dup
		 11f4	ldi  	$1b  
		 11f6	eq?
		 11f7	bnt  	$6   	(.43)
		 11fc	jmp  	$1b2 	(.60)
.43
		 11ff	toss
		 1200	jmp  	$fb80	(.2)
.44
		 1206	lst  	$2   	(eType)
		 1208	ldi  	$4000
		 120b	and
		 120c	bnt  	$fb74	(.2)
		 1212	pushi	$98  
		 1215	push2
		 1216	pushi	$10e 
		 1219	lst  	$1   	(event)
		 121b	self
		 121b	word	$8
		 121e	sat  	$0   	(thisIcon)
		 1220	bnt  	$fb60	(.2)
		 1229	lst  	$2   	(eType)
		 122b	ldi  	$2000
		 122e	and
		 122f	bnt  	$92  	(.48)
		 1235	lat  	$0   	(thisIcon)
		 1237	bnt  	$61  	(.47)
		 123a	pushi	$124 
		 123d	push0
		 123e	send
		 123e	word	$4
.45
		 1241	bnt  	$57  	(.47)
		 1247	pushi	$7   
		 1249	push0
		 124d	pushi	$123 
		 1250	push0
		 1251	lat  	$0   	(thisIcon)
		 1253	send
		 1253	word	$4
		 1256	push
		 125a	pushi	$124 
		 125d	push0
		 125e	lat  	$0   	(thisIcon)
		 1260	send
		 1260	word	$4
		 1263	push
		 1267	pushi	$1c3 
		 126a	push0
		 126b	lat  	$0   	(thisIcon)
		 126d	send
		 126d	word	$4
		 1270	push
		 1274	push0
		 1278	push1
		 127c	pushi	$71  
		 127e	push0
		 127f	lat  	$a   	(buffer)
		 1281	send
		 1281	word	$4
		 1284	push
		 1285	callk	$48  	(Message)
		 1285	word	$e
		 1289	bnt  	$10  	(.46)
		 1291	push1
		 1292	lst  	$a   	(buffer)
		 1294	calle	$fd99/0	(Prints)
		 1294	word	$2
.46
.47
		 129e	pushi	$1a  
		 12a0	push1
		 12a1	pushi	$1a  
		 12a3	push0
		 12a4	pToa 	$2a  	(helpIconItem)
		 12a6	send
		 12a6	word	$4
		 12a9	push
		 12aa	ldi  	$ffef
		 12ac	and
		 12ad	push
		 12ae	pToa 	$2a  	(helpIconItem)
		 12b0	send
		 12b0	word	$6
		 12b6	pushi	$19f 
		 12b9	push1
		 12ba	lsg  	$14  	(normalCursor)
		 12bc	lag  	$1   	(theGame)
		 12be	send
		 12be	word	$6
		 12c1	jmp  	$fabf	(.2)
.48
		 12c7	lst  	$0   	(thisIcon)
		 12c9	pToa 	$3a  	(okButton)
		 12cb	eq?
		 12cc	bnt  	$15  	(.49)
		 12d1	pushi	$50  
		 12d3	push1
		 12d4	push1
		 12d5	lat  	$1   	(event)
		 12d7	send
		 12d7	word	$6
		 12dd	jmp  	$d1  	(.60)
		 12e0	jmp  	$faa0	(.2)
.49
		 12e6	pushi	$91  
		 12e9	push1
		 12ea	class	$43  	(InvItem)
		 12ec	push
		 12ed	lat  	$0   	(thisIcon)
		 12ef	send
		 12ef	word	$6
		 12f2	not
		 12f3	bnt  	$6f  	(.54)
		 12f9	pushi	$1c4 
		 12fc	push2
		 12fd	lst  	$0   	(thisIcon)
		 12ff	lat  	$9   	(keyInvoked)
		 1301	not
		 1302	push
		 1303	self
		 1303	word	$8
		 1306	bnt  	$fa7a	(.2)
		 130c	lat  	$0   	(thisIcon)
		 130e	aTop 	$20  	(curIcon)
		 1313	pushi	$19f 
		 1316	push1
		 1317	pushi	$1b7 
		 131a	push0
		 131b	self
		 131b	word	$4
		 131e	push
		 131f	lag  	$1   	(theGame)
		 1321	send
		 1321	word	$6
		 1327	lst  	$0   	(thisIcon)
		 1329	pToa 	$2a  	(helpIconItem)
		 132b	eq?
		 132c	bnt  	$fa54	(.2)
		 1332	pTos 	$30  	(state)
		 1334	ldi  	$800 
		 1337	and
		 1338	bnt  	$d   	(.50)
		 133d	pushi	$1d6 
		 1340	push0
		 1341	self
		 1341	word	$4
		 1344	jmp  	$fa3c	(.2)
.50
		 134d	pushi	$1a  
		 134f	push1
		 1350	pushi	$1a  
		 1352	push0
		 1353	pToa 	$2a  	(helpIconItem)
		 1355	send
		 1355	word	$4
		 1358	push
		 1359	ldi  	$10  
		 135b	or
		 135c	push
		 135d	pToa 	$2a  	(helpIconItem)
		 135f	send
		 135f	word	$6
.51
.52
.53
		 1362	jmp  	$fa1e	(.2)
.54
		 1368	pToa 	$20  	(curIcon)
		 136a	bnt  	$fa16	(.2)
		 1370	pushi	$91  
		 1373	push1
		 1374	class	$43  	(InvItem)
		 1376	push
		 1377	pToa 	$20  	(curIcon)
		 1379	send
		 1379	word	$6
		 137c	bnt  	$18  	(.55)
		 1381	pushi	$133 
		 1384	push1
		 1385	pushi	$31  
		 1387	push0
		 1388	pToa 	$20  	(curIcon)
		 138a	send
		 138a	word	$4
		 138d	push
		 138e	lat  	$0   	(thisIcon)
		 1390	send
		 1390	word	$6
		 1393	jmp  	$f9ed	(.2)
.55
		 139c	pushi	$133 
		 139f	push1
		 13a0	pushi	$31  
		 13a2	push0
		 13a3	lat  	$1   	(event)
		 13a5	send
		 13a5	word	$4
		 13a8	push
		 13a9	lat  	$0   	(thisIcon)
		 13ab	send
		 13ab	word	$6
.56
.57
.58
.59
		 13ae	jmp  	$f9d2	(.2)
.60
		 13b4	pushi	$8f  
		 13b7	push0
		 13b8	lat  	$a   	(buffer)
		 13ba	send
		 13ba	word	$4
		 13c0	ret


Method: (Inventory carryingNothing)

		 13c4	file
		 13d5	pushi	$1a1 
		 13d8	pushi	$3   
		 13db	lofsa	$199 
		 13de	push
		 13df	pTos 	$34  	(normalHeading)
		 13e1	pTos 	$38  	(empty)
		 13e6	pushi	$8e  
		 13e9	push0
		 13ea	class	$3f  	(Print)
		 13ec	send
		 13ec	word	$e
		 13f2	ldi  	$0   
		 13f4	ret
		 13f8	ret


Method: (Inventory setCurIndex)

		 13fc	file
		 1407	link 	$5   
		 140c	lsp  	$1   	(amount)
		 140e	ldi  	$0   
		 1410	lt?
		 1411	bnt  	$4   	(.0)
		 1413	ldi  	$ffff
		 1415	jmp  	$2   	(.1)
.0
		 1417	ldi  	$1   
.1
		 1419	sat  	$1   	(theDir)
		 141e	ldi  	$0   
		 1420	sat  	$2   	(numOwned)
		 1425	pToa 	$4c  	(curIndex)
		 1427	sat  	$4   	(idx)
		 1429	ldi  	$0   
		 142b	sat  	$2   	(numOwned)
.2
		 1430	lst  	$2   	(numOwned)
		 1432	push1
		 1433	lsp  	$1   	(amount)
		 1435	callk	$3d  	(Abs)
		 1435	word	$2
		 1439	lt?
		 143a	bnt  	$66  	(.7)
		 1440	lst  	$4   	(idx)
		 1442	pToa 	$16  	(size)
		 1444	lt?
		 1445	bnt  	$5b  	(.7)
		 144b	lst  	$4   	(idx)
		 144d	ldi  	$0   
		 144f	ge?
.3
		 1450	bnt  	$50  	(.7)
		 1459	lst  	$4   	(idx)
		 145b	lat  	$1   	(theDir)
		 145d	add
		 145e	sat  	$4   	(idx)
		 1463	push0
		 1464	le?
		 1465	bnt  	$ffc6	(.2)
		 1467	pprev
		 1468	pTos 	$16  	(size)
		 146a	ldi  	$1   
		 146c	sub
		 146d	le?
.4
		 146e	bnt  	$ffbd	(.2)
		 1473	pushi	$91  
		 1476	push1
		 1477	class	$43  	(InvItem)
		 1479	push
		 147a	pushi	$4b  
		 147c	push1
		 147d	lst  	$4   	(idx)
		 147f	self
		 147f	word	$6
		 1482	sat  	$3   	(obj)
		 1484	send
		 1484	word	$6
		 1487	bnt  	$ffa4	(.2)
		 148c	pushi	$1dc 
		 148f	push1
		 1490	pTos 	$3e  	(owner)
		 1492	lat  	$3   	(obj)
		 1494	send
		 1494	word	$6
.5
		 1497	bnt  	$ff94	(.2)
		 149f	+at  	$2   	(numOwned)
.6
		 14a1	jmp  	$ff8a	(.2)
.7
		 14a6	lst  	$2   	(numOwned)
		 14a8	push1
		 14a9	lsp  	$1   	(amount)
		 14ab	callk	$3d  	(Abs)
		 14ab	word	$2
		 14af	lt?
		 14b0	bnt  	$6   	(.8)
		 14b5	ret
		 14b6	jmp  	$a   	(.9)
.8
		 14be	lat  	$4   	(idx)
		 14c0	aTop 	$4c  	(curIndex)
.9
		 14c5	pushi	$1f3 
		 14c8	push0
		 14c9	self
		 14c9	word	$4
		 14cf	ret


Method: (Inventory setCurIndexTo)

		 14d3	file
		 14e1	lap  	$1   	(index)
		 14e3	aTop 	$4c  	(curIndex)
.0
		 14e8	pushi	$1dc 
		 14eb	push1
		 14ec	pTos 	$3e  	(owner)
		 14ee	pushi	$4b  
		 14f0	push1
		 14f1	pTos 	$4c  	(curIndex)
		 14f3	self
		 14f3	word	$6
		 14f6	send
		 14f6	word	$6
		 14f9	not
		 14fa	bnt  	$7   	(.1)
		 14ff	ipToa	$4c  	(curIndex)
		 1501	jmp  	$ffe5	(.0)
.1
		 1506	pushi	$1f3 
		 1509	push0
		 150a	self
		 150a	word	$4
		 1510	ret
		 1511	byte	$0


Fixups:
		 1512	word	$6
		 1514	word	$359
		 1516	word	$aeb
		 1518	word	$afc
		 151a	word	$b0d
		 151c	word	$d57
		 151e	word	$13dc
