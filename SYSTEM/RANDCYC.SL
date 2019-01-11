
				Listing of randcyc.sc:	[script 64941]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$64


Variables:
		    2	word	$0



Object: RandCycle           
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $12  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $2c  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $66  	(-script-)
		   10	prop  $15  	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $4e  	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(client)
		   1a	prop  $0   	(caller)
		   1c	prop  $1   	(cycleDir)
		   1e	prop  $0   	(cycleCnt)
		   20	prop  $0   	(completed)
		   22	prop  $0   	(clientLastCel)
		   24	prop  $ffff	(count)
		   26	prop  $0   	(reset)

Object: RTRandCycle         
		(properties)
		   28	prop  $1234	(-objID-)
		   2a	prop  $12  	(-size-)
		   2c	ofs   $3e  	(-propDict-)
		   2e	ofs   $62  	(-methDict-)
		   30	prop  $0   	(-classScript-)
		   32	prop  $67  	(-script-)
		   34	prop  $66  	(-super-)
		   36	prop  $8000	(-info-)
		   38	text  $58  	(name)
		   3a	prop  $0   	(scratch)
		   3c	prop  $0   	(client)
		   3e	prop  $0   	(caller)
		   40	prop  $1   	(cycleDir)
		   42	prop  $0   	(cycleCnt)
		   44	prop  $0   	(completed)
		   46	prop  $0   	(clientLastCel)
		   48	prop  $ffff	(count)
		   4a	prop  $0   	(reset)
		   4c	word	$0



		   4e	text
"RandCycle"

		   58	text
"RTRandCycle"



Fixups:
		   64	word	$2
		   66	word	$14
		   68	word	$38




----------------------
-------- Hunk --------
----------------------

		    0	word	$1fe
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: RandCycle           
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
		   1e	word	$b1
		   20	word	$fb
		   22	word	$e3
		   24	word	$fc
		   26	word	$fd
		   28	word	$261
		   2a	word	$265
		(method dictionary)
		   2c	word	$4
		   2e	word	$8e
		   30	local $64  	(init)
		   32	word	$45
		   34	local $fd  	(doit)
		   36	word	$fe
		   38	local $188 	(nextCel)
		   3a	word	$ff
		   3c	local $1d9 	(cycleDone)

Object: RTRandCycle         
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
		   52	word	$36
		   54	word	$b1
		   56	word	$fb
		   58	word	$e3
		   5a	word	$fc
		   5c	word	$fd
		   5e	word	$261
		   60	word	$265
		(method dictionary)
		   62	word	$0


Method: (RandCycle init)

4;F		   67	file
4;F		   76	pushi	$8e  
		   79	push1
		   7a	lsp  	$1   	(obj)
		   7c	super	$15  	(Cycle)
		   7c	word	$6
4;F		   83	lsp  	$0   	(argc)
		   85	ldi  	$4   
		   87	ge?
		   88	bnt  	$7   	(.0)
4;F		   8d	lap  	$4   	(resetTo0)
		   8f	aTop 	$22  	(reset)
.0
4;F		   94	pToa 	$22  	(reset)
		   96	bnt  	$c   	(.1)
4;F		   9b	pushi	$10  
		   9d	push1
		   9e	push0
		   9f	pToa 	$14  	(client)
		   a1	send
		   a1	word	$6
.1
4;F		   a7	push0
		   a8	callk	$79  	(GetTime)
		   a8	word	$0
		   ac	aTop 	$1a  	(cycleCnt)
4;F		   b1	lsp  	$0   	(argc)
		   b3	ldi  	$2   
		   b5	ge?
		   b6	bnt  	$37  	(.5)
4;F		   bb	lsp  	$2   	(theTime)
		   bd	ldi  	$ffff
		   bf	ne?
		   c0	bnt  	$10  	(.2)
4;F		   c5	push0
		   c6	callk	$79  	(GetTime)
		   c6	word	$0
		   ca	push
		   cb	lap  	$2   	(theTime)
		   cd	add
		   ce	aTop 	$20  	(count)
		   d0	jmp  	$a   	(.3)
