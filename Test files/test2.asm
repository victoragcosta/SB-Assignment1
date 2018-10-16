	TRIANGULO:EQU 0

SECTION TEXT
		INPUT		B
		INPUT		H
		LOAD		B
		MULT		H
IF TRIANGULO
		DIV		DOIS
		STORE		R	; Comment 2
		OUTPUT	R		; Comment 1
		STOP
; Comment 3
							; Comment 4
							        ; Line with random tabs and spaces...
SECTION BSS
	B:		SPACE
	H:		SPACE
	R:		SPACE
SECTION DATA
	DOIS:	CONST		2
