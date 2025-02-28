#include <iostream>
#include <fstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include "statistic.h"
#include <vector>
#include <thread>
#include <queue>
#define CHUNK_SIZE 1024

using namespace std;

//stat
int statistic::counterPUT = 0;
int statistic::counterDELETE = 0;
int statistic::counterINFO = 0;
int statistic::counterGET = 0;
int statistic::counterLIST = 0;

void statistic::writeStaticticTofile() {
    std::ofstream file("Statistic.txt");
    std::string result = "PUT: " + std::to_string(counterPUT) +
                         "\nDELETE: " + std::to_string(counterDELETE) +
                         "\nINFO: " + std::to_string(counterINFO) +
                         "\nGET: " + std::to_string(counterGET) +
                         "\nLIST: " + std::to_string(counterLIST);

    file << result;
    file.close();
}

//stat

// global
int serverPort;
int serverSocket;
int clientSocket;
char* PATHGlobal = "/Users/annanechytailenko/Desktop/serverPart/";
char* PATHFile = "/Users/annanechytailenko/Desktop/serverPart/clientFiles";
char* PATHFLogFile = "/Users/annanechytailenko/Desktop/serverPart/clientLogFiles";
// global



int serverSetup() {
    serverPort= 8080;
    serverSocket = socket(AF_INET,SOCK_STREAM,0);
    if (serverSocket==-1) {
        perror("Creation of server socket failed");
       exit(1);
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPort);

    if(::bind(serverSocket,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr))== -1) {
        perror("Bind of server socket failed");
        close(serverSocket);
        exit(1);
    }


    if(listen(serverSocket,SOMAXCONN)== -1) {
        perror("Listen to client connections failed");
        close(serverSocket);
        exit(1);
    }

}


int handleConnection () {


    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(serverSocket,reinterpret_cast<sockaddr *>(&clientAddr),&clientAddrLen);

    if(clientSocket == -1){
        perror("Accepting connection failed");
        close(serverSocket);
        exit(1);
    }
    cout << "Client Socket: " << clientSocket << endl;

    // optional
    cout << "Connection from client was accepted "<< inet_ntoa(clientAddr.sin_addr)<< " : "<<  ntohs(clientAddr.sin_port)<< endl;
    // optional
    return clientSocket;

}

class clientCommunication {
    bool isActiveSession ;
    uint8_t tag;
    void (clientCommunication::*availableCommands[6])();
    char* argument = nullptr;
    string clientFolder;
    char* fileFolder = nullptr;
    uint32_t argumentLen = 0x00;
    uint32_t fileFolderLen = 0x00;
    int clientSocket;


public:
    clientCommunication(int clientSocket): clientSocket(clientSocket){
        availableCommands[0] = &clientCommunication::commandGET;
        availableCommands[1] = &clientCommunication::commandLIST;
        availableCommands[2] = &clientCommunication::commandPUT;
        availableCommands[3] = &clientCommunication::commandDELETE;
        availableCommands[4] = &clientCommunication::commandINFO;
        availableCommands[5] = &clientCommunication::commandEXIT;
        isActiveSession = true;

    }
    void startCommunication() {
        handleSubDir();
        startNewCommand();
    }
    void discardCommunication() {
        string message = "Server discard the client\n";
        close(clientSocket);
    }

     void commandGET() {
        readDirFromClient();
        readStrFromClient();
        string informClient;

        bool isDirExist = isFileExists(fileFolder,PATHGlobal);
        send(clientSocket,&(isDirExist),sizeof(isDirExist),0);
        bool isFileExist = false;
        if(isDirExist) {
            isFileExist = isFileExists(argument,getDirPath());
            send(clientSocket,&(isFileExist),sizeof(isFileExist),0);
        }
        if(!isDirExist) {
            informClient = "\033[31mThere is no such directory on server\033[0m\n";
            sendStrToClient(informClient);
        } else if(!isFileExist) {
            informClient = "\033[31mThere is no such file on server\033[0m\n";
            sendStrToClient(informClient);
        }




        if (isFileExist  && isDirExist) {
            bool isToSentFile;
            recv(clientSocket,&isToSentFile,sizeof(isToSentFile),0);
            if(isToSentFile) {
                string filePath = string(getDirPath()) +"/"+ argument;
                fstream file(filePath, ios::in);
                sendContentOfFile(file,"\033[33mIn process: sending file\033[0m\n","\033[32mContent of the file was succesfully send.\033[0m\n");
            }
        }
        statistic ::counterGET ++;
        statistic::writeStaticticTofile();
        startNewCommand();
    }