.2
4;F4;F		   d8	ldi  	$ffff
		   da	aTop 	$20  	(count)
.3
4;F		   df	lsp  	$0   	(argc)
		   e1	ldi  	$3   
		   e3	ge?
		   e4	bnt  	$13  	(.6)
4;F		   e9	lap  	$3   	(whoCares)
		   eb	aTop 	$16  	(caller)
.4
		   ed	jmp  	$a   	(.6)
.5
4;F4;F		   f5	ldi  	$ffff
		   f7	aTop 	$20  	(count)
.6
4;F		   fc	ret


Method: (RandCycle doit)

4;F		  100	file
		  10c	link 	$1   
4;F4;F4;F		  117	pTos 	$20  	(count)
		  119	push0
		  11a	callk	$79  	(GetTime)
		  11a	word	$0
		  11e	sat  	$0   	(theTime)
		  120	gt?
		  121	bt   	$8   	(.0)
4;F		  126	pTos 	$20  	(count)
		  128	ldi  	$ffff
		  12a	eq?
.0
		  12b	bnt  	$37  	(.2)
4;F4;F		  133	lst  	$0   	(theTime)
		  135	pToa 	$1a  	(cycleCnt)
		  137	sub
		  138	push
		  139	pushi	$102 
		  13c	push0
		  13d	pToa 	$14  	(client)
		  13f	send
		  13f	word	$4
		  142	gt?
		  143	bnt  	$3f  	(.4)
4;F		  148	pushi	$10  
		  14a	push1
		  14b	pushi	$fe  
		  14e	push0
		  14f	self
		  14f	word	$4
		  152	push
		  153	pToa 	$14  	(client)
		  155	send
		  155	word	$6
4;F		  15b	push0
		  15c	callk	$79  	(GetTime)
		  15c	word	$0
		  160	aTop 	$1a  	(cycleCnt)
.1
		  162	jmp  	$20  	(.4)
.2
4;F4;F		  16a	pToa 	$22  	(reset)
		  16c	bnt  	$c   	(.3)
4;F		  171	pushi	$10  
		  173	push1
		  174	push0
		  175	pToa 	$14  	(client)
		  177	send
		  177	word	$6
.3
4;F		  17d	pushi	$ff  
		  180	push0
		  181	self
		  181	word	$4
.4
4;F		  187	ret


Method: (RandCycle nextCel)

4;F		  18b	file
		  197	link 	$1   
4;F4;F		  19f	push1
		  1a0	pTos 	$14  	(client)
		  1a2	callk	$16  	(NumCels)
		  1a2	word	$2
		  1a6	push
		  1a7	ldi  	$1   
		  1a9	ne?
		  1aa	bnt  	$28  	(.2)
4;F.0
		  1af	push2
		  1b0	push0
		  1b1	pushi	$101 
		  1b4	push0
		  1b5	pToa 	$14  	(client)
		  1b7	send
		  1b7	word	$4
		  1ba	push
		  1bb	callk	$3c  	(Random)
		  1bb	word	$4
		  1bf	sat  	$0   	(newCel)
		  1c1	push
		  1c2	pushi	$10  
		  1c4	push0
		  1c5	pToa 	$14  	(client)
		  1c7	send
		  1c7	word	$4
		  1ca	eq?
		  1cb	bnt  	$2   	(.1)
		  1cd	jmp  	$ffe0	(.0)
.1
4;F		  1d2	lat  	$0   	(newCel)
.2
		  1d4	ret
4;F		  1d8	ret


Method: (RandCycle cycleDone)

4;F		  1dc	file
4;F		  1eb	ldi  	$1   
		  1ed	aTop 	$1c  	(completed)
4;F		  1f2	pushi	$100 
		  1f5	push0
		  1f6	self
		  1f6	word	$4
4;F		  1fc	ret
		  1fd	byte	$0


Fixups:
		  1fe	word	$0
