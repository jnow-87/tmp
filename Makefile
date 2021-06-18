LDLIBS := -lreadline

main: main.o bridge.o protocol.o


clean:
	rm -f main *.o
