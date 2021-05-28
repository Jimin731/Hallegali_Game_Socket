#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock.h>
#include <windows.h>
#include <time.h>




#pragma comment(lib, "wsock32.lib")   
#define PORT 5000         // �����Ʈ�� 5000
#define MAX_CLIENT 3  // �ִ� ��� �ο� �� 3��
#define ALLOW 65535         // �ִ� ���� ���� ���� ��ȣ 65535
#define MAX_CARD_FRUIT 4
#define MAX_CARD_NUM 5

int COUNT = 0; // ī�� �� Ƚ�� ����


void recv_client(void* ns);   // ������ �Լ� ������ Ÿ��
void playgame();
void broadcast_all(char* mess);      //��ü �޼��� ������ �Լ�
void broadcast_cardnum();         //���� ī�� ���� �˷��ִ� �Լ�
void pressBell(int i);              //ī�� �Ǵ� ���� �Է����� �� ����Ǵ� �Լ�
void randomcard(int i);             //ī�带 �����ϰ� �������ִ� �Լ�
void broadcast_cardinfo();          //ī���� ������ �����ִ� �Լ�
void resetFruitState();             //���� ������ ��, ������� ī�� ������ �ʱ�ȭ �����ִ� �Լ�
void finish(int i);                      //ī�带 ������ ��, ī���� ���� ���ų� 0�̵Ǿ����� �˷��ֱ�
void winner();
void voidBuffer(SOCKET s);
int client_num = 0;         // ���� Ƚ�� (Ŭ���̾�Ʈ ����)
int seat = 0;            // Ŭ���Ʈ ��ȣ
char welcome_ok[] = "User Welcome.\n\0";   // Welcome ���� �ʱ� ��
char welcome_full[] = "Can't Connected.(FULLY OCCUPIED)\n";               // Welcome ����� �ʰ��� ����� ��
char game_start[] = "\n-------------------------------------------\n3/3 Game Start.\n card / bell Enter \n";  //������ �����Ѵٰ� ����ڿ��� ����
int client_sock[ALLOW];      // client_sock (Ŭ���̾�Ʈ Welcome Socket)
HANDLE hMutex = 0;         // ���ý�
int po = 0;      //��Ʈ�ѹ� �޾��ִ� ����
int turn = 9;


typedef struct card_state {
    char Fruit[10] = { 0 };
    int F_num = 0;
};

typedef struct player {
    int socket_num = 0;
    int card_num = 30;
    int player_id = 0;
    struct card_state Card_state;
}PLAYER;


const char* fruit[MAX_CARD_FRUIT] = { "Banana", "Strawberry", "Grape", "Apple" };


PLAYER p[MAX_CLIENT];

