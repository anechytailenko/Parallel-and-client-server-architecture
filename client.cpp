#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <regex>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <dirent.h>

#define serverPort 8080
#define CHUNK_SIZE 1024
using namespace std;
// global

const char*serverIP = "127.0.0.1";
int clientSocket;
bool activeSession = true;
const char *  PATH = "/Users/annanechytailenko/Desktop/clientPart/FilesLocal";

// global


int clientSetUp() {

    clientSocket = socket(AF_INET, SOCK_STREAM,0);
    if (clientSocket == -1 ) {
        perror("Creation of client socket failed");
        exit(1);
    }


}


int connectServer() {
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    inet_pton(AF_INET,serverIP,&(serverAddr.sin_addr));

    if(connect(clientSocket,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr)) == -1) {
        perror("Connection to the server failed");
        exit(1);
    }
}

string getCommand() {
    string pattern = R"(^(GET|PUT|DELETE|INFO)\s([a-zA-Z0-9_-]+\.txt)$|^LIST$|^EXIT$)";
    regex rgx(pattern);

    string userCommand;
    while (true) {
        cout << "\033[38;2;128;0;128mEnter a command: \033[0m";
        getline(cin,userCommand);


        if (regex_match(userCommand, rgx)) {
            return userCommand;
        }

        cout << "\033[38;2;255;0;0mInvalid command\033[0m" << endl;
    }
}



enum class OpCode: uint8_t {
    GET = 0x01,
    LIST = 0x02,
    PUT = 0x03,
    DELETE = 0x04,
    INFO = 0x05,
    EXIT = 0x06
};


class serverCommunication {
    uint8_t tag;
    char* argument = nullptr;
    uint32_t argumentLen = 0x00;
    bool activeSession  ;
    void (serverCommunication::*availableCommands[6])();
    public :

        serverCommunication() {
        availableCommands[0] = &serverCommunication::commandGET;
        availableCommands[1] = &serverCommunication::commandLIST;
        availableCommands[2] = &serverCommunication::commandPUT;
        availableCommands[3] = &serverCommunication::commandDELETE;
        availableCommands[4] = &serverCommunication::commandINFO;
        availableCommands[5] = &serverCommunication::commandEXIT;
        activeSession = true ;
        startNewCommand();
    }


    OpCode stringToOpCode(const string& str) {

        const unordered_map<string, OpCode> opCodeMap = {
            {"GET", OpCode::GET},
            {"LIST", OpCode::LIST},
            {"PUT", OpCode::PUT},
            {"DELETE", OpCode::DELETE},
            {"INFO", OpCode::INFO},
            {"EXIT", OpCode::EXIT}
        };

        auto op_code = opCodeMap.find(str);
        return op_code->second;
    }

    void sendCommand() {

        send(clientSocket,&tag,sizeof(tag),0) ;

        if (argument != nullptr) {

            send(clientSocket,&(argumentLen),sizeof(argumentLen),0);
            send(clientSocket,argument,strlen(argument),0 );

        }

    }

    void commandGET() {
        bool isFileExistOnServer;
        recv(clientSocket,&isFileExistOnServer,sizeof(isFileExistOnServer),0);


        if (isFileExistOnServer){
            bool isFileExistsLocal = isFileExists(argument);
            bool isOverwrite = true;
            if (isFileExistsLocal) {
                cout << "\033[34mThe file with the same name is stored locally. Do you want to overwrite it [y/n]? \033[0m\n";
                isOverwrite = handleOverwriting();
            }
            send(clientSocket,&isOverwrite,sizeof(isOverwrite),0);
            if (!isOverwrite) return;


            fstream file(string(PATH) + "/"+ argument, std::ios::out);

            uint64_t fileSize;
            recv(clientSocket,&fileSize,sizeof(fileSize),0);
            fileSize = ntohll(fileSize);

            readStrFromServer();
            cout << argument;

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


            readStrFromServer();
            cout << argument;


        } else {

            readStrFromServer();
            cout << argument;
        }
        startNewCommand();
    }





    void commandLIST() {
        readStrFromServer();
        cout << argument;
        startNewCommand();

    }

