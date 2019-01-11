
				Listing of print.sc:	[script 64921]


LINE/	OFFSET	CODE				NAME
LABEL	(HEX)

----------------------
-------- Heap --------
----------------------

		    0	word	$5e


Variables:
		    2	word	$0



Object: Print               
		(properties)
		    4	prop  $1234	(-objID-)
		    6	prop  $22  	(-size-)
		    8	ofs   $10  	(-propDict-)
		    a	ofs   $54  	(-methDict-)
		    c	prop  $0   	(-classScript-)
		    e	prop  $3f  	(-script-)
		   10	prop  $0   	(-super-)
		   12	prop  $8000	(-info-)
		   14	text  $58  	(name)
		   16	prop  $0   	(scratch)
		   18	prop  $0   	(dialog)
		   1a	prop  $0   	(plane)
		   1c	prop  $0   	(title)
		   1e	prop  $0   	(mode)
		   20	prop  $ffff	(font)
		   22	prop  $0   	(width)
		   24	prop  $ff  	(fore)
		   26	prop  $ffff	(back)
		   28	prop  $ffff	(skip)
		   2a	prop  $ffff	(x)
		   2c	prop  $ffff	(y)
		   2e	prop  $4   	(margin)
		   30	prop  $0   	(ticks)
		   32	prop  $0   	(caller)
		   34	prop  $0   	(retValue)
		   36	prop  $0   	(modeless)
		   38	prop  $0   	(first)
		   3a	prop  $0   	(saveCursor)
		   3c	prop  $0   	(bitmap)
		   3e	prop  $1   	(onScreen)
		   40	prop  $0   	(classButton)
		   42	prop  $0   	(classEdit)
		   44	prop  $0   	(classIcon)
		   46	prop  $0   	(classText)
		   48	word	$0



		   4a	text
""

		   4b	text
"*.*"

		   4f	text
"PODialog"

		   58	text
"Print"



Fixups:
		   5e	word	$1
		   60	word	$14




----------------------
-------- Hunk --------
----------------------

		    0	word	$1480
		    2	word	$0
		    4	word	$0
		    6	word	$4
		(dispatch table)
		    8	dispatch	$a6  	(Prints)
		    a	dispatch	$dd  	(Printf)
		    c	dispatch	$114 	(GetInput)
		    e	dispatch	$191 	(FindFormatLen)

Object: Print               
		(property dictionary)
		   10	word	$1000
		   12	word	$1001
		   14	word	$1002
		   16	word	$1003
		   18	word	$1004
		   1a	word	$1005
		   1c	word	$1006
		   1e	word	$1007
		   20	word	$20
		   22	word	$8c
		   24	word	$1a3
		   26	word	$0
		   28	word	$54
		   2a	word	$27
		   2c	word	$2a
		   2e	word	$4e
		   30	word	$25
		   32	word	$26
		   34	word	$72
		   36	word	$1
		   38	word	$2
		   3a	word	$199
		   3c	word	$ad
		   3e	word	$b1
		   40	word	$1a4
		   42	word	$19a
		   44	word	$9c
		   46	word	$1a5
		   48	word	$11
		   4a	word	$19b
		   4c	word	$1a6
		   4e	word	$1a7
		   50	word	$1a8
		   52	word	$1a9
		(method dictionary)
		   54	word	$14
		   56	word	$8e
		   58	local $1023	(init)
		   5a	word	$45
		   5c	local $1069	(doit)
		   5e	word	$8f
		   60	local $f4d 	(dispose)
		   62	word	$1aa
		   64	local $29d 	(addBitmap)
		   66	word	$1ab
		   68	local $334 	(addButton)
		   6a	word	$1ac
		   6c	local $612 	(addButtonBM)
		   6e	word	$1a2
		   70	local $695 	(addEdit)
		   72	word	$1ad
		   74	local $7c8 	(addFSelector)
		   76	word	$1ae
		   78	local $8b6 	(addIcon)
		   7a	word	$1af
		   7c	local $9cf 	(addSelector)
		   7e	word	$1b0
		   80	local $a83 	(addSlider)
		   82	word	$1a0
		   84	local $a9a 	(addText)
		   86	word	$1b1
		   88	local $d85 	(addTextBM)
		   8a	word	$1a1
		   8c	local $e02 	(addTextF)
		   8e	word	$1b2
		   90	local $e60 	(addTitle)
		   92	word	$14c
		   94	local $1004	(posn)
		   96	word	$a6
		   98	local $1085	(handleEvent)
		   9a	word	$b3
		   9c	local $fc8 	(cue)
		   9e	word	$1b3
		   a0	local $10b8	(showSelf)
		   a2	word	$1b4
		   a4	local $1451	(isModeless)


Procedure: (Prints)

’K		   a9	file
		   b3	link 	$1   
’K		   b8	pushi	$8d  
		   bb	push0
		   bc	class	$3f  	(Print)
		   be	send
		   be	word	$4
		   c1	sat  	$0   	(p)
’K’K		   c9	pushi	$1a0 
		   cc	push0
		   cd	&rest	$1   
’K		   d2	pushi	$8e  
		   d5	push0
		   d6	send
		   d6	word	$8
’K		   dc	ret


Procedure: (Printf)

’K		   e0	file
		   ea	link 	$1   
’K		   ef	pushi	$8d  
		   f2	push0
		   f3	class	$3f  	(Print)
		   f5	send
		   f5	word	$4
		   f8	sat  	$0   	(p)
’K’K		  100	pushi	$1a1 
		  103	push0
		  104	&rest	$1   
’K		  109	pushi	$8e  
		  10c	push0
		  10d	send
		  10d	word	$8
’K		  113	ret


Procedure: (GetInput)

’K		  117	file
’K’K’K’K		  12d	pushi	$2a  
		  12f	push1
		  130	lsp  	$0   	(argc)
		  132	ldi  	$3   
		  134	gt?
		  135	bnt  	$4   	(.0)
		  137	lap  	$4   	(theFont)
		  139	jmp  	$2   	(.1)
.0
		  13b	lag  	$16  	(userFont)
.1
		  13d	sag  	$55  	(inputFont)
		  13f	push
’K		  143	pushi	$1a0 
		  146	push1
		  147	lsp  	$0   	(argc)
		  149	ldi  	$2   
		  14b	gt?
		  14c	bnt  	$8   	(.3)
		  14e	lap  	$3   	(prompt)
.2
		  150	bnt  	$4   	(.3)
		  152	lap  	$3   	(prompt)
		  154	jmp  	$3   	(.4)
.3
		  156	lofsa	$4a  
.4
		  159	push
’K		  15d	pushi	$1a2 
		  160	pushi	$5   
		  163	lsp  	$1   	(str)
		  165	lsp  	$2   	(maxLen)
		  167	push0
		  168	pushi	$c   
		  16a	lsp  	$1   	(str)
’K		  16f	pushi	$8e  
		  172	push0
		  173	pushi	$8d  
		  176	push0
		  177	class	$3f  	(Print)
		  179	send
		  179	word	$4
		  17c	send
		  17c	word	$1e
		  17f	bnt  	$b   	(.5)
’K		  184	pushi	$5a  
		  186	push0
		  187	lap  	$1   	(str)
		  189	send
		  189	word	$4
.5
		  18c	ret
’K		  190	ret


Procedure: (FindFormatLen)

’K		  194	file
		  19e	link 	$6   
’K		  1a3	pushi	$ba  
		  1a6	push2
		  1a7	push2
		  1a8	pushi	$a   
		  1aa	push2
		  1ab	pushi	$9   
		  1ad	lsp  	$1   	(ctrlString)
		  1af	callk	$5c  	(KString)
		  1af	word	$4
		  1b3	push
		  1b4	callk	$5c  	(KString)
		  1b4	word	$4
		  1b8	push
		  1b9	lofsa	$4a  
		  1bc	push
		  1bd	class	$f   	(String)
		  1bf	send
		  1bf	word	$8
		  1c2	sat  	$4   	(str)
’K		  1c7	pushi	$bc  
		  1ca	push1
		  1cb	lsp  	$1   	(ctrlString)
		  1cd	send
		  1cd	word	$6
’K		  1d3	pushi	$5a  
		  1d5	push0
		  1d6	lat  	$4   	(str)
		  1d8	send
		  1d8	word	$4
		  1db	sat  	$1   	(len)
		  1dd	sat  	$0   	(finalLen)
’K		  1e2	ldi  	$0   
		  1e4	sat  	$2   	(parmCount)
’K		  1e9	ldi  	$0   
		  1eb	sat  	$3   	(i)
.0
’K		  1f0	lst  	$3   	(i)
		  1f2	lat  	$1   	(len)
		  1f4	lt?
		  1f5	bnt  	$8f  	(.5)
’K’K		  1fe	pushi	$4b  
		  200	push1
		  201	lst  	$3   	(i)
		  203	lat  	$4   	(str)
		  205	send
		  205	word	$6
		  208	push
		  209	ldi  	$25  
		  20b	eq?
		  20c	bnt  	$74  	(.4)
’K		  211	pushi	$4b  
		  213	push1
		  214	+at  	$3   	(i)
		  216	push
		  217	lat  	$4   	(str)
		  219	send
		  219	word	$6
		  21c	push
		  21d	dup
