/*

*
* How to compile:  g++ simulation.cpp -std=c++14 -o simulation.o -lpthread
* How to run:  ./simulation.o configuration_path output_path
 *
 * There is a make "Makefile" file in the project folder. This code can be compiled with this file. When "make" is written to the shell this code will be compiled.
 * Then in order to run this code you need to use this command "./simulation.o configuration_path output_path" . configuration_path is the path of the input file
 * and output_path is the path of the outputfile. The code will create an outputfile and write the output into this file.
 * (Ex run: ./simulation.o ./configuration_file_5.txt ./OutputFile.txt )


*/
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;
string inputPath;  // path of the input file from run command
string outputpath;  // path of the output file from run command
string theaterhall; /*
 * holds name of the theater
 */
int numberofclients; /*
 * holds total number of clients
 */
string clientname[300];/*
 * holds names of the clients. Size is 300 beacuse there are maximum 300 clients.
 */
int clientarrive[300];/*
 * holds arrive times of cilents
 */
int clientservice[300];/*
 * holds service times of clients
 */
int clientchair[300];/*
 * holds requested chair number of clients
 */

int buffer[300];/*
 * holds the requested chairs order of queuing
 */
int START_NUMBER=0;/*
 * determine indexitem in the prodeucer function
 */
string lineclientname[300];/*
 * holds the client names order of queuing
 */
int lineclientarrive[300];/*
 * holds the arrive times order of queuing
 */
int lineclientservice[300];/*
 * holds the service times order of queuing
 */


/*Define mutex and semaphores */
pthread_mutex_t mutex;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
sem_t empty;
sem_t full;


int insertPointer = 0;/*
 * defines insertPointer in the producer function
 */
int removePointer = 0; /*
 * defines removepointer in the consumer function
 */
pthread_t tellerid1[3];/*
 * holds the id of tellers
 */

/*
 * defines producer and consumer functions
 */
void *producer(void *param);
void *consumer(void *param);


/*
 * defines the tellers are busy for a client or free
 */
int aisworking=0;
int bisworking=0;
int cisworking=0;
/*
 * defines the tellers enter the consumer function or not
 */
int aentered=0;
int bentered=0;
int centered=0;

/*
 * defines ouputfile
 */
ofstream outputfile;


int seats[300];/*
 * defines the empty seats at first
 */

int seatcount=0;/*
 * defines max seat count in the theater
 */




/*
 * In the main function input file is read and client and teller threads are created.
 */

int main(int argc, char *argv[]){





    for (int m = 1; m <=300 ; ++m) {  // at first all seats assigned to the index number
        seats[m]=m;
    }
    
    for (int k = 0; k <300 ; ++k) { // at first all buffer items essigned to -1
        buffer[k]= -1;

    }

    inputPath = argv[1]; //I toke the name of the input file from command line with argv[1]
    outputpath= argv[2]; // takes output path from command line

    outputfile.open(outputpath);/*
 * opens outputfile
 */
    outputfile<<"Welcome to the Sync-Ticket!"<<endl;
    ifstream file;
    string line;



    file.open(inputPath);
    file>>theaterhall;  // get the theaterhall from input file
    file>>numberofclients; // get the numberofclients from input file
    getline(file, line);
    for(int i=0; i<numberofclients; i++) {   // get the clients' names, requested chairs, arrive time and service time from input file. Then put these values to the arrays
        getline(file, line);
        istringstream ss(line);
        string token;

        getline(ss, token, ',');
        clientname[i]=token;
        getline(ss, token, ',');
        int token1=stoi(token);
        clientarrive[i]=token1;
        getline(ss, token, ',');
        int token2=stoi(token);
         clientservice[i]=token2;
        getline(ss, token, ',');
        int token3=stoi(token);
         clientchair[i]=token3;


    }
    file.close();

    if(theaterhall=="OdaTiyatrosu"){ // İf theater name is "OdaTiyatrosu" max set number is 60
seatcount=60;
    } else if (theaterhall=="UskudarStudyoSahne"){
        seatcount=80;

    } else if (theaterhall=="KucukSahne"){
    seatcount=200;
    }






    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_mutex_init(&mutex3, NULL);
    /* sem full is initialized as 0 */
    sem_init(&full, 0, 0);
    /* sem empty is initialized as buffer size */
    sem_init(&empty, 0, 300);

    pthread_t pid[numberofclients];  // defines numberofclients producer threads
    pthread_t cid[3]; // defines three consumer threads

    for(int j = 0; j < 3; j++){  // creates consumer threads. They are tellers
        pthread_create(&cid[j], NULL, &consumer, NULL);
    }


    for(int j = 0; j < 3; j++){
        tellerid1[j]=cid[j];  // put id of the tellers to teller1 array to identify tellers A,B,C
    }



    for(int i = 0; i < numberofclients; i++){ // creates producer threads. They are clients
        pthread_create(&pid[i], NULL, &producer, NULL);
    }



    /*
     * join threads
     */
    for(int i = 0; i < numberofclients; i++) {
        pthread_join(pid[i], NULL);

    }
    for(int j = 0; j < 3; j++) {
        pthread_join(cid[j], NULL);

    }

    outputfile<<"All clients received service."<<endl; // There are no more clients
    outputfile.close();/*
 * close the output file
 */

/* Delete mutex and destroy semaphores */
    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);


    return 0;

}




