target = file_server

$(target): *.c *.h
	gcc $^ -o $@ -lpthread -lprotobuf-c -lcjson -g -std=c99
clean:
	rm -rf file_server
