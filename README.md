# Assignment 5




## Components of research
## Analyze on assignment 4:

### Scenario:
- CLIENT_1 connected
- CLIENT_2 connected
- CLIENT_1 send its name
- CLIENT_2 send its name
- CLIENT_1 send file1.txt to SERVER 
- SERVER response with success
- CLIENT_2 get file1.txt from server
- SERVER response with success
- CLIENT_2 terminate connection
- CLIENT_1 terminate connection

## **Part 1:**
## Identify and analyze different types of TCP traffic: 
- TCP handshake;
- TCP data segments && TCP acknowledgments;
- TCP connection termination.

### TCP handshake:
- firstly we locate to the first three rows that are responsible for the connection: 
  1. Flag [SYN]: Let's break information located here : src port=54284(randomly allocated port of the CLIENT_1) , dst port=8080(server port that we choose) , stream index= 4 (tcp stream assigned by wireshark in this trace file), Sequence Number= 1431139160 (wireshark simplify to relative 0),  Acknowledgment number= 0(wireshark simplify to relative 0), Window= 65535(tell the server how many bytes CLIENT_1 can read unacknowledged, but from the option section we can see that it will be scaled by 6/multiply by 64- the reason is that in header we can represent window size with only 2 bytes), Maximum segment size= 16344 bytes(tell how much max bytes can be sent with in tcp segment), Len =0 (indicate that no data was sent), SACK permitted (tells us that if the packet was not received but further packets were, we can ask just for the missing packet and not for ones that get after it as it was if SACK was not permitted).
  2. FLAG [SYN, ACK]: Comes from SERVER: src port=8080, dst port=54284, Sequence Number of server== 753372049, Acknowledgment number= 1431139161(also called ghost byte, this is incremented Sequence Number of CLIENT_1, relative is 1), Window is the same as for CLIENT_1, SACK permitted.
  3. FLAG [ASK] : Comes from CLIENT_1:Incremented Sequence Number of CLIENT_1=  1431139161(relative 1), Acknowledgment number= 753372050(incremented Sequence Number of SERVER),true  window size: 408256.
Can see that the handshake last 100 ms.

### TCP data segments && TCP acknowledgments:

- CLIENT_1 send its name: FLAG [PSH,ASK], Transmission Control Protocol, Src Port: 54284, Dst Port: 8080, Seq: 1(relative), Ack: 1(relative), Len: 4 (this shows that CLIENT_1 sends data which is 4 bytes to the SERVER)  , Sequence Number: 1 (relative), [Next Sequence Number: 5(relative)] (shows us the current Sequence Number of CLIENT_1 and the expected acknowledgment number that will come from the SERVER if it receives data (payload) )Acknowledgment Number: 1(relative) (it is incremented Sequence Number of SERVER), TCP payload (4 bytes) 00 00 00 08 (this hex big endian notation (network byte order) which represent the amount of char that CLIENT_1 plan to send ).

- SERVER acknowledge that it receive the length of the message: FLAG[ASK], Transmission Control Protocol, Src Port: 8080, Dst Port: 54284, Seq: 1, Ack: 5, Len: 0(no data will be sent), Acknowledgment Number: 5(relative) (note that is the same that was expected value of Sequence Number of CLIENT_1, so SERVER receive data successfully)

- CLIENT_1 send the message: FLAG [PSH,ASK], Transmission Control Protocol, Src Port: 54284, Dst Port: 8080, Seq: 5, Ack: 1, Len: 8 (mean it will send 8 char 1 byte each),Sequence Number: 5 (relative)(expected will be 13 cause 5 + Len = 5 + 8 = 13), TCP payload (8 bytes) 43 4c 49 45 4e 54 5f 31 (which in ascii is CLIENT_1).

- SERVER receive the message: FLAG[ASK], Transmission Control Protocol, Src Port: 8080, Dst Port: 54284, Seq: 1, Ack: 13, Len: 0(no data), Sequence Number of SERVER: 1  (relative, expected is 1 cause 1 + Len = 1 + 0 = 1), Acknowledgment Number : 13 (relative, which equal to expected sequential number of the CLIENT_1 so all data was received)

- Then we send CLIENT_1 "Welcome new user" in the same manner. I will provide only header cause it is the same steps and just to make sure that all data was sent and received successfully:

FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54284, Seq: 1, Ack: 13, Len: 4, 