    void commandPUT() {

        bool isFileExistLocal = isFileExists(argument);

        if (isFileExistLocal) {

            bool isFileExistOnServer;
            recv(clientSocket,&isFileExistOnServer,sizeof(isFileExistOnServer),0);

            bool isToOverwriteFileOnServer = true;
            if(isFileExistOnServer) {
                cout << "\033[34mThe file with the same name is stored on the server. Do you want to overwrite it [y/n]? \033[0m\n";
                isToOverwriteFileOnServer = handleOverwriting();
            }
            send(clientSocket,&isToOverwriteFileOnServer,sizeof(isToOverwriteFileOnServer),0);
            if (!isToOverwriteFileOnServer) return;


            string filePath = string(PATH) +"/"+ argument;
            fstream file(filePath, ios::in);
            file.seekg(0, std::ios::end);
            uint64_t fileSize = htonll(static_cast<uint64_t>(file.tellg()));
            file.seekg(0, std::ios::beg);

            send(clientSocket,&fileSize,sizeof(fileSize),0);
            readStrFromServer();
            cout<< argument;

            if (ntohll(fileSize) != 0) {

                char buffer[1024];
                while (file.read(buffer, CHUNK_SIZE)) {
                    send(clientSocket, buffer, CHUNK_SIZE, 0);
                }
                if (file.gcount() > 0) {
                    send(clientSocket, buffer, file.gcount(), 0);
                }

                file.close();
            }
            readStrFromServer();
            cout << argument;
        } else {
            cout <<  "\033[31mThere is no such file locally\033[0m\n";

        }
        startNewCommand();
    }





    void commandDELETE() {
        readStrFromServer();
        cout << argument;
        startNewCommand();
    }

    void commandINFO() {

        bool isFileExistOnServer;
        recv(clientSocket, &isFileExistOnServer,sizeof(isFileExistOnServer),0);

        if(!isFileExistOnServer) {
            readStrFromServer();
           cout << argument;
        } else {
            uint64_t fileSize;
            recv(clientSocket,&fileSize,sizeof(fileSize),0);
            fileSize = ntohll(fileSize);

            readStrFromServer();
            cout << argument;

            if (fileSize != 0) {
                uint64_t bytesReceived = 0;

                char buffer[1024];
                while (bytesReceived < fileSize) {

                    int bytes = recv(clientSocket, buffer, (CHUNK_SIZE < (fileSize - bytesReceived)) ? CHUNK_SIZE : (fileSize - bytesReceived), 0);

                    if (bytes > 0) {
                        cout << buffer;
                        bytesReceived += bytes;
                    }

                }

            }

            readStrFromServer();
            cout << argument;

        }

        startNewCommand();
    }

    void readStrFromServer() {
        recv(clientSocket,&argumentLen, sizeof(argumentLen),0);
        argument = new char[ntohl(argumentLen)+1];
        recv(clientSocket,argument, ntohl(argumentLen),0);
        argument[ntohl(argumentLen)] = '\0';

    }

    bool isFileExists( const string& fileName) {
        DIR* dir = opendir(PATH);

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

    bool handleOverwriting() {
        bool isFileStoreLocal = isFileExists(argument);
        bool validAnswer = false;
        char answer;
        if (isFileStoreLocal) {
            while (!validAnswer) {
                cin >> answer;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (answer == 'y' || answer == 'Y') {
                    validAnswer = true;
                } else if (answer == 'n' || answer == 'N') {
                    cout << "\033[38;2;255;0;0mOperation canceled.\033[0m\n";
                    return false;
                } else {
                    cout << "\033[38;2;255;0;0mmInvalid input.\033[0m\n";
                }
            }
        }
        return true;
    }


    void commandEXIT() {
        activeSession = false;
        startNewCommand();
    }


    int startNewCommand() {
        if (activeSession){
            argument = nullptr;
            string userCommand = getCommand();
            istringstream commandStream(userCommand);
            string tagStr, argStr;

            commandStream >> tagStr ;
            tag = static_cast<uint8_t>(stringToOpCode(tagStr));

            if (commandStream >> argStr) {
                argument = new char[argStr.length() + 1];
                strcpy(argument,argStr.c_str());
                argumentLen = htonl(strlen(argument));
            }

            sendCommand();

            (this->*availableCommands[static_cast<int>(tag) - 1])();
        }
        return 1;
    }
};

int main()
{
    clientSetUp();
    connectServer();


  // Assignment start

    // command : GET <filename> - 0x01; LIST - 0x02; PUT <filename> - 0x03; DELETE <filename> - 0x04; INFO <filename> - 0x05



     serverCommunication* server_communication = new serverCommunication();




    return 0;
}
