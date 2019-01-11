
				Listing of pchase.sc:	[script 64930]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$52


Variables:
		    2	word	$0



Object: PChase              
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $22  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $4c  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $6b  	(-script-)
		   10	prop  $23  	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $4a  	(name)
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
		   36	prop  $2   	(value)
		   38	prop  $0   	(points)
		   3a	prop  $0   	(finalX)
		   3c	prop  $0   	(finalY)
		   3e	prop  $0   	(obstacles)
		   40	prop  $0   	(who)
		   42	prop  $0   	(distance)
		   44	prop  $0   	(targetX)
		   46	prop  $0   	(targetY)
		   48	word	$0



		   4a	text
"PChase"

		   51	byte	$0


Fixups:
		   52	word	$1
		   54	word	$14




----------------------
-------- Hunk --------
----------------------

		    0	word	$23c
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: PChase              
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
		   3a	word	$51
		   3c	word	$5b
		   3e	word	$10f
		   40	word	$110
		   42	word	$111
		   44	word	$30
		   46	word	$10b
		   48	word	$26d
		   4a	word	$26e
		(method dictionary)
		   4c	word	$3
		   4e	word	$8e
		   50	local $5a  	(init)
		   52	word	$45
		   54	local $111 	(doit)
		   56	word	$64
		   58	local $1a8 	(moveDone)


Method: (PChase init)

=hE		   5d	file
=hE		   6b	lap  	$0   	(argc)
		   6d	bnt  	$8f  	(.6)
=hE=hE		   76	push
		   77	ldi  	$5   
		   79	ge?
		   7a	bnt  	$9   	(.0)
=hE		   7f	lap  	$5   	(theObst)
		   81	aTop 	$3a  	(obstacles)
		   83	jmp  	$16  	(.1)
.0
=hE		   88	pToa 	$3a  	(obstacles)
		   8a	not
		   8b	bnt  	$e   	(.1)
=hE		   90	pushi	$111 
		   93	push0
		   94	lag  	$2   	(curRoom)
		   96	send
		   96	word	$4
		   99	aTop 	$3a  	(obstacles)
.1
=hE		   9e	lsp  	$0   	(argc)
		   a0	ldi  	$1   
		   a2	ge?
		   a3	bnt  	$40  	(.5)
		   a5	lap  	$1   	(actor)
		   a7	aTop 	$14  	(client)
=hE		   ac	lsp  	$0   	(argc)
		   ae	ldi  	$2   
		   b0	ge?
		   b1	bnt  	$32  	(.4)
		   b3	lap  	$2   	(whom)
		   b5	aTop 	$3c  	(who)
		   b7	push1
		   b8	push0
		   b9	pToa 	$3c  	(who)
		   bb	send
		   bb	word	$4
		   be	aTop 	$40  	(targetX)
		   c0	push2
		   c1	push0
		   c2	pToa 	$3c  	(who)
		   c4	send
		   c4	word	$4
		   c7	aTop 	$42  	(targetY)
=hE		   cc	lsp  	$0   	(argc)
		   ce	ldi  	$3   
		   d0	ge?
		   d1	bnt  	$12  	(.3)
		   d3	lap  	$3   	(howClose)
		   d5	aTop 	$3e  	(distance)
=hE		   da	lsp  	$0   	(argc)
		   dc	ldi  	$4   
		   de	ge?
		   df	bnt  	$4   	(.2)
		   e1	lap  	$4   	(whoCares)
		   e3	aTop 	$16  	(caller)
.2
.3
.4
.5
=hE		   e8	pushi	$8e  
		   eb	pushi	$6   
		   ee	pTos 	$14  	(client)
		   f0	pTos 	$40  	(targetX)
		   f2	pTos 	$42  	(targetY)
		   f4	pTos 	$16  	(caller)
		   f6	push1
		   f7	pTos 	$3a  	(obstacles)
		   f9	super	$23  	(PolyPath)
		   f9	word	$10
		   fd	jmp  	$e   	(.7)
.6
=hE=hE		  105	pushi	$8e  
		  108	push0
		  109	super	$23  	(PolyPath)
		  109	word	$4
