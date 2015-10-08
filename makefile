#########  Makefile       ###########
#all:MyHttpServer.c
#	gcc MyHttpServer.c -lpthread -o MyHttpServer
CXX=c++
all:imgproc.cpp
	$(CXX) imgproc.cpp svm.o libjpeg.a -o imgproc -lm
clean:clear
clear:
	rm imgproc
