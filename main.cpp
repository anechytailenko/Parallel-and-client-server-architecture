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
#include <filesystem>






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
    string pattern = R"(^(GET|PUT|DELETE|INFO)\s\w+\s([a-zA-Z0-9_-]+\.txt)$|^LIST\s\w+$|^EXIT$)";
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
    char* fileFolder = nullptr;
    uint32_t argumentLen = 0x00;
    uint32_t fileFolderLen = 0x00;
    bool activeSession ;
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
        askClientName();
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

        send(clientSocket,&tag,sizeof(tag),0);

        if (fileFolder!= nullptr) {
            send(clientSocket,&(fileFolderLen),sizeof(fileFolderLen),0);
            send(clientSocket,fileFolder,strlen(fileFolder),0);
        }

        if (argument != nullptr) {
            send(clientSocket,&(argumentLen),sizeof(argumentLen),0);
            send(clientSocket,argument,strlen(argument),0 );
        }

    }

    void commandGET() {

        bool isDirExistOnServer;
        recv(clientSocket,&isDirExistOnServer,sizeof(isDirExistOnServer),0);
        bool isFileExistOnServer;
        recv(clientSocket,&isFileExistOnServer,sizeof(isFileExistOnServer),0);


        if (isFileExistOnServer && isDirExistOnServer) {
            bool isFileExistsLocal = isFileExists(argument);
            bool isOverwrite = true;
            if (isFileExistsLocal) {
                cout << "\033[34mThe file with the same name is stored locally. Do you want to overwrite it [y/n]? \033[0m\n";
                isOverwrite = handleOverwriting();
            }
            send(clientSocket,&isOverwrite,sizeof(isOverwrite),0);


            if (isOverwrite) {

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


            }
        } else {
            readStrFromServer();
            cout << argument;
        }
        startNewCommand();
    }





    void commandLIST() {
        bool isDirOnServer;
        recv(clientSocket,&(isDirOnServer),sizeof(isDirOnServer),0);
        readStrFromServer();
        cout << argument;
        startNewCommand();

    }

    void commandPUT() {
        bool isDirExistOnServer;
        recv(clientSocket,&isDirExistOnServer,sizeof(isDirExistOnServer),0);
        if (isDirExistOnServer) {
            bool isFileLocalExist = isFileExists(argument);
            send(clientSocket,&isFileLocalExist,sizeof(isFileLocalExist),0);
            if(isFileLocalExist) {
                bool isFileExistOnServer;
                recv(clientSocket,&isFileExistOnServer,sizeof(isDirExistOnServer),0);
                bool isOverwrite = true;
                if(isFileExistOnServer) {
                    cout << "There is the file with the same name on the server.Do you want to overwrite it?[y/n]"<< endl;
                    isOverwrite = handleOverwriting();
                }
                send(clientSocket,&isOverwrite,sizeof(isOverwrite),0);
                if(isOverwrite) {
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
                }
            }
        }
        readStrFromServer();
        cout << argument;
        startNewCommand();

        }





    void commandDELETE() {
        readStrFromServer();
        cout << argument;
        startNewCommand();
    }

    void commandINFO() {

        bool isDataAvailable ;
        recv(clientSocket,&isDataAvailable,sizeof(isDataAvailable),0);

        if(isDataAvailable) {
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

        } else {
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
                    return false;
                } else {
                    cout << "\033[38;2;255;0;0mmInvalid input.\033[0m\n";
                }
            }
        }
        return true;
    }


    void commandEXIT() {
        close(clientSocket);
        activeSession = false;
        startNewCommand();
    }


    int startNewCommand() {
        if (activeSession){
            argument = nullptr;
            string userCommand = getCommand();
            istringstream commandStream(userCommand);
            string tagStr, folderName, fileName ;

            commandStream >> tagStr ;
            tag = static_cast<uint8_t>(stringToOpCode(tagStr));

            if (commandStream >> folderName) {
                fileFolder = new char[folderName.length() + 1];
                strcpy(fileFolder,folderName.c_str());
                fileFolderLen = htonl(strlen(fileFolder));
            }

            if (commandStream >> fileName) {
                argument = new char[fileName.length() + 1];
                strcpy(argument,fileName.c_str());
                argumentLen = htonl(strlen(argument));
            }


            sendCommand();

            (this->*availableCommands[static_cast<int>(tag) - 1])();
        }
        return 1;
    }


    void askClientName() {
        cout << "\033[38;2;128;0;128mEnter your name: \033[0m";
        string clientName;
        getline(cin,clientName);
        sendStrToServer(clientName);
        readStrFromServer();
        cout << argument;
    }

    void sendStrToServer(string entity) {
        uint32_t sizeOfString = htonl(entity.length());
        send(clientSocket,&sizeOfString, sizeof(sizeOfString),0);
        send(clientSocket,entity.c_str(),entity.length(),0);


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
