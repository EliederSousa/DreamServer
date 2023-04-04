# DreamServer

This is a... simple server created with love, and C++.

I always dreamed of learning how to create a full customizable C++ server with sockets. So, this is a dream, that came true.  

![image](https://user-images.githubusercontent.com/16262291/229853049-77625513-320d-4950-ab74-06e30f03b72e.png)


## Features

Despite a lot of errors, poorly optimized code and structure among other things, I can list some of the good things that this server has:

* Nice console window created with fmt library, featuring:
  * Customizable number of log messages in console window, that are shown as numbered lines for easy recognition;
  * Four different types of message (Success, Info, Warning, Error), each one with it's own icon;
  * All log messages are saved in a log file:  
  ![image](https://user-images.githubusercontent.com/16262291/229858867-eb4a527d-6365-46fa-992f-15e66524abb5.png)  

  * A status bar showing useful stats:  
  ![image](https://user-images.githubusercontent.com/16262291/229857765-6f298c9e-64ba-41d8-9125-2c063fe30d0c.png)  


* An INI file lets you configure limits and common server settings like port, maximum number of clients, etc;
* Debug Mode: Lets you bypass some standard blocking settings like duplicated IPs, etc;


## Next ideas and TODOs

* More status icons (thread workload, CPU consuption);
* An embed Finite State Machine to improve application workflow;
* Implementing rooms;
* Deal with dataraces;
* Implementing thread-safe solutions;
* Reorganize code structure;
* More configurations in INI file, less hardcoded values.
