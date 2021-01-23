#KeyLogger Client-Server
[keylogger defintion](https://en.wikipedia.org/wiki/Keystroke_logging): tool that let you record keystrokes of someone else.

My keylogger have server-client(victim) communication, so every client's keystroke will send to the server side.
At the end of the client session, the list of all the keystrokes will be saved to a server-side file named: "client ip address"-victim_doc.txt.

##Usage
###Client
```bash
cd Keylogger
sudo ./keylogger
```
make sure you have to specify "sudo" because you need the premission to read from keyboard event.
###server
```bash
cd Keylogger
./server
```
##Custom use of the IP addresses & Ports
* Note that if your server has its own ip address so you have to change on **'victim.c' line 232** the loopback address to your customize address.
* To change the communication port you will change on both **'victim.c' and 'server.c' the macro PORT** to your customize port.
##Disclaimer
This code is just for practice and not for evil purposes :D