’K		  221	ldi  	$64  
		  223	eq?
		  224	bnt  	$c   	(.1)
’K		  229	lst  	$0   	(finalLen)
		  22b	ldi  	$5   
		  22d	add
		  22e	sat  	$0   	(finalLen)
		  230	jmp  	$4a  	(.3)
.1
		  232	dup
’K		  236	ldi  	$78  
		  238	eq?
		  239	bnt  	$c   	(.2)
’K		  23e	lst  	$0   	(finalLen)
		  240	ldi  	$4   
		  242	add
		  243	sat  	$0   	(finalLen)
		  245	jmp  	$35  	(.3)
.2
		  247	dup
’K		  24b	ldi  	$73  
		  24d	eq?
		  24e	bnt  	$2c  	(.3)
’K		  253	pushi	$b9  
		  256	push1
		  257	lat  	$2   	(parmCount)
		  259	lspi 	$2   	()
		  25b	class	$f   	(String)
		  25d	send
		  25d	word	$6
		  260	sat  	$5   	(argStr)
’K		  265	lst  	$0   	(finalLen)
		  267	pushi	$5a  
		  269	push0
		  26a	send
		  26a	word	$4
		  26d	add
		  26e	sat  	$0   	(finalLen)
’K		  273	pushi	$8f  
		  276	push0
		  277	lat  	$5   	(argStr)
		  279	send
		  279	word	$4
.3
		  27c	toss
’K		  280	+at  	$2   	(parmCount)
.4
		  282	+at  	$3   	(i)
		  284	jmp  	$ff66	(.0)
.5
’K		  28a	pushi	$8f  
		  28d	push0
		  28e	lat  	$4   	(str)
		  290	send
		  290	word	$4
’K		  296	+at  	$0   	(finalLen)
		  298	ret
’K		  29c	ret


Method: (Print addBitmap)

’K		  2a0	file
’K		  2ad	pToa 	$14  	(dialog)
		  2af	not
		  2b0	bnt  	$e   	(.0)
’K		  2b5	pushi	$8d  
		  2b8	push0
		  2b9	class	$3d  	(Dialog)
		  2bb	send
		  2bb	word	$4
		  2be	aTop 	$14  	(dialog)
.0
’K		  2c3	pushi	$8d  
		  2c6	push0
		  2c7	class	$38  	(DIcon)
		  2c9	send
		  2c9	word	$4
		  2cc	aTop 	$38  	(bitmap)
’K		  2d1	pushi	$96  
		  2d4	push1
’K’K		  2db	pushi	$e   
		  2dd	push1
		  2de	lsp  	$1   	(v)
’K		  2e3	pushi	$f   
		  2e5	push1
		  2e6	lsp  	$2   	(l)
’K		  2eb	pushi	$10  
		  2ed	push1
		  2ee	lsp  	$3   	(c)
’K		  2f3	pushi	$177 
		  2f6	push0
’K		  2fa	pushi	$c3  
		  2fd	push2
		  2fe	push0
		  2ff	push0
’K		  303	pushi	$94  
		  306	push0
		  307	pToa 	$38  	(bitmap)
		  309	send
		  309	word	$22
		  30c	push
		  30d	pToa 	$14  	(dialog)
		  30f	send
		  30f	word	$6
’K		  315	pushi	$3   
		  317	lsp  	$1   	(v)
		  319	lsp  	$2   	(l)
		  31b	lsp  	$3   	(c)
		  31d	callk	$1c  	(CelWide)
		  31d	word	$6
		  321	push
		  322	pTos 	$2a  	(margin)
		  324	ldi  	$2   
		  326	mul
		  327	sub
		  328	aTop 	$1e  	(width)
’K		  32d	pToa 	$38  	(bitmap)
		  32f	ret
’K		  333	ret


Method: (Print addButton)

’K		  337	file
		  341	link 	$a   
’K’K		  349	pToa 	$14  	(dialog)
		  34b	not
		  34c	bnt  	$e   	(.0)
’K		  351	pushi	$8d  
		  354	push0
		  355	class	$3d  	(Dialog)
		  357	send
		  357	word	$4
		  35a	aTop 	$14  	(dialog)
.0
’K		  35f	pTos 	$1c  	(font)
		  361	ldi  	$ffff
		  363	eq?
		  364	bnt  	$7   	(.1)
’K		  369	lag  	$16  	(userFont)
		  36b	aTop 	$1c  	(font)
.1
’K’K		  373	pToa 	$3c  	(classButton)
		  375	bnt  	$5   	(.2)
’K		  37a	jmp  	$8   	(.3)
.2
’K’K		  382	class	$36  	(DButton)
.3
		  384	sat  	$9   	(buttonClass)
’K		  389	lsp  	$0   	(argc)
		  38b	ldi  	$4   
		  38d	gt?
		  38e	bnt  	$18f 	(.15)
’K		  394	ldi  	$0   
		  396	lapi 	$2   	()
		  398	sat  	$0   	(noun)
’K		  39d	ldi  	$1   
		  39f	lapi 	$2   	()
		  3a1	sat  	$1   	(verb)
’K		  3a6	ldi  	$2   
		  3a8	lapi 	$2   	()
		  3aa	sat  	$2   	(case)
’K		  3af	ldi  	$3   
		  3b1	lapi 	$2   	()
		  3b3	bnt  	$6   	(.4)
		  3b5	ldi  	$3   
		  3b7	lapi 	$2   	()
		  3b9	jmp  	$2   	(.5)
.4
		  3bb	ldi  	$1   
.5
		  3bd	sat  	$3   	(seq)
’K		  3c2	ldi  	$0   
		  3c4	sat  	$4   	(theX)
’K		  3c9	ldi  	$0   
		  3cb	sat  	$5   	(theY)
’K		  3d0	lag  	$b   	(curRoomNum)
		  3d2	sat  	$6   	(module)
’K		  3d7	lsp  	$0   	(argc)
		  3d9	ldi  	$5   
		  3db	gt?
		  3dc	bnt  	$2f  	(.8)
’K		  3e1	ldi  	$4   
		  3e3	lapi 	$2   	()
		  3e5	sat  	$4   	(theX)
’K		  3ea	lsp  	$0   	(argc)
		  3ec	ldi  	$6   
		  3ee	gt?
		  3ef	bnt  	$1c  	(.7)
’K		  3f4	ldi  	$5   
		  3f6	lapi 	$2   	()
		  3f8	sat  	$5   	(theY)
’K		  3fd	lsp  	$0   	(argc)
		  3ff	ldi  	$7   
		  401	gt?
		  402	bnt  	$9   	(.6)
’K		  407	ldi  	$6   
		  409	lapi 	$2   	()
		  40b	sat  	$6   	(module)
.6
.7
.8
’K		  410	pushi	$8d  
		  413	push0
		  414	class	$f   	(String)
		  416	send
		  416	word	$4
		  419	sat  	$7   	(theText)
’K		  41e	pushi	$6   
		  420	push2
		  421	lst  	$6   	(module)
		  423	lst  	$0   	(noun)
		  425	lst  	$1   	(verb)
		  427	lst  	$2   	(case)
		  429	lst  	$3   	(seq)
		  42b	callk	$48  	(Message)
		  42b	word	$c
		  42f	not
		  430	bt   	$1e  	(.9)
’K		  435	pushi	$7   
		  437	push0
		  438	lst  	$6   	(module)
		  43a	lst  	$0   	(noun)
		  43c	lst  	$1   	(verb)
		  43e	lst  	$2   	(case)
		  440	lst  	$3   	(seq)
		  442	pushi	$71  
		  444	push0
		  445	lat  	$7   	(theText)
		  447	send
		  447	word	$4
		  44a	push
		  44b	callk	$48  	(Message)
		  44b	word	$e
		  44f	not
.9
		  450	bnt  	$15  	(.10)
’K’K		  458	pushi	$8f  
		  45b	push0
		  45c	lat  	$7   	(theText)
		  45e	send
		  45e	word	$4
’K		  464	ldi  	$0   
		  466	ret
.10
’K’K		  46d	pushi	$96  
		  470	push1
’K’K’K		  47a	pushi	$51  
		  47c	push1
		  47d	lsp  	$1   	(val)
’K		  482	pushi	$2a  
		  484	push1
		  485	pTos 	$1c  	(font)
’K		  48a	pushi	$25  
		  48c	push1
		  48d	pTos 	$20  	(fore)
’K		  492	pushi	$26  
		  494	push1
		  495	pTos 	$22  	(back)
		  497	ldi  	$ffff
		  499	eq?
		  49a	bnt  	$a   	(.11)
		  49c	pushi	$26  
		  49e	push0
		  49f	lat  	$9   	(buttonClass)
		  4a1	send
		  4a1	word	$4
		  4a4	jmp  	$2   	(.12)
.11
		  4a6	pToa 	$22  	(back)
.12
		  4a8	push
