# Snoop_Me
ELEC4123 Network Project

The	objective	of	 this	design	project	is	 to	design	a	client	(or	clients)	which	can	reliably	
recover	the	content	of	a	message by	snooping	individual	packets	 from	a	server.	There	
are	 many	 important	 constraints	 on	 the	 snooping	 process,	 and	 it	 is	 important	 to	
understand	that	up	to	three	snoopers	with	different	IP	addresses	(different	computers	
here)	can	be	used	together	to	recover	the	message.

Figure 1.

<-------------Server----------------<         -------------HTTP Server-----------------
-                                   ^         -                                       -
-                                   ^         -                                       -
-                                   ^         -                                       -
------->[   ]----->[    ]----------->         -----------------------------------------
                -                                                ^
                -                                                 -
                -                                                   -
               -                                                      -
              -                                                         -
            -                                                             -   
          -                                                                 -
         -                                                                    - 
        -            ------[Snooping Queue] ========>  [Computer 1]  -----> [Computer 4]
       -            -                                                           ^   ^
       [Backdoor]---------------[Snooping Queue] ========>  [Computer 2]  ------>   ^
                    -                                                               ^
                      -----[Snooping Queue] ========>  [Computer 3]  --------------->
                      
The	 arrangement	 is	 sketched	 in	 Figure	 1,	 but	 note	 that	 there	 are	 many	 details	 not	
shown	 on	 the	 figure.	 	 One	 item	 not	 shown [updated to be shown]	 is	 an	 HTTP	 server	 to	 which	 one	 of	 your	
computers	should	connect,	in	order	 to	send	 the	deciphered	message	once	you	have	it.		
You	will	need	 to	implement	an	HTTP	client,	using	TCP	as	 the	underlying	 transport,	 to	
post	 your	 message	 and	 receive	 a	 success	 or	 failure	 notification.	 	 Once	 successful,	 the	
message	server	selects	a	new	message	automatically,	which	you	should	again	proceed	
to	try	to	recover.		 In	this	way,	your	designed	solution	is	expected	to	recover	messages	
sequentially, earning	 points	 for	 each	 message	 recovered,	 until	 you	 have	 either	
recovered	all	messages	or	a	timer	expires.
Your	main	 objective	is	 to	 recover	 all	messages	 before	 the	 timer	 expires,	 and	 to	 do	 so	
with	 a	 maximum	 score,	 where	 the	 score	 is	 reduced	 if	 you	 receive	 failure	 codes	 in	
response	 to	your	HTTP	posts.		A	secondary	objective	is	 to	recover	all	 the	messages	as	
fast	as	possible	– i.e.,	well	before	the		timer	expires,	if	possible.
                      
Details	 of	 the	 expiry	 time	 T,	 message	 rate	 R and	 typical	 number	 of	 total	 message	
characters	 C will	 be	 provided	 on	 the	 course	 web-site	 by	 Monday	 of	 Week	 13	 at	 the	
latest.

Detailed Description of the Snoop-Me System

1. 
The server runs in a loop, repeating a current message over and over again until your
system has successfully recovered it.
  a. Notionally, the server is sending the message repeatedly to a client somewhere
else, and you are “snooping” the communication, but the existence of this
other client is not important to you.
  b. The message consists of a sequence of ASCII characters in the range 0x01 to
0x7F, including special characters. The special character ctrl-D (i.e., 0x04)
marks the end of the message (EOM) and will not occur elsewhere.
  c. The message is partitioned into packets of variable length, with the shortest
packets containing as little as one character and the longest packet having up
to 20 characters. The packetization of the message does not change over its
repetitions. The EOM character (ctrl-D) always appears in its own packet, of
length 1. Apart from this, you can assume that packet lengths are random and
uniformly distributed over the range 1 to 20.
  d. Each message packet has an associated 8-byte big-endian 64-bit integer
identifier that increments sequentially, with natural wrap-around. The
identifier does not repeat with the message, and its initial value is essentially
random, at least as far as you are concerned.
  e. The message characters are transmitted at a constant rate of R characters per
second, which means that message packets are transmitted at a non-constant
rate, since they have variable length. The data rate does not include any
allowance for the packet identifier.

2. 
The server provides a “backdoor” with three (3) snooping channels.
  a. Each snooping channel has an associated queue, whose length L is initially 2,
meaning that it can hold up to two snooping requests.
  b. Snooping requests are sent to a designated port on the server as UDP
datagrams. The port assigned to your team will be given out in the first lab in
Week 11.
  c. The snooping request datagram shall be 4 bytes long, these 4 bytes
