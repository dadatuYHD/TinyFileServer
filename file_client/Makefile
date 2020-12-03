tarject = file_client

$(tarject): *.c *.h
	gcc $^ -o  $@ -lpthread -lprotobuf-c -lcjson -std=c99 -g
clean:
	rm -rf file_client
