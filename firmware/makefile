SOURCES = main.cpp DbcUtil.cpp LogUtil.cpp ##Add more files here
cc = g++ ## CHANGE COMPILIER AS NECESSARY

## Rudimentary Makefile

run: main
	./main

main: main.cpp
	$(cc) $(SOURCES) -o main

main.cpp:
	echo "Compiling main.cpp"

clean:
	rm main