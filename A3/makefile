mailclient:
	rm -f mailclient
	gcc mailclient.c -o mailclient
	clear
	./mailclient 127.0.0.1 20001 20002

smtpmail:
	rm -f smtpmail
	gcc smtpmail.c -o smtpmail 
	clear
	./smtpmail 20001

popserver:
	rm -f popserver
	gcc popserver.c -o popserver 
	clear
	./popserver 20002

clean:
	rm -f mailclient
	rm -f smtpmail
	rm -f popserver