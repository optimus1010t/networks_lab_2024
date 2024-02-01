mailclient:
	rm -f mailclient
	gcc -O0 mailclient.c -o mailclient
	clear
	./mailclient 127.0.0.0.1 20001 20000

smtpmail:
	rm -f smtpmail
	gcc -O0 smtpmail.c -o smtpmail 
	clear
	./smtpmail 20001

clean:
	rm -f mailclient
	rm -f smtpmail