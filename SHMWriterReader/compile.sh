gcc -g -c SHMFunc.c -o SHMFunc.o
gcc -g -c SHMReader.c -o SHMReader.o
gcc -g -c SHMWriter.c -o SHMWriter.o
gcc -g SHMReader.o SHMFunc.o -o SHMReader -lrt
gcc -g SHMWriter.o SHMFunc.o -o SHMWriter -lrt
