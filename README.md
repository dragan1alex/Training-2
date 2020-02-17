# Training-2
Use of message queues for inter-process communication
Build with
```
gcc -o messages messages.c -lrt
```

The app has 2 modes, server and client. 
To run in server mode, pass any argument to it (./messages asd), to run as a client run it without arguments.

NOTE: If there are any errors reported run the app as root.
Ubuntu doesn't allow users to create or modify message queues in "/dev/mqueue" (if you haven't changed the permissions for that directory)
