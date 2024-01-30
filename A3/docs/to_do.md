queries:
    HELO, MAIL, RCPT DATA, QUIT protocol info
    does the server check for mailboxes of recievers and senders; checking domain
    what is <CR><LF> and how are lines getting terminated
    how do we get to know that the entire thing is recieved like what is the EOF?

    what should be the buffer/packet size over tcp -- keeping it to be 100

smtpsmail.c
    implementing timeouts

mailclinet.c
    send port number through cmd arg

popserver.c
