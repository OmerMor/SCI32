
				Listing of tutorial.sc:	[script 64980]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$44


Variables:
		    2	word	$0



Object: Tutorial            
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $1a  	(-size-)
		    8	ofs   $8   	(-propDict-)
		    a	ofs   $3c  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $12  	(-script-)
		   10	prop  $7   	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $3a  	(name)
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
		   32	prop  $0   	(nextItem)
		   34	prop  $0   	(nextAction)
		   36	prop  $0   	(numTries)
		   38	word	$0



		   3a	text
"Tutorial"

		   43	byte	$0


Fixups:
		   44	word	$1
		   46	word	$14




----------------------
-------- Hunk --------
----------------------

		    0	word	$e6
		    2	word	$0
		    4	word	$0
		    6	word	$0
		(dispatch table)

Object: Tutorial            
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
		   36	word	$e9
		   38	word	$ea
		   3a	word	$eb
		(method dictionary)
		   3c	word	$3
		   3e	word	$b3
		   40	local $be  	(cue)
		   42	word	$ec
		   44	local $4a  	(waitFor)
		   46	word	$ed
		   48	local $aa  	(report)


Method: (Tutorial waitFor)

x		   4d	file
x		   5d	lap  	$1   	(obj)
		   5f	aTop 	$2e  	(nextItem)
x		   64	lap  	$2   	(act)
		   66	aTop 	$30  	(nextAction)
xx		   6e	lsp  	$0   	(argc)
		   70	ldi  	$3   
		   72	eq?
		   73	bnt  	$f   	(.0)
x		   78	push1
		   79	lsp  	$3   	(nOrMsg)
		   7b	calle	$fd99/0	(Prints)
		   7b	word	$2
		   82	jmp  	$22  	(.1)
.0
x		   87	lsp  	$0   	(argc)
		   89	ldi  	$3   
		   8b	gt?
		   8c	bnt  	$18  	(.1)
x		   91	pushi	$ee  
		   94	pushi	$5   
		   97	lsp  	$3   	(nOrMsg)
		   99	lsp  	$4   	(v)
		   9b	lsp  	$5   	(c)
		   9d	lsp  	$6   	(s)
		   9f	lsp  	$7   	(m)
		   a1	lag  	$5b  	(messager)
		   a3	send
		   a3	word	$e
.1
x		   a9	ret


Method: (Tutorial report)

x		   ad	file
x		   bd	ret


Method: (Tutorial cue)

x		   c1	file
x		   d1	ldi  	$0   
		   d3	aTop 	$32  	(numTries)
x		   d8	pushi	$b3  
		   db	push0
		   dc	&rest	$1   
		   de	super	$7   	(Script)
		   de	word	$4
x		   e5	ret


Fixups:
		   e6	word	$0