representing an unsigned big-endian 32-bit integer S, which is chosen by your
design.
  d. Before sending each packet, the server checks each snooping queue for a
request. If one is found, its S value is decremented by 1. If this results in the
S value becoming 0, the request is removed from the queue and a copy of the
packet is sent to the snooper as a UDP datagram, whose contents commence
with the 8-byte big-endian packet identifier and are followed by the packet’s
message characters. Be very careful not to send a snooping request with S=0
here, since S will need to be decremented 232 times before it this produces 0.
    • To be completely clear, the server considers snooping requests
immediately before sending the first character of a message packet.
Thus, if P1 and P2 are two consecutive message packets, with L1 and
L2 characters respectively, snoop requests for these packets are
considered at times T1 and T2 which are separated by T2 − T1 =
L1/R.
  e. The three snooping channels are independent, but each channel gets
permanently assigned to a snooper’s address (IP address and port number)
when it is first seen. Thereafter, all snooping requests from the same snooping
address are processed by the same channel and no other snooping requests are
processed by that channel. Up to three snooping addresses (IP address and port
number) can be used, each of which will be bound to a separate channel when
they are first seen, after which snoops from any other addresses will be
discarded. When the server is in the “exclusive” mode, used for final testing,
no IP address may be assigned to multiple snooping channels, so you will
need to exploit three separate computers to use the full set of snooping
channels.

3. 
Snooping too often is dangerous. The server keeps track of the time between
snooping responses that it has sent – not the time between requests it has received --
where time is measured in characters, since characters are transmitted at the constant
rate R. If this inter-response time drops below 50 characters, you have been
“exposed.” Each time you are exposed, the relevant snooping channel’s queue length
is reduced by 1, until it reaches 0. The queue length is auto-incremented by 1, up to a
maximum length of 2, every 1000 characters. Thus, being exposed is expected to
really slow you down and perhaps interfere with the behaviour of your algorithm.
You do not receive any notification that you have been exposed.

4. 
The main snoop-me server is represented by the executable program “4123-server”,
which is to be run on the shared Linux VM whose details are supplied to you prior to
the frist laboratory in Week 10. Since the VM is shared, each team is assigned a port
number to use with the “4123-server” program so as not to collide with other teams.
This port should be passed as the “-port” argument rather than using the default value.

5. 
Once you have recovered the message, you should post a copy to the HTTP server,
which is represented by the executable program “4123-http-server” and is also run on
the shared Linux VM mentioned above. The HTTP server is configured by “4123-
server” to listen on the port that is one larger than its own listening port. You can also
run the “4123-http-server” in isolation, in which case it is best to launch it with the
port number that is one larger than the port assigned to your team for use with the
“4123-server” program.
  a. The HTTP server expects an HTTP POST request – not an HTTP GET request
– with binary (i.e., not encoded) data. The maximum message length accepted
by the HTTP server is 65535 bytes and there should be no Transfer-Encoding
field in the request.
  b. You will need to make sure you understand the syntax for a correct HTTP
POST request.
  c. You will receive back either an HTTP 200 series (success) response code or
an HTTP 406 response status code (not acceptable). In the former case the
server moves on to the next message. In the later case, you need to keep
trying but your final score for the message is scaled by 0.9. If you receive two
failures for the same message your score for that message is scaled twice by
0.9 (i.e. 0.81) and so forth.
  d. If you issue an invalid request, you will receive a different HTTP error code
and the connection will be closed.
  e. You are to use the HTTP/1.1 protocol, for which persistent TCP connections
are the default. The HTTP server accepts only one TCP connection at a time,
so you should plan to use only one computer to talk to the HTTP server. You
may close the connection after receiving a response and then re-establish a 
connection. In this case you should follow the standard protocol of declaring
your intent to close the connection via request fields, and the server will
respond to let you know that it is closing the connection.
  f. Once the final message is successfully recovered, the HTTP response code
will be 205, rather than 200. You can then disconnect, gracefully or otherwise
from the HTTP server.

Constraints
Your	 design	 should	ideally	 be	implemented	in	C	 or	C++ and	must	 use	BSD	 sockets.	 In	
particular,	it	is	expected	that	your	program(s)	use	the	“select”	 function	to	wait	 for	 I/O	
completion,	 rather	 than	 less	 portable	 methods	 such	 as	 Event	 signaling or	 callback	
functions	that	are	offered	in	the	Windows	sockets	model.	The	BSD	model	is	available	on	
all	operating	systems	of	interest.

                      
                   