     void commandLIST() {
        readDirFromClient();
        bool isDirExist = isFileExists(fileFolder,PATHGlobal);
        send(clientSocket,&(isDirExist),sizeof(isDirExist),0);
        string result;
        if (isDirExist) {
            result = getStrAvailableFile().c_str();
            sendStrToClient(result);

        } else {
            result= "\033[31mThere is no such directory on server\033[0m\n";
            sendStrToClient(result);

        }
        statistic ::counterLIST ++;
        statistic::writeStaticticTofile();
        startNewCommand();

    }

     void commandPUT() {
        readDirFromClient();
        readStrFromClient();
        string informClient;

        bool isDirExist = isFileExists(fileFolder,PATHGlobal);
        send(clientSocket,&(isDirExist),sizeof(isDirExist),0);
        if (isDirExist) {
            bool isFileExistLocal ;
            recv(clientSocket,&isFileExistLocal,sizeof(isFileExistLocal),0);
            if (isFileExistLocal) {
                bool isFileExistOnServer = isFileExists(argument,getDirPath());
                send(clientSocket,&isFileExistOnServer,sizeof(isFileExistOnServer),0);
                bool isOverwrite = true;

                recv(clientSocket,&isOverwrite,sizeof(isOverwrite),0);

                if(isOverwrite) {
                    fstream file(string(getDirPath()) + "/"+ argument, ios::out);

                    uint64_t fileSize;
                    recv(clientSocket,&fileSize,sizeof(fileSize),0);
                    fileSize = ntohll(fileSize);
                    informClient = "\033[33mIn process: downloading file\033[0m\n";
                    sendStrToClient(informClient);

                    if (fileSize != 0) {
                        uint64_t bytesReceived = 0;

                        char buffer[1024];
                        while (bytesReceived < fileSize) {
                            int bytes = recv(clientSocket, buffer, (CHUNK_SIZE < (fileSize - bytesReceived)) ? CHUNK_SIZE : (fileSize - bytesReceived), 0);

                            if (bytes > 0) {
                                file.write(buffer, bytes);
                                bytesReceived += bytes;
                            }
                        }
                        file.close();
                    }
                    informClient = "\033[32mContent of the file was succesfully uploading.\033[0m\n";
                    writeInfoLog( string( argument) + ".log",fileSize);
                } else {
                        informClient = "Operation was cancelled\n";
                    }
                } else {
                    informClient = "There is no such file locally\n";
                }
            } else {
                informClient = "There is no such directory on the server\n";
            }
        sendStrToClient(informClient);
        statistic ::counterPUT ++;
        statistic::writeStaticticTofile();
        startNewCommand();

    }

     void commandDELETE() {
        readDirFromClient();
        readStrFromClient();

        string result ;
        bool isDirExist = isFileExists(fileFolder,PATHGlobal);
        if(isDirExist) {
            string filePath = string(getDirPath()) +"/"+ argument;
            string logFilePath =  string(getDirPath()) +"/logFiles/"+ argument + ".log";
            if (isFileExists(argument,getDirPath())) {
                if (::remove(filePath.c_str()) == 0) {
                    result =  "\033[32mFile deleted successfully.\033[0m\n" ;
                    ::remove(logFilePath.c_str());
                } else {
                    result =  "\033[31mFailed to delete the file.\033[0m\n";
                }
            } else {
                result =  "\033[31mThere is no such file on server.\033[0m\n";
            }
        } else {
            result  = "\033[31mThere is no such directory on server.\033[0m\n";
        }
        sendStrToClient(result);
        statistic ::counterDELETE ++;
        statistic::writeStaticticTofile();
        startNewCommand();

    }

