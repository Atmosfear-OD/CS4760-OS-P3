P3 by Pascal Odijk 10/3/2021
CMPSCI 4760 Prof. Bhatia

--------------------------------------------------------
To compile program:
make

To clean:
make clean
--------------------------------------------------------
Method of invocation:
	1) ./runsim n < testing.data 	     // Where n is an integer greater than or equal to 1. This will be used as the number of available licenses.
	2) ./runsim -t sec n < testing.data  // Where sec is an integer greater than or equal to 0. This will be the runtime for runsim.

/* Note 1: The parent process takes away 1 license, so if n = 1, only one license will be available and no children will be able to be 
 	   forked so program will wait for default 100 seconds and then terminate without printing anything to logfile.data. 
   Note 2: If -t sec is initialized with 0, the program will terminate immediatly, since the program will be given 0 seconds to run!
   Note 3: If -t is not specified the default runtime is 100 seconds.
*/ 
--------------------------------------------------------
Problems I encountered:
1. The instructions were very confusing -- I tried my best and from what I can tell, everything works fine
2. I would have liked to have my Project 2 graded before submitting Project 3 so I could have made any necessary improvements...hopefully no problems carried over
---------------------------------------------------------
Github version control web address:
https://github.com/Atmosfear-OD/CS4760-OS-P3
