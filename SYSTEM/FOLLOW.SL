
				Listing of follow.sc:	[script 64971]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$44


Variables:
		    2	word	$0



Object: Follow              
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $1b  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $3e  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $20  	(-script-)
		   10	prop  $1c  	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $3c  	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(client)
		   1a	prop  $0   	(caller)
		   1c	prop  $0   	(x)
		   1e	prop  $0   	(y)
		   20	prop  $0   	(dx)
		   22	prop  $0   	(dy)
		   24	prop  $0   	(b-moveCnt)
		   26	prop  $0   	(b-i1)
		   28	prop  $0   	(b-i2)
		   2a	prop  $0   	(b-di)
		   2c	prop  $0   	(b-xAxis)
		   2e	prop  $0   	(b-incr)
		   30	prop  $0   	(completed)
		   32	prop  $0   	(xLast)
		   34	prop  $0   	(yLast)
		   36	prop  $0   	(who)
		   38	prop  $14  	(distance)
		   3a	word	$0



		   3c	text
"Follow"

		   43	byte	$0


Fixups:
		   44	word	$1
		   46	word	$14




----------------------
-------- Hunk --------
----------------------

		    0	word	$234
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: Follow              
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
		   20	word	$1
		   22	word	$2
		   24	word	$37
		   26	word	$38
		   28	word	$39
		   2a	word	$3a
		   2c	word	$3b
		   2e	word	$3c
		   30	word	$3d
		   32	word	$3e
		   34	word	$fc
		   36	word	$106
		   38	word	$107
		   3a	word	$30
		   3c	word	$10b
		(method dictionary)
		   3e	word	$5
		   40	word	$8e
		   42	local $54  	(init)
		   44	word	$45
		   46	local $13a 	(doit)
		   48	word	$64
		   4a	local $221 	(moveDone)
		   4c	word	$108
		   4e	local $e7  	(setTarget)
		   50	word	$109
		   52	local $c3  	(onTarget)


Method: (Follow init)

		   57	file
		   65	lsp  	$0   	(argc)
		   67	ldi  	$1   
		   69	ge?
		   6a	bnt  	$20  	(.2)
		   6c	lap  	$1   	(theObj)
		   6e	aTop 	$14  	(client)
		   73	lsp  	$0   	(argc)
		   75	ldi  	$2   
		   77	ge?
		   78	bnt  	$12  	(.1)
		   7a	lap  	$2   	(whom)
		   7c	aTop 	$32  	(who)
		   81	lsp  	$0   	(argc)
		   83	ldi  	$3   
		   85	ge?
		   86	bnt  	$4   	(.0)
		   88	lap  	$3   	(dist)
		   8a	aTop 	$34  	(distance)
.0
.1
.2
		   8f	pushi	$10c 
		   92	push1
		   93	pTos 	$32  	(who)
		   95	pToa 	$14  	(client)
		   97	send
		   97	word	$6
		   9a	push
		   9b	pToa 	$34  	(distance)
		   9d	gt?
		   9e	bnt  	$1f  	(.3)
		   a3	pushi	$8e  
		   a6	pushi	$3   
		   a9	pTos 	$14  	(client)
		   ab	push1
		   ac	push0
		   ad	pToa 	$32  	(who)
		   af	send
		   af	word	$4
		   b2	push
		   b3	push2
		   b4	push0
		   b5	pToa 	$32  	(who)
		   b7	send
		   b7	word	$4
		   ba	push
		   bb	super	$1c  	(Motion)
		   bb	word	$a
.3
		   c2	ret


Method: (Follow onTarget)

		   c6	file
		   d4	pushi	$10c 
		   d7	push1
		   d8	pTos 	$32  	(who)
		   da	pToa 	$14  	(client)
		   dc	send
		   dc	word	$6
		   df	push
		   e0	pToa 	$34  	(distance)
		   e2	le?
		   e6	ret


Method: (Follow setTarget)

		   ea	file
		   fb	lap  	$0   	(argc)
		   fd	bnt  	$f   	(.0)
		  102	pushi	$108 
		  105	push0
		  106	&rest	$1   
		  108	super	$1c  	(Motion)
		  108	word	$4
		  10c	jmp  	$28  	(.1)
