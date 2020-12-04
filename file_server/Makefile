target = FileServer

$(target): *.c *.h
	gcc $^ -o $@ -lpthread -lprotobuf-c -lcjson -g -std=c99
clean:
	rm -rf FileServer