’K		  4ac	pushi	$72  
		  4ae	push1
		  4af	pTos 	$24  	(skip)
		  4b1	ldi  	$ffff
		  4b3	eq?
		  4b4	bnt  	$a   	(.13)
		  4b6	pushi	$72  
		  4b8	push0
		  4b9	lat  	$9   	(buttonClass)
		  4bb	send
		  4bb	word	$4
		  4be	jmp  	$2   	(.14)
.13
		  4c0	pToa 	$24  	(skip)
.14
		  4c2	push
’K		  4c6	pushi	$23  
		  4c8	push1
		  4c9	pushi	$71  
		  4cb	push0
		  4cc	lat  	$7   	(theText)
		  4ce	send
		  4ce	word	$4
		  4d1	push
’K		  4d5	pushi	$177 
		  4d8	push0
’K		  4dc	pushi	$c3  
		  4df	push2
		  4e0	pTos 	$2a  	(margin)
		  4e2	lat  	$4   	(theX)
		  4e4	add
		  4e5	push
		  4e6	pTos 	$2a  	(margin)
		  4e8	lat  	$5   	(theY)
		  4ea	add
		  4eb	push
’K		  4ef	pushi	$94  
		  4f2	push0
		  4f3	pushi	$8d  
		  4f6	push0
		  4f7	lat  	$9   	(buttonClass)
		  4f9	send
		  4f9	word	$4
		  4fc	send
		  4fc	word	$34
		  4ff	sat  	$8   	(ret)
		  501	push
		  502	pToa 	$14  	(dialog)
		  504	send
		  504	word	$6
’K’K		  50d	pushi	$71  
		  50f	push1
		  510	push0
’K		  514	pushi	$8f  
		  517	push0
		  518	lat  	$7   	(theText)
		  51a	send
		  51a	word	$a
		  51d	jmp  	$e8  	(.22)
.15
’K’K		  526	ldi  	$0   
		  528	sat  	$4   	(theX)
’K		  52d	ldi  	$0   
		  52f	sat  	$5   	(theY)
’K		  534	lsp  	$0   	(argc)
		  536	ldi  	$2   
		  538	gt?
		  539	bnt  	$1c  	(.17)
’K		  53e	ldi  	$1   
		  540	lapi 	$2   	()
		  542	sat  	$4   	(theX)
’K		  547	lsp  	$0   	(argc)
		  549	ldi  	$3   
		  54b	gt?
		  54c	bnt  	$9   	(.16)
’K		  551	ldi  	$2   
		  553	lapi 	$2   	()
		  555	sat  	$5   	(theY)
.16
.17
’K		  55a	push2
		  55b	pushi	$8   
		  55d	push2
		  55e	pushi	$9   
		  560	ldi  	$0   
		  562	lspi 	$2   	()
		  564	callk	$5c  	(KString)
		  564	word	$4
		  568	push
		  569	callk	$5c  	(KString)
		  569	word	$4
		  56d	sat  	$7   	(theText)
’K’K		  575	pushi	$96  
		  578	push1
’K’K’K		  582	pushi	$51  
		  584	push1
		  585	lsp  	$1   	(val)
’K		  58a	pushi	$2a  
		  58c	push1
		  58d	pTos 	$1c  	(font)
’K		  592	pushi	$25  
		  594	push1
		  595	pTos 	$20  	(fore)
’K		  59a	pushi	$26  
		  59c	push1
		  59d	pTos 	$22  	(back)
		  59f	ldi  	$ffff
		  5a1	eq?
		  5a2	bnt  	$a   	(.18)
		  5a4	pushi	$26  
		  5a6	push0
		  5a7	lat  	$9   	(buttonClass)
		  5a9	send
		  5a9	word	$4
		  5ac	jmp  	$2   	(.19)
.18
		  5ae	pToa 	$22  	(back)
.19
		  5b0	push
’K		  5b4	pushi	$72  
		  5b6	push1
		  5b7	pTos 	$24  	(skip)
		  5b9	ldi  	$ffff
		  5bb	eq?
		  5bc	bnt  	$a   	(.20)
		  5be	pushi	$72  
		  5c0	push0
		  5c1	lat  	$9   	(buttonClass)
		  5c3	send
		  5c3	word	$4
		  5c6	jmp  	$2   	(.21)
.20
		  5c8	pToa 	$24  	(skip)
.21
		  5ca	push
’K		  5ce	pushi	$23  
		  5d0	push1
		  5d1	lst  	$7   	(theText)
’K		  5d6	pushi	$177 
		  5d9	push0
’K		  5dd	pushi	$c3  
		  5e0	push2
		  5e1	pTos 	$2a  	(margin)
		  5e3	lat  	$4   	(theX)
		  5e5	add
		  5e6	push
		  5e7	pTos 	$2a  	(margin)
		  5e9	lat  	$5   	(theY)
		  5eb	add
		  5ec	push
’K		  5f0	pushi	$94  
		  5f3	push0
		  5f4	pushi	$8d  
		  5f7	push0
		  5f8	lat  	$9   	(buttonClass)
		  5fa	send
		  5fa	word	$4
		  5fd	send
		  5fd	word	$34
		  600	sat  	$8   	(ret)
		  602	push
		  603	pToa 	$14  	(dialog)
		  605	send
		  605	word	$6
.22
’K		  60b	lat  	$8   	(ret)
		  60d	ret
’K		  611	ret


Method: (Print addButtonBM)

’K		  615	file
		  61f	link 	$1   
’K		  624	pushi	$1ab 
		  627	push0
		  628	&rest	$4   
		  62a	self
		  62a	word	$4
		  62d	sat  	$0   	(theButton)
’K		  632	lsp  	$1   	(v)
		  634	ldi  	$ffff
		  636	ne?
		  637	bnt  	$29  	(.0)
’K’K		  63f	pushi	$e   
		  641	push1
		  642	lsp  	$1   	(v)
’K		  647	pushi	$f   
		  649	push1
		  64a	lsp  	$2   	(l)
’K		  64f	pushi	$10  
		  651	push1
		  652	lsp  	$3   	(c)
’K		  657	pushi	$177 
		  65a	push0
		  65b	lat  	$0   	(theButton)
		  65d	send
		  65d	word	$16
		  660	jmp  	$29  	(.1)
.0
’K’K’K		  66b	pushi	$e   
		  66d	push1
		  66e	pushi	$fdde
’K		  674	pushi	$f   
		  676	push1
		  677	push0
’K		  67b	pushi	$10  
		  67d	push1
		  67e	push0
’K		  682	pushi	$177 
		  685	push0
		  686	lat  	$0   	(theButton)
		  688	send
		  688	word	$16
.1
’K		  68e	lat  	$0   	(theButton)
		  690	ret
’K		  694	ret


Method: (Print addEdit)

’K		  698	file
		  6a2	link 	$4   
’K		  6a7	pToa 	$14  	(dialog)
		  6a9	not
		  6aa	bnt  	$e   	(.0)
’K		  6af	pushi	$8d  
		  6b2	push0
		  6b3	class	$3d  	(Dialog)
		  6b5	send
		  6b5	word	$4
		  6b8	aTop 	$14  	(dialog)
.0
’K		  6bd	pTos 	$1c  	(font)
		  6bf	ldi  	$ffff
		  6c1	eq?
		  6c2	bnt  	$7   	(.1)
’K		  6c7	lag  	$55  	(inputFont)
		  6c9	aTop 	$1c  	(font)
.1
’K’K		  6d1	pToa 	$3e  	(classEdit)
		  6d3	bnt  	$5   	(.2)
’K		  6d8	jmp  	$8   	(.3)
.2
’K’K		  6e0	class	$37  	(DEdit)
.3
		  6e2	sat  	$3   	(editClass)
’K		  6e7	lsp  	$0   	(argc)
		  6e9	ldi  	$4   
		  6eb	gt?
		  6ec	bnt  	$e   	(.4)
’K		  6f1	pushi	$bc  
		  6f4	push1
		  6f5	lsp  	$5   	(default)
		  6f7	lap  	$1   	(buf)
		  6f9	send
		  6f9	word	$6
.4
’K		  6ff	lsp  	$0   	(argc)
		  701	ldi  	$2   
		  703	gt?
		  704	bnt  	$18  	(.6)
’K		  709	lap  	$3   	(theX)
		  70b	sat  	$0   	(eX)
’K		  710	lsp  	$0   	(argc)
		  712	ldi  	$3   
		  714	gt?
		  715	bnt  	$7   	(.5)
’K		  71a	lap  	$4   	(theY)
		  71c	sat  	$1   	(eY)
.5
.6
’K’K		  724	pushi	$96  
		  727	push1
’K’K’K		  731	pushi	$23  
		  733	push1
		  734	pushi	$71  
		  736	push0
		  737	lap  	$1   	(buf)
		  739	send
		  739	word	$4
		  73c	push
’K		  740	pushi	$4e  
		  742	push1
		  743	lsp  	$2   	(maxSize)
’K		  748	pushi	$25  
		  74a	push1
		  74b	pTos 	$20  	(fore)
’K		  750	pushi	$2a  
		  752	push1
		  753	pTos 	$1c  	(font)
’K		  758	pushi	$26  
		  75a	push1
		  75b	pTos 	$22  	(back)
		  75d	ldi  	$ffff
		  75f	eq?
		  760	bnt  	$a   	(.7)
		  762	pushi	$26  
		  764	push0
		  765	lat  	$3   	(editClass)
		  767	send
		  767	word	$4
		  76a	jmp  	$2   	(.8)
