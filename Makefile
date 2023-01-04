make: file_manager file_client

file_manager: file_manager.c
	gcc file_manager.c -o manager 

file_client: file_client.c 
	gcc file_client.c -o client 

clear:
	rm -rf *o shell
