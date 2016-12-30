# oscbridge

This service will transform UDP to TCP based OSC communication.

The initial intent is to work around communication problems occur between Ardmix and Ardour, which rely on
slow Android network performance. In case of my tablet, I did miss some important UDP notifications from Ardour,
so my Ardmix app did miss some strips and/or their values.

```
OSC TCP/UDP bridge 0.1.14.
Usage: oscbridge [OPTIONS]

parameters:
  -c TCP_PORT              The port the OSC clients may connect to
  -u SERVER_SEND_PORT      The udp port of the server receiver
  -r SERVER_RECEIVE_PORT   The UDP port the server sends responses to.
  -s HOST_IP               The IP address of the udp server
  -h                       This help
  -d DEBUG_LEVEL           Enable debugging. (1 for client messages, 2 for server messages, 3 for both)
```