.7
		  76c	pToa 	$22  	(back)
.8
		  76e	push
’K		  772	pushi	$72  
		  774	push1
		  775	pTos 	$24  	(skip)
		  777	ldi  	$ffff
		  779	eq?
		  77a	bnt  	$a   	(.9)
		  77c	pushi	$72  
		  77e	push0
		  77f	lat  	$3   	(editClass)
		  781	send
		  781	word	$4
		  784	jmp  	$2   	(.10)
.9
		  786	pToa 	$24  	(skip)
.10
		  788	push
’K		  78c	pushi	$177 
		  78f	push0
’K		  793	pushi	$c3  
		  796	push2
		  797	lst  	$0   	(eX)
		  799	pToa 	$2a  	(margin)
		  79b	add
		  79c	push
		  79d	lst  	$1   	(eY)
		  79f	pToa 	$2a  	(margin)
		  7a1	add
		  7a2	push
’K		  7a6	pushi	$94  
		  7a9	push0
		  7aa	pushi	$8d  
		  7ad	push0
		  7ae	lat  	$3   	(editClass)
		  7b0	send
		  7b0	word	$4
		  7b3	send
		  7b3	word	$34
		  7b6	sat  	$2   	(ret)
		  7b8	push
		  7b9	pToa 	$14  	(dialog)
		  7bb	send
		  7bb	word	$6
’K		  7c1	lat  	$2   	(ret)
		  7c3	ret
’K		  7c7	ret


Method: (Print addFSelector)

’K		  7cb	file
		  7d5	link 	$2   
’K		  7da	pToa 	$14  	(dialog)
		  7dc	not
		  7dd	bnt  	$e   	(.0)
’K		  7e2	pushi	$8d  
		  7e5	push0
		  7e6	class	$3d  	(Dialog)
		  7e8	send
		  7e8	word	$4
		  7eb	aTop 	$14  	(dialog)
.0
’K		  7f0	pTos 	$1c  	(font)
		  7f2	ldi  	$ffff
		  7f4	eq?
		  7f5	bnt  	$7   	(.1)
’K		  7fa	lag  	$16  	(userFont)
		  7fc	aTop 	$1c  	(font)
.1
’K’K		  804	pushi	$2a  
		  806	push1
		  807	pTos 	$1c  	(font)
		  809	pushi	$8d  
		  80c	push0
		  80d	class	$3c  	(FileSelector)
		  80f	send
		  80f	word	$4
		  812	sat  	$1   	(d)
		  814	send
		  814	word	$6
’K		  81a	pushi	$96  
		  81d	push1
’K’K’K		  827	pushi	$18e 
		  82a	push0
’K		  82e	pushi	$192 
		  831	push1
		  832	lsp  	$0   	(argc)
		  834	ldi  	$3   
		  836	gt?
		  837	bnt  	$16  	(.2)
’K		  83c	push2
		  83d	pushi	$8   
		  83f	push2
		  840	pushi	$9   
		  842	lsp  	$4   	(theMask)
		  844	callk	$5c  	(KString)
		  844	word	$4
		  848	push
		  849	callk	$5c  	(KString)
		  849	word	$4
		  84d	jmp  	$11  	(.3)
.2
’K’K		  855	push2
		  856	pushi	$8   
		  858	lofsa	$4b  
		  85b	push
		  85c	callk	$5c  	(KString)
		  85c	word	$4
.3
		  860	push
’K		  864	pushi	$a5  
		  867	push1
		  868	push0
’K		  86c	pushi	$185 
		  86f	push1
		  870	lsp  	$3   	(maxLen)
		  872	ldi  	$ffff
		  874	ne?
		  875	bnt  	$4   	(.4)
		  877	lap  	$3   	(maxLen)
		  879	jmp  	$2   	(.5)
.4
		  87b	ldi  	$0   
.5
		  87d	push
’K		  881	pushi	$177 
		  884	push0
’K		  888	pushi	$c3  
		  88b	push2
		  88c	lsp  	$1   	(theX)
		  88e	pToa 	$2a  	(margin)
		  890	add
		  891	push
		  892	lsp  	$2   	(theY)
		  894	pToa 	$2a  	(margin)
		  896	add
		  897	push
’K		  89b	pushi	$94  
		  89e	push0
		  89f	lat  	$1   	(d)
		  8a1	send
		  8a1	word	$26
		  8a4	sat  	$0   	(ret)
		  8a6	push
		  8a7	pToa 	$14  	(dialog)
		  8a9	send
		  8a9	word	$6
’K		  8af	lat  	$0   	(ret)
		  8b1	ret
’K		  8b5	ret


Method: (Print addIcon)

’K		  8b9	file
		  8c3	link 	$4   
’K		  8c8	pToa 	$14  	(dialog)
		  8ca	not
		  8cb	bnt  	$e   	(.0)
’K		  8d0	pushi	$8d  
		  8d3	push0
		  8d4	class	$3d  	(Dialog)
		  8d6	send
		  8d6	word	$4
		  8d9	aTop 	$14  	(dialog)
.0
’K		  8de	lsp  	$0   	(argc)
		  8e0	ldi  	$3   
		  8e2	gt?
		  8e3	bnt  	$10  	(.1)
’K		  8e8	lap  	$4   	(theX)
		  8ea	sat  	$0   	(iX)
’K		  8ef	lap  	$5   	(theY)
		  8f1	sat  	$1   	(iY)
		  8f3	jmp  	$c   	(.2)
.1
’K’K		  8fb	ldi  	$0   
		  8fd	sat  	$1   	(iY)
		  8ff	sat  	$0   	(iX)
.2
’K’K		  907	pToa 	$40  	(classIcon)
		  909	bnt  	$5   	(.3)
’K		  90e	jmp  	$8   	(.4)
.3
’K’K		  916	class	$38  	(DIcon)
.4
		  918	sat  	$3   	(iconClass)
’K		  91d	lsp  	$2   	(theLoop)
		  91f	ldi  	$ffff
		  921	eq?
		  922	bnt  	$41  	(.5)
’K’K		  92a	pushi	$96  
		  92d	push1
’K’K’K		  937	pushi	$177 
		  93a	push0
’K		  93e	pushi	$c3  
		  941	push2
		  942	lst  	$0   	(iX)
		  944	pToa 	$2a  	(margin)
		  946	add
		  947	push
		  948	lst  	$1   	(iY)
		  94a	pToa 	$2a  	(margin)
		  94c	add
		  94d	push
’K		  951	pushi	$94  
		  954	push0
		  955	lap  	$1   	(theViewOrObj)
		  957	send
		  957	word	$10
		  95a	sat  	$2   	(ret)
		  95c	push
		  95d	pToa 	$14  	(dialog)
		  95f	send
		  95f	word	$6
		  962	jmp  	$60  	(.6)
.5
’K’K’K		  96e	pushi	$96  
		  971	push1
’K’K’K		  97b	pushi	$e   
		  97d	push1
		  97e	lsp  	$1   	(theViewOrObj)
’K		  983	pushi	$f   
		  985	push1
		  986	lsp  	$2   	(theLoop)
’K		  98b	pushi	$10  
		  98d	push1
		  98e	lsp  	$3   	(theCel)
’K		  993	pushi	$177 
		  996	push0
’K		  99a	pushi	$c3  
		  99d	push2
		  99e	lst  	$0   	(iX)
		  9a0	pToa 	$2a  	(margin)
		  9a2	add
		  9a3	push
		  9a4	lst  	$1   	(iY)
		  9a6	pToa 	$2a  	(margin)
		  9a8	add
		  9a9	push
’K		  9ad	pushi	$94  
		  9b0	push0
		  9b1	pushi	$8d  
		  9b4	push0
		  9b5	lat  	$3   	(iconClass)
		  9b7	send
		  9b7	word	$4
		  9ba	send
		  9ba	word	$22
		  9bd	sat  	$2   	(ret)
		  9bf	push
		  9c0	pToa 	$14  	(dialog)
		  9c2	send
		  9c2	word	$6
.6
’K		  9c8	lat  	$2   	(ret)
		  9ca	ret
’K		  9ce	ret


Method: (Print addSelector)

’K		  9d2	file
		  9dc	link 	$2   
’K		  9e1	pToa 	$14  	(dialog)
		  9e3	not
		  9e4	bnt  	$e   	(.0)
’K		  9e9	pushi	$8d  
		  9ec	push0
		  9ed	class	$3d  	(Dialog)
		  9ef	send
		  9ef	word	$4
		  9f2	aTop 	$14  	(dialog)
.0
’K		  9f7	pTos 	$1c  	(font)
		  9f9	ldi  	$ffff
		  9fb	eq?
		  9fc	bnt  	$7   	(.1)
’K		  a01	lag  	$16  	(userFont)
		  a03	aTop 	$1c  	(font)
.1
’K’K		  a0b	pushi	$2a  
		  a0d	push1
		  a0e	pTos 	$1c  	(font)
		  a10	pushi	$8d  
		  a13	push0
		  a14	class	$3a  	(DSelector)
		  a16	send
		  a16	word	$4
		  a19	sat  	$1   	(d)
		  a1b	send
		  a1b	word	$6
