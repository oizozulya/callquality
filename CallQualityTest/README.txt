This application implements call quality test tool.
Receives 3 integer paramenters:
1. Number of calling bots, should be more than 2, but not very high (about 100 is recommended) to not affect system performance.

All timings in applications are random values from range that configured by user, 2 and 3 paramenters are begin value and end value of this range in milliseconds:

2. Begin of the range: more than 0, but not very high (about 60000 = 60 sec is recommended as highest value)
3. End of the range:more than begin range, but also not very high.

Example of starting application from command prompt, from the path where it's located: "CallQualityTest.exe 5 1000 5000", where 5 is number of calling bots and [1000, 5000] is the range.