/*
 * producer function for clients
 * In this function reserved seats of clients are placed to the buffer array order of queuing
 */

void *producer(void *param){


    int itemindex;  // index of the clients which enter this function
    int item;  // item is the chair number which client wants

    pthread_mutex_lock(&mutex);
    itemindex = START_NUMBER++;    // when each clients enters this mutex they increases itemindex by one for other clients.
    int sleeptime=clientarrive[itemindex];  // clients waits until it enter the line
    int servicesleeptime= clientservice[itemindex]; // service time for clients
    pthread_mutex_unlock(&mutex);

    usleep(sleeptime*1000);  // clients sleep before they enter the line by sleeptime*1000 milliseconds

    sem_wait(&empty);
    pthread_mutex_lock(&mutex);

    item=clientchair[itemindex];  // item holds the chair number that requested by client
    buffer[insertPointer] = item; /*
 * global intertPointer at first zero and when each client put their requested seat to buffer this value will be  increased. Also buffer holds the requested chairs.
 */

    lineclientname[insertPointer]=clientname[itemindex]; // this array holds the clientnames in order of queuing
    lineclientarrive[insertPointer]=clientarrive[itemindex]; // this array holds the arrivetime in order of queuing
    lineclientservice[insertPointer]=clientservice[itemindex]; // this array holds the servicetime in order of queuing
    insertPointer = (insertPointer + 1) ;  //increments the insetpointer.
    pthread_mutex_unlock(&mutex);
    sem_post(&full);

    usleep(servicesleeptime*1000); // clients wait until the teller service to them
    pthread_exit(NULL);  // client threads exit from this function
}

