tarject = file_client

$(tarject): *.c *.h
	gcc $^ -o  $@ -lpthread -lprotobuf-c -std=c99 -g
clean:
	rm -rf file_client