’K		  a21	pushi	$96  
		  a24	push1
’K’K’K		  a2e	pushi	$18e 
		  a31	push1
		  a32	lsp  	$4   	(theText)
		  a34	&rest	$5   
’K		  a39	pushi	$185 
		  a3c	push1
		  a3d	lsp  	$3   	(maxLen)
		  a3f	ldi  	$ffff
		  a41	ne?
		  a42	bnt  	$4   	(.2)
		  a44	lap  	$3   	(maxLen)
		  a46	jmp  	$2   	(.3)
.2
		  a48	ldi  	$0   
.3
		  a4a	push
’K		  a4e	pushi	$177 
		  a51	push0
’K		  a55	pushi	$c3  
		  a58	push2
		  a59	lsp  	$1   	(theX)
		  a5b	pToa 	$2a  	(margin)
		  a5d	add
		  a5e	push
		  a5f	lsp  	$2   	(theY)
		  a61	pToa 	$2a  	(margin)
		  a63	add
		  a64	push
’K		  a68	pushi	$94  
		  a6b	push0
		  a6c	lat  	$1   	(d)
		  a6e	send
		  a6e	word	$1c
		  a71	sat  	$0   	(ret)
		  a73	push
		  a74	pToa 	$14  	(dialog)
		  a76	send
		  a76	word	$6
’K		  a7c	lat  	$0   	(ret)
		  a7e	ret
’K		  a82	ret


Method: (Print addSlider)

’K		  a86	file
’K		  a93	ldi  	$0   
		  a95	ret
’K		  a99	ret


Method: (Print addText)

’K		  a9d	file
		  aa7	link 	$c   
’K’K		  aaf	pToa 	$14  	(dialog)
		  ab1	not
		  ab2	bnt  	$e   	(.0)
’K		  ab7	pushi	$8d  
		  aba	push0
		  abb	class	$3d  	(Dialog)
		  abd	send
		  abd	word	$4
		  ac0	aTop 	$14  	(dialog)
.0
’K		  ac5	pTos 	$1c  	(font)
		  ac7	ldi  	$ffff
		  ac9	eq?
		  aca	bnt  	$7   	(.1)
’K		  acf	lag  	$16  	(userFont)
		  ad1	aTop 	$1c  	(font)
.1
’K’K		  ad9	pToa 	$42  	(classText)
		  adb	bnt  	$5   	(.2)
’K		  ae0	jmp  	$8   	(.3)
.2
’K’K		  ae8	class	$35  	(DText)
.3
		  aea	sat  	$b   	(textClass)
’K		  aef	lsp  	$0   	(argc)
		  af1	ldi  	$3   
		  af3	gt?
		  af4	bnt  	$16f 	(.15)
’K		  afa	ldi  	$0   
		  afc	lapi 	$1   	()
		  afe	sat  	$0   	(noun)
’K		  b03	ldi  	$1   
		  b05	lapi 	$1   	()
		  b07	sat  	$1   	(verb)
’K		  b0c	ldi  	$2   
		  b0e	lapi 	$1   	()
		  b10	sat  	$2   	(case)
’K		  b15	ldi  	$3   
		  b17	lapi 	$1   	()
		  b19	bnt  	$6   	(.4)
		  b1b	ldi  	$3   
		  b1d	lapi 	$1   	()
		  b1f	jmp  	$2   	(.5)
.4
		  b21	ldi  	$1   
.5
		  b23	sat  	$3   	(seq)
’K		  b28	ldi  	$0   
		  b2a	sat  	$4   	(theX)
’K		  b2f	ldi  	$0   
		  b31	sat  	$5   	(theY)
’K		  b36	lag  	$b   	(curRoomNum)
		  b38	sat  	$6   	(module)
’K		  b3d	lsp  	$0   	(argc)
		  b3f	ldi  	$5   
		  b41	ge?
		  b42	bnt  	$2f  	(.8)
’K		  b47	ldi  	$4   
		  b49	lapi 	$1   	()
		  b4b	sat  	$4   	(theX)
’K		  b50	lsp  	$0   	(argc)
		  b52	ldi  	$6   
		  b54	ge?
		  b55	bnt  	$1c  	(.7)
’K		  b5a	ldi  	$5   
		  b5c	lapi 	$1   	()
		  b5e	sat  	$5   	(theY)
’K		  b63	lsp  	$0   	(argc)
		  b65	ldi  	$7   
		  b67	ge?
		  b68	bnt  	$9   	(.6)
’K		  b6d	ldi  	$6   
		  b6f	lapi 	$1   	()
		  b71	sat  	$6   	(module)
.6
.7
.8
’K		  b76	pushi	$8d  
		  b79	push0
		  b7a	class	$f   	(String)
		  b7c	send
		  b7c	word	$4
		  b7f	sat  	$7   	(theText)
’K		  b84	pushi	$6   
		  b86	push2
		  b87	lst  	$6   	(module)
		  b89	lst  	$0   	(noun)
		  b8b	lst  	$1   	(verb)
		  b8d	lst  	$2   	(case)
		  b8f	lst  	$3   	(seq)
		  b91	callk	$48  	(Message)
		  b91	word	$c
		  b95	not
		  b96	bt   	$1e  	(.9)
’K		  b9b	pushi	$7   
		  b9d	push0
		  b9e	lst  	$6   	(module)
		  ba0	lst  	$0   	(noun)
		  ba2	lst  	$1   	(verb)
		  ba4	lst  	$2   	(case)
		  ba6	lst  	$3   	(seq)
		  ba8	pushi	$71  
		  baa	push0
		  bab	lat  	$7   	(theText)
		  bad	send
		  bad	word	$4
		  bb0	push
		  bb1	callk	$48  	(Message)
		  bb1	word	$e
		  bb5	not
.9
		  bb6	bnt  	$9   	(.10)
’K’K		  bbe	ldi  	$0   
		  bc0	ret
.10
’K’K		  bc7	pushi	$96  
		  bca	push1
’K’K’K		  bd4	pushi	$23  
		  bd6	push1
		  bd7	pushi	$71  
		  bd9	push0
		  bda	lat  	$7   	(theText)
		  bdc	send
		  bdc	word	$4
		  bdf	push
’K		  be3	pushi	$2a  
		  be5	push1
		  be6	pTos 	$1c  	(font)
’K		  beb	pushi	$27  
		  bed	push1
		  bee	pTos 	$1a  	(mode)
’K		  bf3	pushi	$25  
		  bf5	push1
		  bf6	pTos 	$20  	(fore)
’K		  bfb	pushi	$26  
		  bfd	push1
		  bfe	pTos 	$22  	(back)
		  c00	ldi  	$ffff
		  c02	eq?
		  c03	bnt  	$a   	(.11)
		  c05	pushi	$26  
		  c07	push0
		  c08	lat  	$b   	(textClass)
		  c0a	send
		  c0a	word	$4
		  c0d	jmp  	$2   	(.12)
.11
		  c0f	pToa 	$22  	(back)
.12
		  c11	push
’K		  c15	pushi	$72  
		  c17	push1
		  c18	pTos 	$24  	(skip)
		  c1a	ldi  	$ffff
		  c1c	eq?
		  c1d	bnt  	$a   	(.13)
		  c1f	pushi	$72  
		  c21	push0
		  c22	lat  	$b   	(textClass)
		  c24	send
		  c24	word	$4
		  c27	jmp  	$2   	(.14)
.13
		  c29	pToa 	$24  	(skip)
.14
		  c2b	push
’K		  c2f	pushi	$177 
		  c32	push1
		  c33	pTos 	$1e  	(width)
’K		  c38	pushi	$c3  
		  c3b	push2
		  c3c	pTos 	$2a  	(margin)
		  c3e	lat  	$4   	(theX)
		  c40	add
		  c41	push
		  c42	pTos 	$2a  	(margin)
		  c44	lat  	$5   	(theY)
		  c46	add
		  c47	push
’K		  c4b	pushi	$94  
		  c4e	push0
		  c4f	pushi	$8d  
		  c52	push0
		  c53	lat  	$b   	(textClass)
		  c55	send
		  c55	word	$4
		  c58	send
		  c58	word	$36
		  c5b	sat  	$9   	(ret)
		  c5d	push
		  c5e	pToa 	$14  	(dialog)
		  c60	send
		  c60	word	$6
		  c63	jmp  	$ff  	(.22)
.15
’K’K		  c6c	ldi  	$0   
		  c6e	sat  	$4   	(theX)
’K		  c73	ldi  	$0   
		  c75	sat  	$5   	(theY)
’K		  c7a	lsp  	$0   	(argc)
		  c7c	ldi  	$2   
		  c7e	ge?
		  c7f	bnt  	$1c  	(.17)
’K		  c84	ldi  	$1   
		  c86	lapi 	$1   	()
		  c88	sat  	$4   	(theX)
’K		  c8d	lsp  	$0   	(argc)
		  c8f	ldi  	$3   
		  c91	ge?
		  c92	bnt  	$9   	(.16)
