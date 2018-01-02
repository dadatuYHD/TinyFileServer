target = file_server

$(target): *.c
	gcc $^ -o $@ -lpthread -lprotobuf-c -g -std=c99
clean:
	rm -rf file_server
