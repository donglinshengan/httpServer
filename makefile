cmdOut=g++ -o $@

httpServerApp.out: AppMain.o HttpHandler.o TaskManager.o
	${cmdOut} $^ -lpthread

AppMain.o: AppMain.cc ./Header/TaskManager.h ./Header/config.h
	${cmdOut} -c AppMain.cc

HttpHandler.o: ./Src/HttpHandler.cc ./Header/HttpHandler.h ./Header/config.h
	${cmdOut} -c ./Src/HttpHandler.cc

TaskManager.o: ./Src/TaskManager.cc ./Header/TaskManager.h ./Header/HttpHandler.h
	${cmdOut} -c ./Src/TaskManager.cc

clean:
	rm -f *.out *.o