     void commandINFO() {
        readDirFromClient();
        readStrFromClient();
        string result;
        bool isToSent = false;
        bool isDirExist = isFileExists(fileFolder, PATHGlobal);
        if(isDirExist) {
            bool isFileExist = isFileExists(argument, getDirPath());
            if (isFileExist) {
                isToSent = true;
                send(clientSocket,&isToSent, sizeof(isToSent), 0);
                string filePath = string(getDirPath()) +"/logFiles/"+ argument + ".log";
                fstream file(filePath,ios::in);
                sendContentOfFile(file,"\033[33mIn process: reading log of file\033[0m\n","\033[32mInfo about file was sent.\033[0m\n");
            } else {
                result = "\033[31mThere is no such file on server\033[0m\n";
            }
        } else {
            result =  "\033[31mThere is no such directory on server\033[0m\n";
        }
        sendStrToClient(result);

        statistic ::counterINFO ++;
        statistic::writeStaticticTofile();
        startNewCommand();
    }

    void writeInfoLog(string logFile, int sizeByte) {
        string fileLogPath = string(getDirPath()) + "/" + "logFiles/"+ logFile;
        time_t now = time(nullptr);
        fstream logging(fileLogPath, ios::in);

        if (!logging) {
            logging.open(fileLogPath, ios::out);
            logging << "[LOG]: Created; Size: " << sizeByte << " Bytes; Last modified: " << ctime(&now);
            logging.close();
        } else {
            logging.close();
            logging.open(fileLogPath, ios::out | ios::app);
            logging << "[LOG]: Updated; Size: " << sizeByte << " Bytes; Last modified: " << ctime(&now);
            logging.close();
        }

    }


     string getStrAvailableFile() {

        DIR* dir = opendir(getDirPath());

        bool isDirEmpty = true;
        string listOfFiles ;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (string(entry->d_name) != "." && string(entry->d_name) != ".." && string(entry->d_name) != "logFiles") {
                listOfFiles+="\033[38;5;214m>" + string(entry->d_name) + "\033[0m;\n";
                isDirEmpty = false;
            }
        }
        closedir(dir);

