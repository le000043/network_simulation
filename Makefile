# A Makefile is provided which compiles bl_server, bl_client and any ADVANCED programs implemented
# Required test_Makefile is included via include test_Makefile

# bl_server : builds the server executable, be included in the default targets.
# bl_client : builds the client executable, be included in the default targets.
# test-binary : compile and run the binary tests
# test-shell : run the shell scripts associated with integration tests



include test_Makefile

CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

PROGRAMS = bl_server bl_client

all : $(PROGRAMS)

%.o : %.c			#default rule to create .o files
	$(CC) -c $<

bl_server : bl_server.o server_funcs.o util.o blather.h
	$(CC) -o bl_server bl_server.o server_funcs.o util.o
	@echo bl_server is ready

bl_client : bl_client.o server_funcs.o util.o blather.h
	$(CC) -o $@ $^ -lpthread

	# $(CC) -o bl_client bl_client.o server_funcs.o util.o
	@echo bl_client is ready

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c

bl_showlog : bl_showlog.o util.o blather.h
	$(CC) -o bl_showlog bl_showlog.o util.o -lpthread
	@echo bl_showlog is ready

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

bl_client.o : bl_client.c blather.h
	$(CC) -c bl_client.c

util.o : util.c blather.h
	$(CC) -c util.c

clean :
	rm -f *.o *.fifo $(PROGRAMS)




# test-binary : bl_client.o server_funcs.o util.o blather.h
# 	$(CC) -o test-binary bl_client.o server_funcs.o util.o
# 	@echo test-binary is ready

# test-shell : bl_client.o server_funcs.o util.o blather.h
# 	$(CC) -o test-shell bl_client.o server_funcs.o util.o
# 	@echo test-shell is ready
