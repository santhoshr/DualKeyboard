dualkeyboards: dual.c
	gcc -Wall -o dual dual.c -framework ApplicationServices
	osascript assistive-enabled.scpt
