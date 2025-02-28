#ifndef STATISTIC_H
#define STATISTIC_H

#include <fstream>
#include <string>

class statistic {
public:

    static int counterPUT;
    static int counterDELETE;
    static int counterINFO;
    static int counterGET;
    static int counterLIST;


    static void writeStaticticTofile();
};

#endif