’K		  c97	ldi  	$2   
		  c99	lapi 	$1   	()
		  c9b	sat  	$5   	(theY)
.16
.17
’K		  ca0	pushi	$8d  
		  ca3	push0
		  ca4	class	$f   	(String)
		  ca6	send
		  ca6	word	$4
		  ca9	sat  	$7   	(theText)
’K		  cae	pushi	$bc  
		  cb1	push1
		  cb2	push2
		  cb3	pushi	$9   
		  cb5	ldi  	$0   
		  cb7	lspi 	$1   	()
		  cb9	callk	$5c  	(KString)
		  cb9	word	$4
		  cbd	push
		  cbe	lat  	$7   	(theText)
		  cc0	send
		  cc0	word	$6
’K’K		  cc9	pushi	$96  
		  ccc	push1
’K’K’K		  cd6	pushi	$23  
		  cd8	push1
		  cd9	pushi	$71  
		  cdb	push0
		  cdc	lat  	$7   	(theText)
		  cde	send
		  cde	word	$4
		  ce1	push
’K		  ce5	pushi	$2a  
		  ce7	push1
		  ce8	pTos 	$1c  	(font)
’K		  ced	pushi	$27  
		  cef	push1
		  cf0	pTos 	$1a  	(mode)
’K		  cf5	pushi	$25  
		  cf7	push1
		  cf8	pTos 	$20  	(fore)
’K		  cfd	pushi	$26  
		  cff	push1
		  d00	pTos 	$22  	(back)
		  d02	ldi  	$ffff
		  d04	eq?
		  d05	bnt  	$a   	(.18)
		  d07	pushi	$26  
		  d09	push0
		  d0a	lat  	$b   	(textClass)
		  d0c	send
		  d0c	word	$4
		  d0f	jmp  	$2   	(.19)
.18
		  d11	pToa 	$22  	(back)
.19
		  d13	push
’K		  d17	pushi	$72  
		  d19	push1
		  d1a	pTos 	$24  	(skip)
		  d1c	ldi  	$ffff
		  d1e	eq?
		  d1f	bnt  	$a   	(.20)
		  d21	pushi	$72  
		  d23	push0
		  d24	lat  	$b   	(textClass)
		  d26	send
		  d26	word	$4
		  d29	jmp  	$2   	(.21)
.20
		  d2b	pToa 	$24  	(skip)
.21
		  d2d	push
’K		  d31	pushi	$177 
		  d34	push1
		  d35	pTos 	$1e  	(width)
’K		  d3a	pushi	$c3  
		  d3d	push2
		  d3e	pTos 	$2a  	(margin)
		  d40	lat  	$4   	(theX)
		  d42	add
		  d43	push
		  d44	pTos 	$2a  	(margin)
		  d46	lat  	$5   	(theY)
		  d48	add
		  d49	push
’K		  d4d	pushi	$94  
		  d50	push0
		  d51	pushi	$8d  
		  d54	push0
		  d55	lat  	$b   	(textClass)
		  d57	send
		  d57	word	$4
		  d5a	send
		  d5a	word	$36
		  d5d	sat  	$9   	(ret)
		  d5f	push
		  d60	pToa 	$14  	(dialog)
		  d62	send
		  d62	word	$6
.22
’K’K		  d6b	pushi	$71  
		  d6d	push1
		  d6e	push0
’K		  d72	pushi	$8f  
		  d75	push0
		  d76	lat  	$7   	(theText)
		  d78	send
		  d78	word	$a
’K		  d7e	lat  	$9   	(ret)
		  d80	ret
’K		  d84	ret


Method: (Print addTextBM)

’K		  d88	file
		  d92	link 	$1   
’K		  d97	pushi	$1a0 
		  d9a	push0
		  d9b	&rest	$4   
		  d9d	self
		  d9d	word	$4
		  da0	sat  	$0   	(theText)
’K		  da5	lsp  	$1   	(v)
		  da7	ldi  	$ffff
		  da9	ne?
		  daa	bnt  	$29  	(.0)
’K’K		  db2	pushi	$e   
		  db4	push1
		  db5	lsp  	$1   	(v)
’K		  dba	pushi	$f   
		  dbc	push1
		  dbd	lsp  	$2   	(l)
’K		  dc2	pushi	$10  
		  dc4	push1
		  dc5	lsp  	$3   	(c)
’K		  dca	pushi	$177 
		  dcd	push0
		  dce	lat  	$0   	(theText)
		  dd0	send
		  dd0	word	$16
		  dd3	jmp  	$29  	(.1)
.0
’K’K’K		  dde	pushi	$e   
		  de0	push1
		  de1	pushi	$4d2 
’K		  de7	pushi	$f   
		  de9	push1
		  dea	push2
’K		  dee	pushi	$10  
		  df0	push1
		  df1	push0
’K		  df5	pushi	$177 
		  df8	push0
		  df9	lat  	$0   	(theText)
		  dfb	send
		  dfb	word	$16
.1
’K		  e01	ret


Method: (Print addTextF)

’K		  e05	file
		  e0f	link 	$2   
’K		  e14	push0
		  e15	&rest	$1   
		  e17	call 	$f375	(FindFormatLen)
		  e17	word	$0
		  e1c	sat  	$0   	(len)
’K		  e21	pushi	$ba  
		  e24	push2
		  e25	push
		  e26	lofsa	$4a  
		  e29	push
		  e2a	class	$f   	(String)
		  e2c	send
		  e2c	word	$8
		  e2f	sat  	$1   	(buffer)
’K		  e34	pushi	$d6  
		  e37	push0
		  e38	&rest	$1   
		  e3a	send
		  e3a	word	$4
’K		  e40	pushi	$1a0 
		  e43	push1
		  e44	pushi	$71  
		  e46	push0
		  e47	lat  	$1   	(buffer)
		  e49	send
		  e49	word	$4
		  e4c	push
		  e4d	self
		  e4d	word	$6
’K		  e53	pushi	$8f  
		  e56	push0
		  e57	lat  	$1   	(buffer)
		  e59	send
		  e59	word	$4
’K		  e5f	ret


Method: (Print addTitle)

’K		  e63	file
		  e6d	link 	$6   
’K		  e72	lsp  	$0   	(argc)
		  e74	ldi  	$1   
		  e76	gt?
		  e77	bnt  	$b4  	(.2)
’K		  e7d	ldi  	$0   
		  e7f	lapi 	$1   	()
		  e81	sat  	$0   	(noun)
’K		  e86	ldi  	$1   
		  e88	lapi 	$1   	()
		  e8a	sat  	$1   	(verb)
’K		  e8f	ldi  	$2   
		  e91	lapi 	$1   	()
		  e93	sat  	$2   	(case)
’K		  e98	ldi  	$3   
		  e9a	lapi 	$1   	()
		  e9c	sat  	$3   	(seq)
’K		  ea1	ldi  	$4   
		  ea3	lapi 	$1   	()
		  ea5	sat  	$4   	(module)
’K		  eaa	pushi	$8d  
		  ead	push0
		  eae	class	$f   	(String)
		  eb0	send
		  eb0	word	$4
		  eb3	sat  	$5   	(theText)
’K		  eb8	pushi	$6   
		  eba	push2
		  ebb	lst  	$4   	(module)
		  ebd	lst  	$0   	(noun)
		  ebf	lst  	$1   	(verb)
		  ec1	lst  	$2   	(case)
		  ec3	lst  	$3   	(seq)
		  ec5	callk	$48  	(Message)
		  ec5	word	$c
		  ec9	not
		  eca	bt   	$1e  	(.0)
’K		  ecf	pushi	$7   
		  ed1	push0
		  ed2	lst  	$4   	(module)
		  ed4	lst  	$0   	(noun)
		  ed6	lst  	$1   	(verb)
		  ed8	lst  	$2   	(case)
		  eda	lst  	$3   	(seq)
		  edc	pushi	$71  
		  ede	push0
		  edf	lat  	$5   	(theText)
		  ee1	send
		  ee1	word	$4
		  ee4	push
		  ee5	callk	$48  	(Message)
		  ee5	word	$e
		  ee9	not
.0
		  eea	bnt  	$15  	(.1)
’K’K		  ef2	pushi	$8f  
		  ef5	push0
		  ef6	lat  	$5   	(theText)
		  ef8	send
		  ef8	word	$4
’K		  efe	ldi  	$0   
		  f00	ret
.1
’K		  f04	push2
		  f05	pushi	$8   
		  f07	pushi	$71  
		  f09	push0
		  f0a	lat  	$5   	(theText)
		  f0c	send
		  f0c	word	$4
		  f0f	push
		  f10	callk	$5c  	(KString)
		  f10	word	$4
		  f14	aTop 	$18  	(title)
’K’K		  f1c	pushi	$71  
		  f1e	push1
		  f1f	push0
’K		  f23	pushi	$8f  
		  f26	push0
		  f27	lat  	$5   	(theText)
		  f29	send
		  f29	word	$a
		  f2c	jmp  	$1b  	(.3)
