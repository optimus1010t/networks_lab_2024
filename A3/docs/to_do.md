to-do:
    look at multiline output from the server
    case-insensitive
    add conditions to check if there is < and > such that not get stuck in a forever loop

queries:
    HELO, MAIL, RCPT, DATA, QUIT protocol info
    does the server check for mailboxes of recievers and senders; checking domain



smtpsmail.c
    implementing timeouts

mailclinet.c

popserver.c

how do we get to know that the entire thing is recieved like what is the EOF? -- \r\n
what should be the buffer/packet size over tcp -- 4000, added other constraints too