.7
=hE		  110	ret


Method: (PChase doit)

=hE		  114	file
		  11f	link 	$1   
=hE=hE		  127	pushi	$4   
		  129	pTos 	$40  	(targetX)
		  12b	pTos 	$42  	(targetY)
		  12d	push1
		  12e	push0
		  12f	pToa 	$3c  	(who)
		  131	send
		  131	word	$4
		  134	push
		  135	push2
		  136	push0
		  137	pToa 	$3c  	(who)
		  139	send
		  139	word	$4
		  13c	push
		  13d	callk	$40  	(GetDistance)
		  13d	word	$8
		  141	push
		  142	pToa 	$3e  	(distance)
		  144	gt?
		  145	bnt  	$2f  	(.1)
=hE		  14a	pToa 	$34  	(points)
		  14c	bnt  	$a   	(.0)
=hE		  151	pushi	$8f  
		  154	push0
		  155	send
		  155	word	$4
.0
=hE		  15b	ldi  	$0   
		  15d	aTop 	$34  	(points)
=hE		  162	ldi  	$2   
		  164	aTop 	$32  	(value)
=hE		  169	pushi	$8e  
		  16c	push2
		  16d	pTos 	$14  	(client)
		  16f	pTos 	$3c  	(who)
		  171	self
		  171	word	$8
		  174	jmp  	$2e  	(.3)
.1
=hE		  179	pushi	$10c 
		  17c	push1
		  17d	pTos 	$3c  	(who)
		  17f	pToa 	$14  	(client)
		  181	send
		  181	word	$6
		  184	sat  	$0   	(theDistance)
		  186	push
		  187	pToa 	$3e  	(distance)
		  189	le?
		  18a	bnt  	$b   	(.2)
=hE		  18f	pushi	$64  
		  191	push0
		  192	self
		  192	word	$4
		  195	jmp  	$d   	(.3)
.2
=hE=hE		  19d	pushi	$45  
		  19f	push0
		  1a0	super	$23  	(PolyPath)
		  1a0	word	$4
.3
=hE		  1a7	ret


Method: (PChase moveDone)

=hE		  1ab	file
		  1b6	link 	$1   
=hE=hE		  1be	pushi	$10c 
		  1c1	push1
		  1c2	pTos 	$3c  	(who)
		  1c4	pToa 	$14  	(client)
		  1c6	send
		  1c6	word	$6
		  1c9	sat  	$0   	(theDistance)
		  1cb	push
		  1cc	pToa 	$3e  	(distance)
		  1ce	le?
		  1cf	bnt  	$c   	(.0)
=hE		  1d4	pushi	$64  
		  1d6	push0
		  1d7	super	$23  	(PolyPath)
		  1d7	word	$4
		  1db	jmp  	$5a  	(.3)
.0
=hE		  1e0	pushi	$4b  
		  1e2	push1
		  1e3	pTos 	$32  	(value)
		  1e5	pToa 	$34  	(points)
		  1e7	send
		  1e7	word	$6
		  1ea	push
		  1eb	ldi  	$7777
		  1ee	eq?
		  1ef	bnt  	$2f  	(.2)
=hE		  1f4	pToa 	$34  	(points)
		  1f6	bnt  	$a   	(.1)
=hE		  1fb	pushi	$8f  
		  1fe	push0
		  1ff	send
		  1ff	word	$4
.1
=hE		  205	ldi  	$0   
		  207	aTop 	$34  	(points)
=hE		  20c	ldi  	$2   
		  20e	aTop 	$32  	(value)
=hE		  213	pushi	$8e  
		  216	push2
		  217	pTos 	$14  	(client)
		  219	pTos 	$3c  	(who)
		  21b	self
		  21b	word	$8
		  21e	jmp  	$17  	(.3)
.2
=hE=hE=hE		  229	pushi	$108 
		  22c	push0
=hE		  230	pushi	$8e  
		  233	push0
		  234	self
		  234	word	$8
.3
=hE		  23a	ret
		  23b	byte	$0


Fixups:
		  23c	word	$0
