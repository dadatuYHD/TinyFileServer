tarject = FileClient

$(tarject): *.c *.h
	gcc $^ -o  $@ -lpthread -lprotobuf-c -lcjson -std=c99 -g
clean:
	rm -rf FileClient
