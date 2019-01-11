;;;;
;;;;	RELDPATH.SC
;;;;
;;;;	(c) Sierra On-Line, Inc, 1993
;;;;
;;;;	Author: 	Unknown
;;;;	Updated:
;;;;
;;;;	A DPath class that uses relative points.
;;;;
;;;;	Classes:
;;;;		RelDPath


(script#	RELDPATH)


(class RelDPath of DPath
	(method (setTarget)
		(if (!= (points at value) PATHEND)
			(+= x (points at value))
			(+= y (points at (++ value)))
			(++ value)
		)
	)
);RelDPath
