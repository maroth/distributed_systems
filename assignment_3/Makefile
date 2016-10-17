all:
	popcc -o main helloworld.cc helloworld.ph main.cc
	popcc -object -o helloworld.obj helloworld.cc helloworld.ph
	./helloworld.obj -listlong > obj.map

run:
	popcrun obj.map ./main 2

clean:
	rm *.o *.obj main