.2
’K’K		  f34	push2
		  f35	pushi	$8   
		  f37	push2
		  f38	pushi	$9   
		  f3a	ldi  	$0   
		  f3c	lspi 	$1   	()
		  f3e	callk	$5c  	(KString)
		  f3e	word	$4
		  f42	push
		  f43	callk	$5c  	(KString)
		  f43	word	$4
		  f47	aTop 	$18  	(title)
.3
’K		  f4c	ret


Method: (Print dispose)

’K		  f50	file
’K		  f5d	pushi	$9a  
		  f60	push1
		  f61	pushSelf
		  f62	lag  	$5c  	(prints)
		  f64	send
		  f64	word	$6
		  f67	bnt  	$c   	(.0)
’K		  f6c	pushi	$58  
		  f6e	push1
		  f6f	pushSelf
		  f70	lag  	$5c  	(prints)
		  f72	send
		  f72	word	$6
.0
’K		  f78	pushi	$8f  
		  f7b	push0
		  f7c	pToa 	$16  	(plane)
		  f7e	send
		  f7e	word	$4
’K		  f84	ldi  	$0   
		  f86	aTop 	$38  	(bitmap)
		  f88	aTop 	$16  	(plane)
		  f8a	aTop 	$36  	(saveCursor)
		  f8c	aTop 	$34  	(first)
		  f8e	aTop 	$18  	(title)
		  f90	aTop 	$2c  	(ticks)
’K		  f95	aTop 	$1a  	(mode)
’K		  f9a	aTop 	$1e  	(width)
’K		  f9f	ldi  	$ffff
		  fa1	aTop 	$28  	(y)
		  fa3	aTop 	$26  	(x)
’K		  fa8	ldi  	$0   
		  faa	aTop 	$32  	(modeless)
’K		  faf	pushi	$f2  
		  fb2	push1
		  fb3	push0
		  fb4	lag  	$8   	(sounds)
		  fb6	send
		  fb6	word	$6
’K		  fbc	pushi	$8f  
		  fbf	push0
		  fc0	super	$0   	(Object)
		  fc0	word	$4
’K		  fc7	ret


Method: (Print cue)

’K		  fcb	file
		  fd5	link 	$1   
’K		  fda	pToa 	$2e  	(caller)
		  fdc	sat  	$0   	(theCaller)
’K		  fe1	ldi  	$0   
		  fe3	aTop 	$14  	(dialog)
’K		  fe8	pushi	$8f  
		  feb	push0
		  fec	self
		  fec	word	$4
’K		  ff2	lat  	$0   	(theCaller)
		  ff4	bnt  	$a   	(.0)
’K		  ff9	pushi	$b3  
		  ffc	push0
		  ffd	send
		  ffd	word	$4
.0
’K		 1003	ret


Method: (Print posn)

’K		 1007	file
’K		 1014	lap  	$1   	(theX)
		 1016	aTop 	$26  	(x)
’K		 101b	lap  	$2   	(theY)
		 101d	aTop 	$28  	(y)
’K		 1022	ret


Method: (Print init)

’K		 1026	file
’K		 1033	ldi  	$0   
		 1035	aTop 	$2e  	(caller)
’K		 103a	lap  	$0   	(argc)
		 103c	bnt  	$7   	(.0)
’K		 1041	lap  	$1   	(whoCares)
		 1043	aTop 	$2e  	(caller)
.0
’K		 1048	lsp  	$0   	(argc)
		 104a	ldi  	$1   
		 104c	gt?
		 104d	bnt  	$c   	(.1)
’K		 1052	pushi	$1a0 
		 1055	push0
		 1056	&rest	$2   
		 1058	self
		 1058	word	$4
.1
’K		 105e	pushi	$1b3 
		 1061	push0
		 1062	self
		 1062	word	$4
’K		 1068	ret


Method: (Print doit)

’K		 106c	file
’K		 1079	pushi	$45  
		 107b	push0
		 107c	pToa 	$14  	(dialog)
		 107e	send
		 107e	word	$4
’K		 1084	ret


Method: (Print handleEvent)

’K		 1088	file
’K		 1095	pushi	$a6  
		 1098	push1
		 1099	lsp  	$1   	(event)
		 109b	pToa 	$14  	(dialog)
		 109d	send
		 109d	word	$6
		 10a0	bnt  	$12  	(.0)
’K		 10a5	pushi	$8f  
		 10a8	push0
		 10a9	pToa 	$14  	(dialog)
		 10ab	send
		 10ab	word	$4
’K		 10b1	ldi  	$1   
		 10b3	ret
.0
’K		 10b7	ret


Method: (Print showSelf)

’K		 10bb	file
		 10c5	link 	$6   
’K		 10ca	pToa 	$36  	(saveCursor)
		 10cc	bnt  	$e   	(.0)
’K		 10d1	pushi	$19f 
		 10d4	push1
		 10d5	lsg  	$14  	(normalCursor)
		 10d7	lag  	$1   	(theGame)
		 10d9	send
		 10d9	word	$6
.0
’K		 10df	pToa 	$14  	(dialog)
		 10e1	not
		 10e2	bnt  	$e   	(.1)
’K		 10e7	pushi	$8d  
		 10ea	push0
		 10eb	class	$3d  	(Dialog)
		 10ed	send
		 10ed	word	$4
		 10f0	aTop 	$14  	(dialog)
.1
’K’K		 10f8	pushi	$26  
		 10fa	push1
		 10fb	pTos 	$22  	(back)
		 10fd	ldi  	$ffff
		 10ff	eq?
		 1100	bnt  	$4   	(.2)
		 1102	ldi  	$0   
		 1104	jmp  	$2   	(.3)
.2
		 1106	pToa 	$22  	(back)
.3
		 1108	push
		 1109	pToa 	$16  	(plane)
		 110b	bnt  	$9   	(.4)
		 110d	pushi	$8d  
		 1110	push0
		 1111	send
		 1111	word	$4
		 1114	jmp  	$9   	(.5)
.4
		 1116	pushi	$8d  
		 1119	push0
		 111a	lag  	$26  	(systemPlane)
		 111c	send
		 111c	word	$4
.5
		 111f	aTop 	$16  	(plane)
		 1121	send
		 1121	word	$6
’K’K		 112a	push0
		 112b	push1
		 112c	pTos 	$16  	(plane)
’K		 1131	pushi	$20  
		 1133	push1
		 1134	lofsa	$4f  
		 1137	push
’K		 113b	pushi	$b1  
		 113e	push1
		 113f	pushSelf
’K		 1143	pushi	$23  
		 1145	push1
		 1146	pTos 	$18  	(title)
’K		 114b	pushi	$ad  
		 114e	push1
		 114f	pTos 	$2c  	(ticks)
’K		 1154	pushi	$199 
		 1157	push1
		 1158	pTos 	$2a  	(margin)
’K		 115d	pushi	$19a 
		 1160	push1
		 1161	pTos 	$32  	(modeless)
’K		 1166	pushi	$19b 
		 1169	push1
		 116a	pTos 	$3a  	(onScreen)
’K		 116f	pushi	$8e  
		 1172	push0
		 1173	pToa 	$14  	(dialog)
		 1175	send
		 1175	word	$34
’K		 117b	pToa 	$38  	(bitmap)
		 117d	bnt  	$44  	(.6)
’K’K		 1186	pushi	$12  
		 1188	push1
		 1189	pushi	$12  
		 118b	push0
		 118c	send
		 118c	word	$4
		 118f	push
’K		 1193	pushi	$13  
		 1195	push1
		 1196	pushi	$13  
		 1198	push0
		 1199	pToa 	$38  	(bitmap)
		 119b	send
		 119b	word	$4
		 119e	push
’K		 11a2	pushi	$14  
		 11a4	push1
		 11a5	pushi	$14  
		 11a7	push0
		 11a8	pToa 	$38  	(bitmap)
		 11aa	send
		 11aa	word	$4
		 11ad	push
’K		 11b1	pushi	$15  
		 11b3	push1
		 11b4	pushi	$15  
		 11b6	push0
		 11b7	pToa 	$38  	(bitmap)
		 11b9	send
		 11b9	word	$4
		 11bc	push
		 11bd	pToa 	$14  	(dialog)
		 11bf	send
		 11bf	word	$18
		 11c2	jmp  	$f   	(.7)
.6
’K’K		 11ca	pushi	$177 
		 11cd	push0
		 11ce	pToa 	$14  	(dialog)
		 11d0	send
		 11d0	word	$4
.7
’K		 11d6	pToa 	$18  	(title)
		 11d8	bnt  	$6e  	(.8)
’K		 11de	pushi	$184 
		 11e1	push1
		 11e2	push
		 11e3	push0
		 11e4	push0
		 11e5	pToa 	$14  	(dialog)
		 11e7	send
		 11e7	word	$4
		 11ea	send
		 11ea	word	$6
’K’K		 11f3	pushi	$12  
		 11f5	push1
		 11f6	pushi	$87  
		 11f9	push0
		 11fa	push0
		 11fb	push0
		 11fc	pToa 	$14  	(dialog)
		 11fe	send
		 11fe	word	$4
		 1201	send
		 1201	word	$4
		 1204	push
’K		 1208	pushi	$13  
		 120a	push1
		 120b	pushi	$89  
		 120e	push0
		 120f	push0
		 1210	push0
		 1211	pToa 	$14  	(dialog)
		 1213	send
		 1213	word	$4
		 1216	send
		 1216	word	$4
		 1219	push