int main()
{
    int i = 0;
    p[0].player_id = 1;
    p[1].player_id = 2;
    p[2].player_id = 3;
    // Welcome Screen
    printf("+---------------------------+\n");
    printf("+ Halli Galli Game          +\n");
    printf("+ Server                    +\n");
    printf("+---------------------------+\n");



    // ���ý� ����
    hMutex = CreateMutex(NULL, FALSE, NULL);   // ���� ���н� ����
    if (!hMutex)
    {
        printf("Mutex error\n");
        CloseHandle(hMutex);
        return 1;
    }

    // ���� �ʱ�ȭ
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0)   // ��� ���� ������ 1.1
    {
        printf("Winsock error\n");
        WSACleanup();
        return 1;

    }

    // Listen ���� ����
    int s, addrsize, ret;
    sockaddr_in server, client;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == SOCKET_ERROR)
    {
        printf("socket() error\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Bind �ϱ�
    if (bind(s, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("bind() error\n");
        closesocket(s);
        WSACleanup();
        return 1;

    }

    printf("Waiting for client.. %d left\n", MAX_CLIENT - client_num);
    listen(s, 10);
    addrsize = sizeof(client);

    // ������� ������ ��ٸ��ϴ�.
    while (1)
    {
        // Blocking ������� Client �� ��ٸ��ϴ�.
        client_sock[seat] = accept(s, (sockaddr*)&client, &addrsize);

        // accept ��(�߿�, client_num �� accept() �Լ� ������ �� ���Ҽ� �����Ƿ�
        // MAX_CLIENT ���޽ö� ���� accept() �� ���� �߻� ���ɼ� ����
        if (client_num < MAX_CLIENT - 1)      // ���� ���� �ϱ�
        {

            if (client_sock[seat] != INVALID_SOCKET || client_sock[seat] != SOCKET_ERROR) {}
            _beginthread(recv_client, 0, &client_sock[seat]);
            Sleep(10);
            printf("User %d client connected from %s:%d\n", seat, inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        }

        else if (client_num == MAX_CLIENT - 1) //���� ����
        {
            if (client_sock[seat] != INVALID_SOCKET || client_sock[seat] != SOCKET_ERROR) {}
            _beginthread(recv_client, 0, &client_sock[seat]);
            Sleep(10);
            printf("User %d client connected from %s:%d \n", seat, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            printf(game_start);
            seat = 0;
            for (int i = 0; i < MAX_CLIENT; i++) {
                ret = send(client_sock[seat], game_start, sizeof(game_start), 0);
                seat++;
            }

            //���� ����
            if (seat == MAX_CLIENT) {
                playgame();
                if (seat == MAX_CLIENT - 1) {           //�߰�
                    playgame();
                }
            }
        }

        else   // ���� á��. ���̻� ��������
        {
            addrsize = sizeof(client);
            if (client_sock[seat] == INVALID_SOCKET)
            {
                printf("accept() error\n");
                closesocket(client_sock[seat]);
                closesocket(s);
                WSACleanup();
                return 1;
            }

            ret = send(client_sock[seat], welcome_full, sizeof(welcome_full), 0);
            closesocket(client_sock[seat]);

            // �޽��� ������ �ٷ� ���´�.

        }
    }

    return 0;

}


void recv_client(void* ns)
{
    // ���������� �޾� �帱��, ������ ����
    // Ŭ���̾�Ʈ�� ���ڸ� �ø�
    WaitForSingleObject(hMutex, INFINITE);
    client_num++;				// Ŭ���̾�Ʈ ���� ����
    seat++;						// Ŭ���̾�Ʈ ��ȣ ����
    printf("%d Users left.. \n", MAX_CLIENT - client_num);    	// ������ �Ǵ�

    ReleaseMutex(hMutex);


    char welcome[100] = { 0 };		// accept �� ���Ͽ��� �� ���� ����
//    char buff[1000000] = { 0 };
    int ret, i, j = 0;


    _itoa_s(seat, welcome, 10);			// Ŭ���̾�Ʈ ��ȣ
    strcat_s(welcome, welcome_ok);		// ���� ȯ�� �޽��� ȯ��
    ret = send(*(SOCKET*)ns, welcome, sizeof(welcome), 0);		// ����
    p[po].socket_num = *(SOCKET*)ns;                  //Ŭ���̾�Ʈ ��Ʈ �ѹ� ����
    po++;
    while (ret != SOCKET_ERROR || ret != INVALID_SOCKET)
    {
        static char buff[1000000] = { 0 };
        ret = recv(*(SOCKET*)ns, buff, 1000000, 0);		// Ŭ���̾�Ʈ�� �޽����� ����

        // broadcast �κ�
        for (i = 0; i < MAX_CLIENT; i++)
        {
            // ���� Ŭ���̾�Ʈ ������ �޸� �ּҿ� ������ Ŭ���̾�Ʈ ���� �޸� �ּҰ� �ٸ����� ����
            char bell[10] = "bell";
            char card[10] = "card";
            char textmessage[100];              //ī��� ���� ������ ����ڰ� ������ ä�ø޼���
            if (strstr(buff, card) != NULL) {                       //Ư�� ����ڰ� ī�带 ������ �� ����ڿ��� �˷��ְ� ī����� ���δ�

                if (*(SOCKET*)ns == p[0].socket_num)
                {
                    finish(0);
                    printf("** USER %d CHOSSE CARD! ** \n", p[0].player_id);
                    if (p[0].card_num != 0) {
                        if (turn == 1) {
                            randomcard(0);
                            broadcast_cardinfo();
                            p[0].card_num--;
                            break;
                        }
                        else {
                            char message1[100] = "NOT YOUR TURN! \n";
                            int ret = send(p[0].socket_num, message1, sizeof(message1), 0);
                            break;
                        }
                    }
                }
                else if (*(SOCKET*)ns == p[1].socket_num)
                {
                    finish(1);
                    printf("** USER %d CHOSSE CARD! ** \n", p[1].player_id);
                    if (p[1].card_num != 0) {
                        if (turn == 2) {
                            randomcard(1);
                            broadcast_cardinfo();
                            p[1].card_num--;
                            break;
                        }
                        else {
                            char message1[100] = "NOT YOUR TURN! .\n";
                            int ret = send(p[1].socket_num, message1, sizeof(message1), 0);
                            break;
                        }

                    }
                }
                else if (*(SOCKET*)ns == p[2].socket_num)
                {
                    finish(2);
                    printf("** USER %d CHOOSE CARD! ** \n", p[2].player_id);
                    if (p[2].card_num != 0) {
                        if (turn == 3) {
                            randomcard(2);
                            broadcast_cardinfo();
                            p[2].card_num--;
                            break;
                        }
                        else {
                            char message1[100] = "NOT YOUR TURN! .\n";
                            int ret = send(p[2].socket_num, message1, sizeof(message1), 0);
                            break;
                        }

                    }
                }

            }
            if (strstr(buff, bell) != NULL) {                   //Ư�� ����ڰ� ���� ������ �� 

                if (*(SOCKET*)ns == p[0].socket_num)
                {
                    printf("** User %d choose BELL! ** \n", p[0].player_id);
                    if (p[0].card_num >= 0) {
                        pressBell(0);
                        finish(0);
                        resetFruitState();
                        broadcast_cardinfo();

                        broadcast_cardnum();
                    }


                    break;
                }
                else if (*(SOCKET*)ns == p[1].socket_num)
                {
                    printf("** User %d choose BELL! ** \n", p[1].player_id);
                    if (p[1].card_num >= 0) {
                        pressBell(1);
                        finish(1);
                        resetFruitState();
                        broadcast_cardinfo();
                        broadcast_cardnum();
                    }

                    break;
                }
                else
                {
                    printf("** User %d choose BELL! ** \n", p[2].player_id);
                    if (p[2].card_num >= 0) {
                        pressBell(2);
                        finish(2);
                        resetFruitState();
                        broadcast_cardinfo();
                        broadcast_cardnum();
                    }


                    break;
                }

            }

            if (strstr(buff, card) == NULL && strstr(buff, bell) == NULL) {                     //ä�� ������
                if (*(SOCKET*)ns == p[0].socket_num)
                {
                    printf("User %d : %s \n", p[0].player_id, buff);


                    char message2[100];
                    _snprintf(message2, sizeof(message2), "User %d : %s", p[0].player_id, buff);
                    for (int j = 0; j < MAX_CLIENT; j++) {
                        if (j != 0) {
                            int ret = send(p[j].socket_num, message2, sizeof(message2), 0);
                        }
                    }

                    break;
                }
                else if (*(SOCKET*)ns == p[1].socket_num)
                {
                    printf("User %d : %s \n", p[1].player_id, buff);

                    char message2[100];
                    _snprintf(message2, sizeof(message2), "User %d : %s", p[1].player_id, buff);
                    for (int j = 0; j < MAX_CLIENT; j++) {
                        if (j != 1) {
                            int ret = send(p[j].socket_num, message2, sizeof(message2), 0);
                        }
                    }

                    break;
                }
                else
                {
                    printf("User %d : %s \n", p[2].player_id, buff);
                    char message2[100];
                    _snprintf(message2, sizeof(message2), "User %d : %s", p[2].player_id, buff);
                    for (int j = 0; j < MAX_CLIENT; j++) {
                        if (j != 2) {
                            int ret = send(p[j].socket_num, message2, sizeof(message2), 0);
                        }
                    }

                    break;
                }
            }


            ReleaseMutex(hMutex);

        }



        memset(buff, 0, 1000000);

    }

    // ���ӵ� ������ ������ ���� ��������
    WaitForSingleObject(hMutex, INFINITE);
    client_num--;
    printf("User %d gone! \n %d Users left ..\n", seat, MAX_CLIENT - client_num);
    ReleaseMutex(hMutex);

    closesocket(*(int*)ns);

    return;
}

void voidBuffer(SOCKET s) {
    u_long tmpl, i;
    char tmpc;
    ioctlsocket(s, FIONREAD, &tmpl);
    for (i = 0; i < tmpl; i++) recv(s, &tmpc, sizeof(char), 0);
}

void finish(int i) { // card ���� �� ó����
    /*if (p[i].card_num < 6 && p[i].card_num >=0 ) { // 5�� ���� ���
        char message[50];
        sprintf_s(message, "%d player's Number Of Cards: %d", p[i].player_id, p[i].card_num);
        for (int j = 0; j < MAX_CLIENT; j++) { // ��ο��� �˸���
            int ret = send(p[j].socket_num, message, sizeof(message), 0);
        }
    }*/
    if (p[i].card_num < 0) { // ī�� ���� ���� ����


        printf("%d player kicked! \n", p[i].player_id);
        for (int j = 0; j < MAX_CLIENT; j++) { // ��ο��� �˸���
            char message[50] = { 0 };
            _snprintf(message, sizeof(message), "%d player kicked! \n", p[i].player_id);

            int ret = send(p[j].socket_num, message, sizeof(message), 0);
        }
        p[i].Card_state.F_num = 0;
        p[i].socket_num = 0;
        closesocket(p[i].socket_num);
    }
}

void winner() {
    int k = 0;
    for (int i = 0; i < MAX_CLIENT; i++) {              //���� ������� ���ϳѹ��� 0���� �ʱ�ȭ�Ǿ����� -> ���� 1�̶�� ����ڰ� ����

        if (p[i].socket_num != 0) {
            k++;
        }
    }

    if (k == 1) {
        for (int j = 0; j < MAX_CLIENT; j++) {
            if (p[j].socket_num != 0) {
                char message[50] = { 0 };
                _snprintf(message, sizeof(message), "%d player is the winner!!!!!!!!!!!!\n", p[j].player_id);
                printf("%d player is the winner!!!!!!!!!!!!", p[j].player_id);
                int ret = send(p[j].socket_num, message, sizeof(message), 0);
                closesocket(p[j].socket_num);
                WSACleanup();
            }
        }
    }
}


void playgame() {
    char message[200] = "Start from User 1 .\n"; //����� 1 ���� ����
    broadcast_all(message);
    broadcast_cardnum();
    int ret;
    while (1) {
        if (p[0].card_num > 0) {
            winner();
            char message1[200] = "User 1's turn\n";
            broadcast_all(message1);
            turn = 1;
            Sleep(10);
            ret = send(p[0].socket_num, "Enter!\n", sizeof("Enter!\n"), 0);

            Sleep(10000);
        }


        if (p[1].card_num > 0) {
            winner();
            char message2[200] = "User 2's turn\n";

            broadcast_all(message2);
            turn = 2;
            Sleep(10);
            ret = send(p[1].socket_num, "Enter!\n", sizeof("Enter!\n"), 0);


            Sleep(10000);
        }

        if (p[2].card_num > 0) {
            winner();
            char message3[200] = "User 3's turn\n";

            broadcast_all(message3);
            turn = 3;
            Sleep(10);
            ret = send(p[2].socket_num, "Enter!\n", sizeof("Enter!\n"), 0);

            Sleep(10000);
        }



    }
}


void broadcast_all(char* mess) {
    int ret;


    //strcpy_s(message, sizeof(message), mess);

    for (int i = 0; i < MAX_CLIENT; i++) {
        char message[300] = { 0 };
        _snprintf(message, sizeof(message), mess);
        ret = send(p[i].socket_num, message, sizeof(message), 0);
    }

    return;
}

void broadcast_cardnum() {

    int ret;
    for (int j = 0; j < MAX_CLIENT; j++) {
        if (p[j].card_num >= 0) {
            char message[50] = { 0 };
            p[j].player_id = j + 1;
            _snprintf(message, sizeof(message), "User ID : %d / Number of Cards : %d\n", p[j].player_id, p[j].card_num);
            printf("%s\n", message);
            for (int i = 0; i < MAX_CLIENT; i++) {
                Sleep(100);
                ret = send(p[i].socket_num, message, sizeof(message), 0);
            }
        }
    }

    return;
}

void broadcast_cardinfo() {
    int ret;
    for (int j = 0; j < MAX_CLIENT; j++) {
        char message[50] = { 0 };
        p[j].player_id = j + 1;
        if (p[j].Card_state.F_num != 0) {
            _snprintf(message, sizeof(message), "User ID : %d / Fruits : %s, Number : %d\n", p[j].player_id, p[j].Card_state.Fruit, p[j].Card_state.F_num);
            printf("%s\n", message);
            for (int i = 0; i < MAX_CLIENT; i++) {
                Sleep(20);
                ret = send(p[i].socket_num, message, sizeof(message), 0);
            }
        }
    }

    return;
}

void resetFruitState() {
    for (int i = 0; i < MAX_CLIENT; i++) {
        p[i].player_id = i + 1;
        //strcpy_s(p[i].Card_state.Fruit, NULL);
        p[i].Card_state.F_num = 0;
    }
}

void randomcard(int i) {
    srand(time(NULL));
    //strcpy_s(p[i].Card_state.Fruit, sizeof(p[i].Card_state.Fruit), fruit[rand() % 4]);
    _snprintf(p[i].Card_state.Fruit, sizeof(p[i].Card_state.Fruit), fruit[rand() % 4]);
    p[i].Card_state.F_num = rand() % 5 + 1;
    COUNT++;
}


void pressBell(int i) {
    // ���� ������ ���� ������ 5������ �Ǵ�

    //{ "�ٳ���", "����", "����", "���" };
    //���� ����� ������ ���
    int banana = 0;
    int strawb = 0;
    int grape = 0;
    int apple = 0;




    for (int ii = 0; ii < MAX_CLIENT; ii++) { // �� ���� ���ϱ�
        if (strcmp(p[ii].Card_state.Fruit, "Banana") == 0) {
            banana += p[ii].Card_state.F_num;

        }
        else if (strcmp(p[ii].Card_state.Fruit, "Strawberry") == 0) {
            strawb += p[ii].Card_state.F_num;
        }
        else if (strcmp(p[ii].Card_state.Fruit, "Grape") == 0) {
            grape += p[ii].Card_state.F_num;
        }
        else { // ���
            apple += p[ii].Card_state.F_num;
        }
    }

    // 5�� �Ѵ°� �ִ��� ���ϱ�
    int check = 0; // 1�̸� True
    if (banana == 5 || strawb == 5 || grape == 5 || apple == 5) {
        check = 1;
    }

    if (check == 1) {
        printf("** User %d takes ALL CARDS! **", p[i].player_id);
        if (p[i].card_num >= 0) {
            p[i].card_num += COUNT;
        }

        COUNT = 0;
    }

    else {                                   //���� �߸������� ���

        int nowmem = 0;         //������ ī��� ���ϱ�
        for (int j = 0; j < MAX_CLIENT; j++) {
            if (p[j].card_num >= 0) {
                nowmem++;
            }
        }
        if (p[i].card_num >= nowmem) {          //������ ī�� ���� ����Ҷ�
            printf("** User %d's mistake! Give your CARDS! **\n", p[i].player_id);
            p[i].card_num = p[i].card_num - (nowmem - 1);
            for (int j = 0; j < MAX_CLIENT; j++) {
                if (p[j].card_num >= 0 && j != i) {       //���ӿ� �������� ������Ը� ī�� �ֱ�
                    p[j].card_num = p[j].card_num + 1;
                }
            }
        }
        else {
            p[i].card_num = -1;
            finish(i);
        }
    }

}