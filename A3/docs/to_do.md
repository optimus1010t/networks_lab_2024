to-do:
    random QUIT in between and handle wherever somehting is getting sent


    look at multiline output from the server
    case-insensitive


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