/*
 * consumer function for tellers
 * In this function clients are serviced by tellers
*/
void *consumer(void *param){
    int item;   // holds the chair number which the client will be assigned
    int itemindex=0;  // itemindex holds the index of the client in the buffer array.
    int sleeptime;  // sleep time is the time amount of service time of teller


    while(1) {








        if (pthread_self() == tellerid1[0] ) {  // shows teller named A

            if(  aentered==0){ // Thanks to aentered A can enter this if just one time
                pthread_mutex_lock(&mutex3);

                outputfile<<"Teller A has arrived."<<endl;
                pthread_mutex_unlock(&mutex3);
                aentered=1;
            }



                if( buffer[removePointer] != -1 && bentered==1 && centered==1) { /*
 * if there is a new client in the buffer also b and c teller has arrived A enters this if in order to deal with customers
                    */

                    itemindex = removePointer; // asssign loccal itemindex to current removePointer
                    pthread_mutex_lock(&mutex2);
                    removePointer = (removePointer + 1); // Then, increase removePointer by one
                    pthread_mutex_unlock(&mutex2);


                    sem_wait(&full);
                    pthread_mutex_lock(&mutex);
                    item = buffer[itemindex]; // take the chair number from buffer array and assign to item
                    sleeptime = lineclientservice[itemindex];  // determine the time teller will sleep
                    pthread_mutex_unlock(&mutex);
                    sem_post(&empty);






                    pthread_mutex_lock(&mutex1);
                    for (int i = 0; i < 300; ++i) {
                        if (seats[item] == -1 || item > seatcount) { // if the chair that clients requested is full or requested chait number is bigger than the total chait number
                            for (int j = 1; j < 300; ++j) {
                                if (seats[j] != -1) {  // start from one to max chair number and assign item to first empty chair
                                    item = j;  //assign item to this empty chair
                                    seats[item] = -1; // make this chair full
                                    break;
                                }
                            }
                            break;
                        } else if (i = 299) { // if the requested chait is empty don't change the item

                            seats[item] = -1; // Then, make this chair full
                        }

                    }
                    pthread_mutex_unlock(&mutex1);

                    aisworking = 1; // this means A is now busy with a client
                    usleep(sleeptime * 1000); //  A sleeps by servie time
                    aisworking = 0; // now A is empty for another client
                    pthread_mutex_lock(&mutex3);
                    if (itemindex >= seatcount) {   // If all the chairs full
                        outputfile << lineclientname[itemindex]
                             << " requests seat "
                             << buffer[itemindex] << ", " << "reserves None. " << "Signed by Teller "
                             << "A" << "." << endl;
                    } else { // If not all the chairs full

                        outputfile  << lineclientname[itemindex]<< " requests seat "<< buffer[itemindex] << ", " << "reserves seat " << item << ". "
                             << "Signed by Teller "
                             << "A" << "." << endl;
                    }

                    pthread_mutex_unlock(&mutex3);
                }

        }  else if (pthread_self() == tellerid1[1]  ) { // shows teller named B

            while(aentered!=1){  // wait arriving of A

            }

            if( bentered==0 ){  //Thanks to bentered, B can enter this if just one time
                pthread_mutex_lock(&mutex3);
                outputfile<<"Teller B has arrived."<<endl;
                pthread_mutex_unlock(&mutex3);
                bentered=1;
                }



            if( buffer[removePointer] != -1 && aisworking==1  && centered==1) { /*
 * if there is a new client in the buffer and a is currently busy and c teller has arrived then B enters this if in order to deal with customers
 */

                itemindex = removePointer;  // same as A
                pthread_mutex_lock(&mutex2);
                removePointer = (removePointer + 1); // same as A
                pthread_mutex_unlock(&mutex2);




                sem_wait(&full);
                pthread_mutex_lock(&mutex);
                item = buffer[itemindex]; // same as A
                sleeptime = lineclientservice[itemindex]; // same as A
                pthread_mutex_unlock(&mutex);
                sem_post(&empty);




                pthread_mutex_lock(&mutex1);
                for (int i = 0; i < 300; ++i) {
                    if (seats[item] == -1 || item > seatcount) {  // same as A
                        for (int j = 1; j < 300; ++j) {
                            if (seats[j] != -1) { // same as A
                                item = j;
                                seats[item] = -1;
                                break;
                            }
                        }
                        break;
                    } else if (i = 299) {
                        seats[item] = -1;
                    }
                }
                pthread_mutex_unlock(&mutex1);


                bisworking = 1; // tells B is now busy
                usleep(sleeptime * 1000); // B sleeps by service time
                bisworking = 0; // now, B is free for other clients
                pthread_mutex_lock(&mutex3);
                if (itemindex >= seatcount) { // same as A
                    outputfile << lineclientname[itemindex] << " requests seat "
                         << buffer[itemindex] << ", " << "reserves None. " << "Signed by Teller "
                         << "B" << "." << endl;
                } else { // same as A
                    outputfile << lineclientname[itemindex] << " requests seat "
                         << buffer[itemindex] << ", " << "reserves seat " << item << ". "
                         << "Signed by Teller "
                         << "B" << "." << endl;
                }
                pthread_mutex_unlock(&mutex3);

            }


        }

         else if (pthread_self() == tellerid1[2] ) { // shows teller named C

            while(bentered!=1){  // teller C wait until B to arrive

            }

            if( centered==0 ){  // Thanks to centered, C can enter this if just one time
                pthread_mutex_lock(&mutex3);
                outputfile<<"Teller C has arrived."<<endl;
                pthread_mutex_unlock(&mutex3);

                centered=1;
            }

            if(buffer[removePointer] != -1&& aisworking==1 && bisworking==1 ) { /*
 * if there is a new client in the buffer also a and b are currently busy then C enters this if in order to deal with customers
 */

                itemindex = removePointer; // same as A
                pthread_mutex_lock(&mutex2);
                removePointer = (removePointer + 1) ; // same as A
                pthread_mutex_unlock(&mutex2);


                sem_wait(&full);
                pthread_mutex_lock(&mutex);
                item = buffer[itemindex]; // same as A
                 sleeptime = lineclientservice[itemindex]; // same as A
                pthread_mutex_unlock(&mutex);
                sem_post(&empty);


                pthread_mutex_lock(&mutex1);

                for (int i = 0; i <300 ; ++i){
                    if(seats[item]==-1 || item>seatcount){ // same as A
                        for (int j = 1; j <300 ; ++j) {
                            if(seats[j]!=-1){ // same as A
                                item=j;
                                seats[item]=-1;
                                break;
                            }
                        }
                        break;
                    }
                    else if(i=299){ // same as A
                        seats[item]=-1;
                    }
                }
                pthread_mutex_unlock(&mutex1);



                cisworking = 1; // c is busy
                usleep(sleeptime * 1000); // b sleeps by service time
                cisworking = 0; // c is free for other clients
                pthread_mutex_lock(&mutex3);
                if(itemindex>=seatcount){  // same as A
                    outputfile << lineclientname[itemindex] << " requests seat "
                         << buffer[itemindex] << ", " << "reserves None. "<< "Signed by Teller "
                         << "C" << "." << endl;
                }
                else { // same as A

                    outputfile  << lineclientname[itemindex] << " requests seat "
                         << buffer[itemindex] << ", " << "reserves seat " << item << ". "
                         << "Signed by Teller "
                         << "C" << "." << endl;
                }
                pthread_mutex_unlock(&mutex3);

}

        }

        if(removePointer==numberofclients ){
            break;

        }
    }

    pthread_exit(NULL);


}

