snipper: manifest
	g++ -c -o build\snipper.o src\snipper.cpp -lgdi32

manifest:
	windres --input src\snipperManifest.rc --output build\snipperManifest.res --output-format=coff

clear:
	del build\snipper.o build\snipperManifest.res