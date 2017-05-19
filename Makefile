all:
	cc -pthread HW3_102062335_Ser.c -o server
	cc -pthread HW3_102062335_Cli.c -o client
clean:
	rm server client
