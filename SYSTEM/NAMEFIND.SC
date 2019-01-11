;;;;
;;;;	NAMEFIND.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Unknown
;;;;	Updated:
;;;;
;;;;	Procedure for finding an element of the list whose name matches
;;;;	the search criteria.
;;;;
;;;;	Procedures:
;;;;		NameFind


(script# NAMEFIND)


(procedure
	NameFind
)

(public
	NameFind	0
)

(procedure (NameFind theNearName theList)
	(theList firstTrue: #perform NameComp theNearName)
)

(instance NameComp of Code
	(properties name "NC")
	(method (doit theObj theName)
		(== 0 (StrCmp (theObj name) theName))
	)
)