        string resultStr;
        if (!isDirEmpty) {
            resultStr = "\033[32mFiles on the server:\n\033[0m" + listOfFiles;
        } else {
            resultStr = "\033[31mThere is no files on the server\033[0m\n";
        }
        return resultStr;
    }

    void readDirFromClient() {
        delete[] fileFolder;
        recv(clientSocket,&(fileFolderLen), sizeof(fileFolderLen),0);
        fileFolder = new char[ntohl(fileFolderLen) + 1];
        recv(clientSocket,fileFolder,ntohl(fileFolderLen),0);
        fileFolder[ntohl(fileFolderLen)] = '\0';
    }

    void readStrFromClient() {
        delete[] argument;
        recv(clientSocket,&(argumentLen), sizeof(argumentLen),0);
        argument = new char[ntohl(argumentLen) + 1];
        recv(clientSocket,argument,ntohl(argumentLen),0);
        argument[ntohl(argumentLen)] = '\0';
    }

    char* getDirPath() {
        string dirPathStr = string(PATHGlobal) + fileFolder;
        char* dirPath = new char[dirPathStr.length() + 1];
        strcpy(dirPath, dirPathStr.c_str());
        return dirPath;
    }

    void sendStrToClient(string& entity) {
        argumentLen = htonl(entity.length());

        send(clientSocket,&argumentLen,sizeof(argumentLen),0);
        send(clientSocket,entity.c_str(),entity.length(),0);
    }

    bool isFileExists( const string& fileName, char *path) {
        DIR* dir = opendir(path);

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {

            if (entry->d_name == fileName) {
                closedir(dir);
                return true;
            }
        }
        closedir(dir);
        return false;
    }


    uint64_t getSizeOfFile(fstream& file){
        file.seekg(0, ios::end);
        uint64_t fileSize = static_cast<uint64_t>(file.tellg());
        file.seekg(0, ios::beg);
       return fileSize;
    }

    void sendContentOfFile(fstream& file, string infrormStart, string informEnd) {

        uint64_t fileSize = htonll(getSizeOfFile(file));

        send(clientSocket,&fileSize,sizeof(fileSize),0);


        sendStrToClient(infrormStart);


        if (ntohll(fileSize) != 0) {

            char buffer[1024];
            while (file.read(buffer, CHUNK_SIZE)) {
                send(clientSocket, buffer, CHUNK_SIZE, 0);
            }
            if (file.gcount() > 0) {
                send(clientSocket, buffer, file.gcount(), 0);
            }

            file.close();

            sendStrToClient(informEnd);
        }

    }


    void commandEXIT() {
        isActiveSession = false;
        startNewCommand();
    }

    int startNewCommand() {
        if (isActiveSession) {
            recv(clientSocket,&tag,sizeof(tag),0);
            (this->*availableCommands[static_cast<int>(tag) - 1])();
        }
        return 1;
    }

    void handleSubDir() {
        readStrFromClient();
        bool isDirExist = isFileExists(argument,PATHGlobal);
        string Message;
        if(isDirExist) {
            Message = "\033[38;5;214mWelcome old user\n\033[0m";
        }else {
            Message = "\033[38;5;214mWelcome new user\n\033[0m";
            clientFolder = string(PATHGlobal) + argument;
            mkdir(clientFolder.c_str(), 0777);
            string folderLog = clientFolder + "/logFiles";
            mkdir(folderLog.c_str(), 0777);

        }
        sendStrToClient(Message);
    }

    ~clientCommunication() {

        if (argument != nullptr) {
            delete[] argument;
        }

        if (fileFolder != nullptr) {
            delete[] fileFolder;
        }
    }



};

class ThreadPool {

    vector<thread> workers;
    queue<function<void()>> tasksQueue;
    mutex queuemtx;
    condition_variable cv;
    atomic<bool> isTerminate;
    size_t maxQueueSize;

public:
    ThreadPool(size_t numThreads, size_t maxQueueSize): isTerminate(false), maxQueueSize(maxQueueSize) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    function<void()> task;
                    {
                        unique_lock<std::mutex> lock(queuemtx);
                        cv.wait(lock, [this] {
                            return this->isTerminate || !this->tasksQueue.empty();
                        });
                        if (isTerminate && tasksQueue.empty())
                            return;
                        task = move(tasksQueue.front());
                        tasksQueue.pop();
                    }
                    task();
                }
            });
        }
    };

    bool enqueueTask(std::function<void()> task) {
        {
            unique_lock<mutex> lock(queuemtx);
            if (tasksQueue.size() >= maxQueueSize)
                return false;
            tasksQueue.emplace(task);
        }
        cv.notify_one();
        return true;
    }


    ~ThreadPool() {
        isTerminate = true;
        cv.notify_all();
        for (thread &worker : workers)
            worker.join();
    };

};






void handle_client(int clientSocket) {

    clientCommunication client_communication = clientCommunication(clientSocket);
    client_communication.startCommunication();
    cout <<  this_thread::get_id()<< endl;

}



int main() {

    serverSetup();

    const size_t numThreads = 1;
    const size_t maxQueueSize = 1;

    ThreadPool pool(numThreads, maxQueueSize);

    while(true) {
        int clientSocket = handleConnection();

        auto task = [clientSocket]() { handle_client(clientSocket); };
        if (!pool.enqueueTask(task)) {


            clientCommunication client_communication = clientCommunication(clientSocket);
            client_communication.discardCommunication();

        }
    }

return 0;

}
