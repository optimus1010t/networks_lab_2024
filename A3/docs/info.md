<!-- regarding 220 -->
In ABNF, server responses are:
Greeting= ( "220 " (Domain / address-literal)
[ SP textstring ] CRLF ) /
( "220-" (Domain / address-literal)
[ SP textstring ] CRLF
*( "220-" [ textstring ] CRLF )
"220" [ SP textstring ] CRLF )
textstring= 1*(%d09 / %d32-126) ; HT, SP, Printable US-ASCII
Reply-line= *( Reply-code "-" [ textstring ] CRLF )
Reply-code [ SP textstring ] CRLF
Reply-code= %x32-35 %x30-35 %x30-39
where "Greeting" appears only in the 220 response that announces that
the server is opening its part of the connection. (Other possible
server responses upon connection follow the syntax of Reply-line.)
<!-- tell if you need more info -->

<!-- HELO thingy -->
HELO
<!-- will look at the earlier discussion -->
As discussed in Sections 3.1 and 4.1.1, EHLO SHOULD be used rather
than HELO when the server will accept the former. Servers MUST
continue to accept and process HELO in order to support older
clients.

ehlo= "EHLO" SP ( Domain / address-literal ) CRLF
ehlo-ok-rsp
= ( "250" SP Domain [ SP ehlo-greet ] CRLF )
/ ( "250-" Domain [ SP ehlo-greet ] CRLF
*( "250-" ehlo-line CRLF )
"250" SP ehlo-line CRLF )

ehlo-greet= 1*(%d0-9 / %d11-12 / %d14-127)
; string of any characters other than CR or LF
ehlo-line= ehlo-keyword *( SP ehlo-param )
ehlo-keyword= (ALPHA / DIGIT) *(ALPHA / DIGIT / "-")
; additional syntax of ehlo-params depends on
; ehlo-keyword
ehlo-param= 1*(%d33-126)
; any CHAR excluding <SP> and all
; control characters (US-ASCII 0-31 and 127
; inclusive)
Although EHLO keywords may be specified in upper, lower, or mixed
case, they MUST always be recognized and processed in a case-
insensitive manner. This is simply an extension of practices
specified in RFC 821 and Section 2.4.
The EHLO response MUST contain keywords (and associated parameters if
required) for all commands not listed as "required" in Section 4.5.1
excepting only private-use commands as described in Section 4.1.5.
Private-use commands MAY be listed


