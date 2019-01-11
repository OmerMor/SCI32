;;;;
;;;;	LOADMANY.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Pablo Ghenis
;;;;	Updated:
;;;;
;;;;	This procedure allows loading several resources of a single type.
;;;;
;;;;	Classes:
;;;;
;;;;	Procedures:
;;;;		LoadMany


(script# LOADMANY)


(procedure
	LoadMany
)

(public
	LoadMany 0
)

(procedure (LoadMany what which &tmp i theRes)
	(for
		((-= argc 2)(= i 0))
		(<= i argc)
		((++ i))
		
		(= theRes [which i])
		(if what
			(Load what theRes)
		else
			(DisposeScript theRes)
		)
	)

	(DisposeScript LOADMANY)
)