FLAG[ASK] Transmission Control Protocol, Src Port: 54284, Dst Port: 8080, Seq: 13, Ack: 5, Len: 0

FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54284, Seq: 5, Ack: 13, Len: 32

FLAG[ASK] Transmission Control Protocol, Src Port: 54284, Dst Port: 8080, Seq: 13, Ack: 37, Len: 0]


- Then we handle the command GET between CLIENT_1 and SERVER:
- The main steps : 
1. Client send tag 1 for get command(actually for command I implement TLV)
2. Server read tag 1
3. Client send directory length of name  and name 
4. Server read directory length of name and name
5. Client send file length of name and name
6. Client read file length of name and name
7. Server send bool to indicate if directory exist remote
8. Client read bool to indicate if directory exist remote
9. Server send bool to indicate if file exist remote
10. Client read bool to indicate if file exist remote
11. Client send a bool to confirmation to send file
12. Server send the file size
13. Client read the file size
14. Server send length and message "In process: downloading file"
15. Client read length and message
16. Server send by chunk file
17. CLient read by chunk file
18. Server send length and message "Content of the file was succesfully uploading."
19. Client read length and message

Let's take a look how it is implemented:

1.  FLAG [PSH,ASK]Transmission Control Protocol, Src Port: 54284, Dst Port: 8080, Seq: 13, Ack: 37, Len: 1 - CLIENT_2 send tag 01
2.  FLAG [ASK]Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 37, Ack: 14, Len: 0 - SERVER read 1 byte with success
3.  FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 14, Ack: 37, Len: 25 - here 4 bytes(00 00 00 08) for directory, 8 bytes("CLIENT_2"), here 4 bytes(00 00 00 09) for file , 9 bytes ("file1.txt")
4.  FLAG [ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 37, Ack: 39, Len: 0 : SERVER read 25 byte with success
5.  FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 37, Ack: 39, Len: 1: SERVER send bool true indicate if directory exist remote
6.  FLAG [ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 39, Ack: 38, Len: 0 :CLIENT_2 read 1 byte with success
7. FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 38, Ack: 39, Len: 1: SERVER send bool true indicate if file exist remote
8. FLAG [ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 39, Ack: 39, Len: 0 : CLIENT_2 read 1 byte with success
9. FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 39, Ack: 39, Len: 1: CLIENT_2 send bool true to confirmation to send file
10. FLAG [ASK]Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 39, Ack: 40, Len: 0 :SERVER read  1 byte with success
11. FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 39, Ack: 40, Len: 8 : SERVER send the file size (29 3e)
12.  FLAG [ASK]Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 40, Ack: 47, Len: 0: CLIENT_2 read 2 8 bytes
13. FLAG [PSH,ASK]    Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 47, Ack: 40, Len: 5158: : SERVER send the lenght of meassage , message , and start of file
14. FLAG [ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 40, Ack: 5205, Len: 0: CLIENT_2 read with success
15. FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 5205, Ack: 40, Len: 2048: SERVER send file chunk
16. FLAG [ASK] Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 40, Ack: 7253, Len: 0: CLIENT_2 read with success
17. FLAG [PSH,ASK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 10643, Ack: 40, Len: 4: SERVER send the lenght of meassage
18. FLAG [ASK]Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 40, Ack: 10647, Len: 0: CLIENT_2 read with success
19.  FLAG [PSH,ASK]  Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 10647, Ack: 40, Len: 51: SERVER send meassage Content of the file was succesfully uploading."
20. FLAG [ASK]   Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 40, Ack: 10698, Len: 0 :CLIENT_2 read with success



### TCP connection termination.:
1. [FIN,PSH,ACK]Transmission Control Protocol, Src Port: 54285, Dst Port: 8080, Seq: 41, Ack: 10698, Len: 12: Indicate that CLIENT_2 close socket
2. [ACK]Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 10698, Ack: 41, Len: 0: SERVER read with success
3. [ACK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 10698, Ack: 54, Len: 0 : SERVER read with success
4. [RST, ACK] Transmission Control Protocol, Src Port: 8080, Dst Port: 54285, Seq: 10698, Ack: 54, Len: 0 :  SERVER close CLIENT_2 socket




## **Part 4:**
## Conclusion: 
This work give a good understanding of the network trafic of my c++ program. The finding for me lies in fact that data can be sent not in the same grouping  as recv or send but can "concatenate" them inkto one TCP segment.




