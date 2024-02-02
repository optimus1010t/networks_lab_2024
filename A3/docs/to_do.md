to-do:
    sending QUIT to server then OK from consumer then QUIT


    look at multiline output from the server
    case-insensitive
    add conditions to check if there is < and > such that not get stuck in a forever loop
    what all to check for recipients mail to start the mail writing process
    checks for sender and recipient between former and later

queries:
    HELO, MAIL, RCPT, DATA, QUIT protocol info
    does the server check for mailboxes of recievers and senders; checking domain



smtpsmail.c
    implementing timeouts

mailclinet.c

popserver.c

how do we get to know that the entire thing is recieved like what is the EOF? -- \r\n
what should be the buffer/packet size over tcp -- 4000, added other constraints too


final:
    case insensitive
    check if we can send mail to a single mail server with multiple mail clients
    handle multiline output from the server
    adding the RECIEVED line which indicates time and all
    give prot number as a command line argument smtp

