#include <iostream>
#include <fstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#define CHUNK_SIZE 1024

using namespace std;

// global
int serverPort;
int serverSocket;
int clientSocket;
bool activeSession = true;
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

}


void handleConnection () {

    if(listen(serverSocket,SOMAXCONN)== -1) {
        perror("Listen to client connections failed");
        close(serverSocket);
        exit(1);
    }

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

}

class clientCommunication {
    bool isActiveSession ;
    uint8_t tag;
    void (clientCommunication::*availableCommands[6])();
    char* argument = nullptr;
    uint32_t argumentLen;



public:
    clientCommunication(){
        availableCommands[0] = &clientCommunication::commandGET;
        availableCommands[1] = &clientCommunication::commandLIST;
        availableCommands[2] = &clientCommunication::commandPUT;
        availableCommands[3] = &clientCommunication::commandDELETE;
        availableCommands[4] = &clientCommunication::commandINFO;
        availableCommands[5] = &clientCommunication::commandEXIT;
        isActiveSession = true;

        startNewCommand();


    }

     void commandGET() {
        readUserFile();
        string informClient;

        bool isFileExist = isFileExists(argument,PATHFile);
        send(clientSocket,&isFileExist,sizeof(isFileExist),0);

        if(!isFileExist) {
            informClient = "\033[31mThere is no such file on server\033[0m\n";
            sendStrToClient(informClient);
        }
        bool isToSentFile;

        recv(clientSocket,&isToSentFile,sizeof(isToSentFile),0);
        if(!isToSentFile) return;;



        if (isFileExist) {

            string filePath = string(PATHFile) +"/"+ argument;
            fstream file(filePath);
            sendContentOfFile(file,"\033[33mIn process: sending file\033[0m\n","\033[32mContent of the file was succesfully send.\033[0m\n");


        }
        startNewCommand();
    }

     void commandLIST() {
        string filesOnServer = getStrAvailableFile().c_str();

        sendStrToClient(filesOnServer);
        startNewCommand();

    }

     void commandPUT() {
        string informClient;
        readUserFile();

        bool isFileExistsOnServer = isFileExists(argument,PATHFile);
        send(clientSocket,&isFileExistsOnServer,sizeof(isFileExistsOnServer), 0);
        bool isToOverwrite;
        recv(clientSocket,&isToOverwrite, sizeof(isToOverwrite),0);
        if (!isToOverwrite) return;

        fstream file(string(PATHFile) + "/"+ argument, ios::out);

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
        sendStrToClient(informClient);
        writeInfoLog( string( argument) + ".log",fileSize);
        startNewCommand();

    }

     void commandDELETE() {
        readUserFile();


        string filePath = string(PATHFile) +"/"+ argument;
        string logFilePath = string(PATHFLogFile) +"/"+ argument+ ".log";

        string result ;
        if (isFileExists(argument,PATHFile)) {
            if (::remove(filePath.c_str()) == 0) {
                result =  "\033[32mFile deleted successfully.\033[0m\n" ;
                ::remove(logFilePath.c_str());
            } else {
                result =  "\033[31mFailed to delete the file.\033[0m\n";
            }
        } else {
            result =  "\033[31mThere is no such file on server.\033[0m\n";
        }
        sendStrToClient(result);
        startNewCommand();

    }

     void commandINFO() {
        readUserFile();
        string informClient;

        bool isFileExist = isFileExists(argument, PATHFile);
        send(clientSocket,&isFileExist,sizeof(isFileExist),0);

        if(!isFileExist) {
            informClient = "\033[31mThere is no such file on server\033[0m\n";
            sendStrToClient(informClient);
            startNewCommand();
        }

        string filePath = string(PATHFLogFile) +"/"+ argument + ".log";
        fstream file(filePath,ios::in);
        sendContentOfFile(file,"\033[33mIn process: reading log of file\033[0m\n","\033[32mInfo about file was sent.\033[0m\n");

        startNewCommand();
    }

    void writeInfoLog(string logFile, int sizeByte) {
        string fileLogPath = string(PATHFLogFile) + "/" + logFile;
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

        DIR* dir = opendir(PATHFile);

        bool isDirEmpty = true;
        string listOfFiles ;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (string(entry->d_name) != "." && string(entry->d_name) != "..") {
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

    void readUserFile() {
        recv(clientSocket,&(argumentLen), sizeof(argumentLen),0);
        argument = new char[ntohl(argumentLen) + 1];
        recv(clientSocket,argument,ntohl(argumentLen),0);
        argument[ntohl(argumentLen)] = '\0';
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
        activeSession = false;
        startNewCommand();
    }

    int startNewCommand() {
        if (activeSession) {
            recv(clientSocket,&tag,sizeof(tag),0);
            (this->*availableCommands[static_cast<int>(tag) - 1])();
        }
        return 1;
    }
};


int main() {


    serverSetup();

    handleConnection();



    clientCommunication* client_communication = new clientCommunication();



return 0;

}

