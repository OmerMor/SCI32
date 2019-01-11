;;;;
;;;;	MDARRAY.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Brian K. Hughes
;;;;	Updated:	
;;;;
;;;;	This class manages a block of memory as a multi-dimensional array.
;;;;
;;;;	Classes:
;;;;		MDArray


(script# MDARRAY)


(class MDArray kindof Object
	(properties
		dimensions	1
		size			0		;an indirect to a list of sizes (1 per dimension)
		index			0		;an indirect to a list of indeces
		curIndex		0		;an absolute pointer to a location
		data			0		;a pointer to the memory allocated
		initializer	0		;value to initialize array to
	)
	(methods
		init					;sets up the memory allocations & clears the data
		findElement			;calculates curIndex based on passed indeces
		get					;returns the element at the curIndex
		put					;stores a value at the curIndex
	)

	(method (init sizes &tmp i total)
		;
		; Allocate the space for the sizes, indeces, and data

		(if (not (= dimensions argc))
			(Prints {<init> No array dimensions passed})
		else
			(= total 1)
			(= size	(Memory MNeedPtr (* dimensions 2)))
			(= index	(Memory MNeedPtr (* dimensions 2)))

			(for 	((= i 0))
					(< i dimensions)
					((++ i))
				(*= total [sizes i])
				; Write the sizes passed into the size array
				(Memory MWriteWord (+ size (* i 2)) [sizes i])
				; Write 0 into the index array
				(Memory MWriteWord (+ index (* i 2)) 0)
			)

			(= data (Memory MNeedPtr (* total 2)))

			; Initialize the data area to 0's
			(for 	((= i 0))
					(< i total)
					((++ i))
				(Memory MWriteWord (+ data (* i 2)) initializer)
			)
		)
	)

	(method (dispose)
		;
		; Deallocate all the space allocated

		(Memory MDisposePtr data)
		(Memory MDisposePtr index)
		(Memory MDisposePtr size)
	)

	(method (findElement indeces &tmp i subtotal)
		;
		; Calculate the curIndex, based on the indeces provided.
		; Also update the index array.

		(if (!= argc dimensions)
			(Prints {<findElement> Incorrect number of indeces specified})
		else
			; Subtotal is the sizes of the dimensions, progressively
			;	(i.e. 1st, then 1st * 2nd, then 1st * 2nd * 3rd)
			; 
			; The current element is:
			;
			;		a + by + cxy + dwxy + evwxy,
			;
			;	where 'a' through 'e' are the indeces supplied and 'v' through 'y'
			;	are the array sizes.  For example, element 1,1,3 in a 3x2x4 array
			;	would be expressed as:
			;
			;		1 + 1*3 + 3*3*2, or element #22
			;
			;	Note that the last size ('z') is never used to calculate indeces.


			(= curIndex [indeces 0])
			(= subtotal 1)
			(for	((= i 0))
					(< i dimensions)
					((++ i))

				; First, check to see if the index supplied is out of range
				(if (or	(>= [indeces i] (Memory MReadWord (+ size (* i 2))))
							(< [indeces i] 0)
					)
					(Printf {<findElement> Index out of range: %d} [indeces i])
				else
					
					; Store off the indeces in the index array
					(Memory MWriteWord (+ index (* i 2)) [indeces i])

					; Now accumulate both the subtotal (array sizes) and
					;	curIndex, according to the indeces provided
					(if i
						(*= subtotal (Memory MReadWord (+ size (* (- i 1) 2))))
						(+= curIndex (* subtotal [indeces i]))
					else
						(= curIndex [indeces 0])
					)
				)
			)
		)
	)

	(method (get indeces)
		;
		; Find the element specified by the indeces supplied and return it
		; If no indeces are supplied, it is assume that the curIndex will
		;	be used.  Otherwise, there must be 'dimension' number of indeces
		;	passed.

		(if (not argc)
		  	(return (Memory MReadWord (+ data (* curIndex 2))))
		else
			(self findElement: indeces &rest)
			(return (Memory MReadWord (+ data (* curIndex 2))))
		)
	)

	(method (put value indeces)
		;
		; Store the value specified in the element specified by the indeces.
		; If no indeces are supplied, it is assume that the curIndex will
		;	be used.  Otherwise, there must be 'dimension' number of indeces
		;	passed.

		(if (not argc)
			(Memory MWriteWord (+ data (* curIndex 2)) value)
		else
			(self findElement: indeces &rest)
			(Memory MWriteWord (+ data (* curIndex 2)) value)
		)
	)
)