.0
		  111	pushi	$109 
		  114	push0
		  115	self
		  115	word	$4
		  118	not
		  119	bnt  	$1b  	(.1)
		  11e	pushi	$108 
		  121	push2
		  122	push1
		  123	push0
		  124	pToa 	$32  	(who)
		  126	send
		  126	word	$4
		  129	push
		  12a	push2
		  12b	push0
		  12c	pToa 	$32  	(who)
		  12e	send
		  12e	word	$4
		  131	push
		  132	super	$1c  	(Motion)
		  132	word	$8
.1
		  139	ret


Method: (Follow doit)

		  13d	file
		  148	link 	$1   
		  14d	pushi	$10c 
		  150	push1
		  151	pTos 	$32  	(who)
		  153	pToa 	$14  	(client)
		  155	send
		  155	word	$6
		  158	push
		  159	pToa 	$34  	(distance)
		  15b	gt?
		  15c	bnt  	$36  	(.1)
		  161	pTos 	$20  	(b-moveCnt)
		  163	ldi  	$0   
		  165	eq?
		  166	bnt  	$1f  	(.0)
		  16b	pushi	$8e  
		  16e	pushi	$3   
		  171	pTos 	$14  	(client)
		  173	push1
		  174	push0
		  175	pToa 	$32  	(who)
		  177	send
		  177	word	$4
		  17a	push
		  17b	push2
		  17c	push0
		  17d	pToa 	$32  	(who)
		  17f	send
		  17f	word	$4
		  182	push
		  183	super	$1c  	(Motion)
		  183	word	$a
.0
		  18a	pushi	$45  
		  18c	push0
		  18d	super	$1c  	(Motion)
		  18d	word	$4
		  191	jmp  	$89  	(.5)
.1
		  19a	push1
		  19b	push0
		  19c	pToa 	$14  	(client)
		  19e	send
		  19e	word	$4
		  1a1	aTop 	$2e  	(xLast)
		  1a6	push2
		  1a7	push0
		  1a8	pToa 	$14  	(client)
		  1aa	send
		  1aa	word	$4
		  1ad	aTop 	$30  	(yLast)
		  1b2	pushi	$4   
		  1b4	pTos 	$2e  	(xLast)
		  1b6	pTos 	$30  	(yLast)
		  1b8	push1
		  1b9	push0
		  1ba	pToa 	$32  	(who)
		  1bc	send
		  1bc	word	$4
		  1bf	push
		  1c0	push2
		  1c1	push0
		  1c2	pToa 	$32  	(who)
		  1c4	send
		  1c4	word	$4
		  1c7	push
		  1c8	callk	$3f  	(GetAngle)
		  1c8	word	$8
		  1cc	sat  	$0   	(angle)
		  1d1	push
		  1d2	pushi	$43  
		  1d4	push0
		  1d5	pToa 	$14  	(client)
		  1d7	send
		  1d7	word	$4
		  1da	ne?
		  1db	bnt  	$40  	(.4)
		  1e0	pushi	$43  
		  1e2	push1
		  1e3	lst  	$0   	(angle)
		  1e5	pToa 	$14  	(client)
		  1e7	send
		  1e7	word	$6
		  1ed	pushi	$47  
		  1ef	push0
		  1f0	pToa 	$14  	(client)
		  1f2	send
		  1f2	word	$4
		  1f5	bnt  	$17  	(.2)
		  1fa	pushi	$45  
		  1fc	push2
		  1fd	pTos 	$14  	(client)
		  1ff	lst  	$0   	(angle)
		  201	pushi	$47  
		  203	push0
		  204	pToa 	$14  	(client)
		  206	send
		  206	word	$4
		  209	send
		  209	word	$8
		  20c	jmp  	$f   	(.3)
.2
		  214	push2
		  215	pTos 	$14  	(client)
		  217	lst  	$0   	(angle)
		  219	callk	$5f  	(DirLoop)
		  219	word	$4
.3
.4
.5
		  220	ret


Method: (Follow moveDone)

		  224	file
		  232	ret
		  233	byte	$0


Fixups:
		  234	word	$0
