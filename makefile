# the source/header files directory
#VPATH = ./Src:./Header
vpath %.h ./Header
vpath %.cc ./Src
# create the .out file command
cmdOut = g++ -g -o $@
# create the .o file command
cmnCmd = ${cmdOut} -c $<

httpServerApp.out: AppMain.o HttpHandler.o TaskManager.o
	${cmdOut} $^ -lpthread

AppMain.o: AppMain.cc TaskManager.h config.h
	${cmnCmd}

HttpHandler.o: HttpHandler.cc HttpHandler.h config.h
	${cmnCmd}

TaskManager.o: TaskManager.cc TaskManager.h HttpHandler.h
	${cmnCmd}

.PHONY: clean

clean:
	rm -f *.out *.o