all:
	popcc -o randG shortpathobject.ph shortpathobject.cc randG.cc
	popcc -o kring shortpathobject.ph shortpathobject.cc kring.cc
	popcc -o torus2D shortpathobject.ph shortpathobject.cc torus2D.cc
	popcc -o star shortpathobject.ph shortpathobject.cc star.cc
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap

randG:
	popcc -o randG shortpathobject.ph shortpathobject.cc randG.cc
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap

kring:
	popcc -o kring shortpathobject.ph shortpathobject.cc kring.cc
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap

torus2D:
	popcc -o torus2D shortpathobject.ph shortpathobject.cc torus2D.cc
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap

star:
	popcc -o star shortpathobject.ph shortpathobject.cc star.cc
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap


object:
	popcc -object -o shortpathobject.obj shortpathobject.ph shortpathobject.cc
	./shortpathobject.obj -listlong > objmap

run:
	popcrun objmap ./torus2D 1 3
	mv Excel.txt ExcelTorus2D.txt

	popcrun objmap ./kring 1 9 3 2 4
	mv Excel.txt ExcelKring.txt

	popcrun objmap ./randG 1 13 5
	mv Excel.txt ExcelRandg.txt

	popcrun objmap ./star 1 7
	mv Excel.txt ExcelStar.txt

clean:
	rm *.o randG kring torus2D star *.obj
