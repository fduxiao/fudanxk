#########  Makefile       ###########
#all:MyHttpServer.c
#	gcc MyHttpServer.c -lpthread -o MyHttpServer
all:imgproc.c
	g++ imgproc.c svm.o libjpeg.a -o imgproc -lm
