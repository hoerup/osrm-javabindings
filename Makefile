#!/bin/bash


JAVA_HOME=$(shell readlink -f /usr/bin/javac | sed "s:bin/javac::")


default: libosrmjavabindings.so

clean:
	find . -name '*.class' -exec rm -f {} \;
	rm -f *.o *.so
	rm -f *.jar
	rm -f *.h
	rm -f hs*.log


osrmbinding.jar: dk/thoerup/osrmbinding/OSRMBinding.java
	javac dk/thoerup/osrmbinding/*.java
	jar cf osrmbinding.jar dk

dk_thoerup_osrmbinding_OSRMBinding.h: osrmbinding.jar
	javah dk.thoerup.osrmbinding.OSRMBinding

OSRMBinding.o: OSRMBinding.cpp dk_thoerup_osrmbinding_OSRMBinding.h
	g++ -c -DBOOST_TEST_DYN_LINK -flto -pedantic -fPIC -fdiagnostics-color=auto -std=c++11  -fopenmp -O3 -DNDEBUG \
	-I$(JAVA_HOME)/include \
	OSRMBinding.cpp -o OSRMBinding.o

libosrmjavabindings.so: OSRMBinding.o
	g++  -flto -Wall -pedantic -fPIC -fdiagnostics-color=auto -std=c++11  -fopenmp -O3 -DNDEBUG -shared -rdynamic \
	-lboost_system -lboost_program_options -lboost_filesystem -lboost_thread \
	-o libosrmjavabindings.so \
	OSRMBinding.o /usr/local/lib/libOSRM.a



test: libosrmjavabindings.so
	java -Djava.library.path=. -classpath . dk.thoerup.osrmbinding.OSRMBinding

installso: libosrmjavabindings.so
	install --mode=644 libosrmjavabindings.so /usr/local/lib
	ldconfig

installjar: libosrmjavabindings.so
	mvn install:install-file -Dfile=osrmbinding.jar -DgroupId=dk.thoerup -DartifactId=osrmbinding -Dversion=1.0 -Dpackaging=jar



.PHONY: clean test installso installjar