’K		 121d	pushi	$14  
		 121f	push1
		 1220	pushi	$88  
		 1223	push0
		 1224	push0
		 1225	push0
		 1226	pToa 	$14  	(dialog)
		 1228	send
		 1228	word	$4
		 122b	send
		 122b	word	$4
		 122e	push
’K		 1232	pushi	$15  
		 1234	push1
		 1235	pushi	$8a  
		 1238	push0
		 1239	push0
		 123a	push0
		 123b	pToa 	$14  	(dialog)
		 123d	send
		 123d	word	$4
		 1240	send
		 1240	word	$4
		 1243	push
		 1244	pToa 	$14  	(dialog)
		 1246	send
		 1246	word	$18
.8
’K		 124c	pushi	$73  
		 124e	push0
		 124f	pToa 	$14  	(dialog)
		 1251	send
		 1251	word	$4
’K		 1257	pTos 	$26  	(x)
		 1259	ldi  	$ffff
		 125b	eq?
		 125c	bnt  	$a   	(.9)
		 125e	pushi	$12  
		 1260	push0
		 1261	pToa 	$14  	(dialog)
		 1263	send
		 1263	word	$4
		 1266	jmp  	$2   	(.10)
.9
		 1268	pToa 	$26  	(x)
.10
		 126a	sat  	$2   	(theX)
’K		 126f	pTos 	$28  	(y)
		 1271	ldi  	$ffff
		 1273	eq?
		 1274	bnt  	$a   	(.11)
		 1276	pushi	$13  
		 1278	push0
		 1279	pToa 	$14  	(dialog)
		 127b	send
		 127b	word	$4
		 127e	jmp  	$2   	(.12)
.11
		 1280	pToa 	$28  	(y)
.12
		 1282	sat  	$3   	(theY)
’K’K		 128a	pushi	$c3  
		 128d	push2
		 128e	lst  	$2   	(theX)
		 1290	push
’K		 1294	pushi	$97  
		 1297	push1
		 1298	pushi	$17c 
		 129b	pToa 	$14  	(dialog)
		 129d	send
		 129d	word	$e
’K		 12a3	push0
		 12a4	callk	$21  	(FrameOut)
		 12a4	word	$0
’K		 12ab	pTos 	$32  	(modeless)
		 12ad	ldi  	$0   
		 12af	eq?
		 12b0	bnt  	$18a 	(.27)
’K		 12b6	pushi	$f2  
		 12b9	push1
		 12ba	push1
		 12bb	lag  	$8   	(sounds)
		 12bd	send
		 12bd	word	$6
’K		 12c3	pToa 	$34  	(first)
		 12c5	sat  	$0   	(default)
		 12c7	not
		 12c8	bnt  	$43  	(.15)
’K		 12ce	pushi	$98  
		 12d1	push2
		 12d2	pushi	$17a 
		 12d5	push1
		 12d6	pToa 	$14  	(dialog)
		 12d8	send
		 12d8	word	$8
		 12db	sat  	$0   	(default)
		 12dd	bnt  	$40  	(.16)
’K		 12e3	pushi	$98  
		 12e6	push2
		 12e7	pushi	$17a 
		 12ea	push2
		 12eb	pToa 	$14  	(dialog)
		 12ed	send
		 12ed	word	$8
		 12f0	not
		 12f1	bnt  	$2c  	(.16)
’K		 12f7	pushi	$29  
		 12f9	push1
		 12fa	pushi	$29  
		 12fc	push0
		 12fd	lat  	$0   	(default)
		 12ff	send
		 12ff	word	$4
		 1302	push
		 1303	ldi  	$2   
		 1305	or
		 1306	push
		 1307	lat  	$0   	(default)
		 1309	send
		 1309	word	$6
.13
.14
		 130c	jmp  	$12  	(.16)
.15
’K’K		 1314	pushi	$4b  
		 1316	push1
		 1317	lst  	$0   	(default)
		 1319	pToa 	$14  	(dialog)
		 131b	send
		 131b	word	$6
		 131e	sat  	$0   	(default)
.16
’K		 1323	pushi	$45  
		 1325	push1
		 1326	lst  	$0   	(default)
		 1328	pToa 	$14  	(dialog)
		 132a	send
		 132a	word	$6
		 132d	aTop 	$30  	(retValue)
’K		 1332	pTos 	$30  	(retValue)
		 1334	dup
’K		 1338	ldi  	$ffff
		 133a	eq?
		 133b	bnt  	$a   	(.17)
’K		 1340	ldi  	$0   
		 1342	aTop 	$30  	(retValue)
		 1344	jmp  	$b7  	(.24)
.17
		 1347	dup
’K		 134b	ldi  	$fffe
		 134d	eq?
		 134e	bnt  	$a   	(.18)
’K		 1353	ldi  	$1   
		 1355	aTop 	$30  	(retValue)
		 1357	jmp  	$a4  	(.24)
.18
		 135a	dup
’K		 135e	ldi  	$0   
		 1360	eq?
		 1361	bnt  	$a   	(.19)
’K		 1366	ldi  	$1   
		 1368	aTop 	$30  	(retValue)
		 136a	jmp  	$91  	(.24)
.19
’K’K		 1373	pushi	$172 
		 1376	push0
		 1377	pToa 	$30  	(retValue)
		 1379	send
		 1379	word	$4
		 137c	bnt  	$2a  	(.20)
’K’K		 1385	pushi	$1b5 
		 1388	push1
		 1389	pushi	$172 
		 138c	push0
		 138d	pToa 	$30  	(retValue)
		 138f	send
		 138f	word	$4
		 1392	push
’K		 1396	pushi	$1b6 
		 1399	push1
		 139a	pushi	$173 
		 139d	push0
		 139e	pToa 	$30  	(retValue)
		 13a0	send
		 13a0	word	$4
		 13a3	push
		 13a4	lag  	$1   	(theGame)
		 13a6	send
		 13a6	word	$c
.20
’K’K		 13af	pushi	$91  
		 13b2	push1
		 13b3	class	$37  	(DEdit)
		 13b5	push
		 13b6	pToa 	$30  	(retValue)
		 13b8	send
		 13b8	word	$6
		 13bb	bnt  	$10  	(.21)
’K		 13c0	pushi	$23  
		 13c2	push0
		 13c3	pToa 	$30  	(retValue)
		 13c5	send
		 13c5	word	$4
		 13c8	aTop 	$30  	(retValue)
		 13ca	jmp  	$31  	(.23)
.21
’K		 13d0	pushi	$91  
		 13d3	push1
		 13d4	class	$3a  	(DSelector)
		 13d6	push
		 13d7	pToa 	$30  	(retValue)
		 13d9	send
		 13d9	word	$6
		 13dc	bnt  	$10  	(.22)
’K		 13e1	pushi	$18f 
		 13e4	push0
		 13e5	pToa 	$30  	(retValue)
		 13e7	send
		 13e7	word	$4
		 13ea	aTop 	$30  	(retValue)
		 13ec	jmp  	$10  	(.23)
.22
’K’K		 13f4	pushi	$51  
		 13f6	push0
		 13f7	pToa 	$30  	(retValue)
		 13f9	send
		 13f9	word	$4
		 13fc	aTop 	$30  	(retValue)
.23
.24
		 13fe	toss
’K		 1402	pToa 	$36  	(saveCursor)
		 1404	bnt  	$1b  	(.26)
		 1407	lag  	$45  	(theIconBar)
.25
		 1409	bnt  	$16  	(.26)
’K		 140f	pushi	$19f 
		 1412	push1
		 1413	pushi	$1b7 
		 1416	push0
		 1417	lag  	$45  	(theIconBar)
		 1419	send
		 1419	word	$4
		 141c	push
		 141d	lag  	$1   	(theGame)
		 141f	send
		 141f	word	$6
.26
’K		 1425	pToa 	$30  	(retValue)
		 1427	sat  	$5   	(ret)
’K		 142c	pushi	$8f  
		 142f	push0
		 1430	pToa 	$14  	(dialog)
		 1432	send
		 1432	word	$4
’K		 1438	lat  	$5   	(ret)
		 143a	ret
		 143b	jmp  	$10  	(.28)
.27
’K’K		 1443	pushi	$a1  
		 1446	push1
		 1447	pushSelf
		 1448	lag  	$5c  	(prints)
		 144a	send
		 144a	word	$6
.28
’K		 1450	ret


Method: (Print isModeless)

’K		 1454	file
’K’K		 1464	lap  	$0   	(argc)
		 1466	bnt  	$a   	(.0)
’K		 146b	pTos 	$32  	(modeless)
		 146d	lap  	$1   	(checkFor)
		 146f	eq?
		 1470	jmp  	$8   	(.1)
.0
’K’K		 1478	pToa 	$32  	(modeless)
.1
		 147a	ret
’K		 147e	ret
		 147f	byte	$0


Fixups:
		 1480	word	$5
		 1482	word	$157
		 1484	word	$1ba
		 1486	word	$859
		 1488	word	$e27
		 148a	word	$1135
