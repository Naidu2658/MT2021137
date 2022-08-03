
#include <stdio.h> 
#include <string.h>  
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>  
#include <arpa/inet.h>  
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdbool.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include "constants.h"
#include "structs.h"

struct clientCred cred;
struct clientDetails clientDet;
struct train train;
struct bookingRequest breq;
struct clientRequest cr;
struct serverResponse response;
struct flock userlock, trainlock, loginlock;
void handler(int);
bool authenticateUser(int ,int ,int,int);

int main() 
{ 

	int servsd ,  clisd; 
	struct sockaddr_in servaddr,cliaddr; 
		 
	servsd = socket(AF_INET , SOCK_STREAM , 0); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = INADDR_ANY; //server's ip address not client's
	servaddr.sin_port = htons(PORT); 
		
	bind(servsd, (void *)&servaddr, sizeof(servaddr)); 

	listen(servsd, 5);
		
	int addrlen = sizeof(servaddr);
        int cliaddrlen = sizeof(cliaddr); 
	
        printf("waiting for connections\n");	
		
	while(TRUE) 
	{ 
			
	        clisd = accept(servsd, (void *)&cliaddr,(void*)&cliaddrlen);
        	printf("New connection - ip : %s , port : %d\n" , inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

	if (!fork()) {
          		  close(servsd);
         		  handler(clisd);
		          printf("Host disconnected - ip : %s , port : %d\n" ,inet_ntoa(cliaddr.sin_addr) , ntohs(cliaddr.sin_port));
           		  exit(1);
     		} else{
                	close(clisd);// in this servsd will not be closed and waits for new connections in parent
		}
	
	}  
	
		
	return 0; 
} 


void handler(int clisd)
{
	
	int userfd = open("account.txt",O_RDWR);
	
	loginlock.l_type=F_WRLCK;
	loginlock.l_whence=SEEK_SET;
	loginlock.l_start=0;
	loginlock.l_len=0;
	loginlock.l_pid=getpid();

	char choice[10];
	
	bool userfound = false;
	read( clisd , &clientDet, sizeof(clientDet)); 
        userlock.l_type=F_WRLCK;
	userlock.l_whence=SEEK_SET;
	userlock.l_start=0;
	userlock.l_len=0;
	userlock.l_pid=getpid();
	
	trainlock.l_type=F_WRLCK;
	trainlock.l_whence=SEEK_SET;
	trainlock.l_start=0;
	trainlock.l_len=0;
	trainlock.l_pid=getpid();
	
	switch(clientDet.clientType)
					{
					case  1:
						printf("Login request: Agent\n");
              					userfound = authenticatUser(clientDet.cred.userid,clientDet.cred.password ,1,userfd);
						if(userfound)
						{
							printf("login success\n");
							write(clisd , "success" , sizeof("success") );

     	while(1) {
                                       
        read(clisd , &cr, sizeof(cr));

	struct clientRequest delcr;
	 train = cr.tr;
	 breq = cr.bookReq;
	int command = cr.command;
	struct train ret;
	struct serverResponse response;

	struct account user;
	struct bookingDetails bookdets;

	printf("command %d\n",command);

	int found=0;
	int fd;
	int userfd;
	int read_ret=1;

	switch(command)
	{
		case SEARCH:
				printf("Searching...\n");
				fd=open("train.txt",O_RDONLY);


						trainlock.l_start=0;
						trainlock.l_len=0;
						trainlock.l_type = F_RDLCK;
						fcntl(fd,F_SETLK,&trainlock) ;


				while(read_ret!=0)
		                {
					read_ret = read(fd,&ret,sizeof(struct train));

					if(ret.trainNo == train.trainNo)
					{
						found=1;
						response.commandStatus = 1;
						response.tr = ret;
						break;
					}

				}
				
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

				close(fd);

				write(clisd , &response ,sizeof(struct serverResponse));

		break;

		case BOOK_TICKET:
				printf("Booking ticket\n");
				fd=open("train.txt",O_RDWR);
				int index =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					index++;
					if(ret.trainNo == train.trainNo)
					{
						found=1;
						response.commandStatus = 1;
						response.tr = ret;
						break;
					}

				}

				userfd = open("account.txt",O_RDWR,0774);
				int uindex=0;
				read_ret =1;

				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					uindex++;
					if(user.uid == cr.uid)
					{
						break;
					}

				}
				if(response.commandStatus==1)
				{
					printf("Train found\n");
					printf("Train number %d\n",response.tr.trainNo);
					printf("Available seats %d\n",response.tr.totalSeats - response.tr.totalBooked );
					if(response.tr.totalSeats - response.tr.totalBooked >= breq.number_of_seats)
					{
                                  		ret.totalBooked += breq.number_of_seats;
						ret.totalSeats -= breq.number_of_seats;

						index--;
						trainlock.l_start=index*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

						lseek(fd,index*sizeof(struct train),SEEK_SET);
						if(write(fd,&ret,sizeof(struct train))==-1)
						{
								perror("write error\n");
						}
						else{
						response.bookingStatus =1;
						}

					
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

						bookdets.trainNo = ret.trainNo;
						bookdets.number_of_seats = breq.number_of_seats;


						int totalBookings = user.totalBookings;
						user.booked[totalBookings]=bookdets;

						user.totalBookings++;
						uindex--;
	
						userlock.l_start=uindex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

						lseek(userfd,uindex*sizeof(struct account),SEEK_SET);
						if(write(userfd,&user,sizeof(struct account))==-1)
						{
								perror("write error\n");
						}
						else{
								printf("User data updated\n");
						}

						
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);
					}
					else
					{
						printf("No more available seats\n");
					}

				}
				else
				{
					printf("Train not found\n");
				}


				close(fd);

				write(clisd , &response ,sizeof(struct serverResponse));
				



		break;

		case CANCEL_TICKET:

			userfd = open("account.txt",O_RDWR);

				read_ret =1;
				int usrindex=0;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					usrindex++;
					if(user.uid == cr.uid)
					{

						response.acc = user;
						break;
					}
				}

			write(clisd , &response ,sizeof(struct serverResponse));
			read(clisd , &delcr, sizeof(delcr));
			int delindex = delcr.deleteIndex;
			printf("DELINDEX %d\n",delindex);
			delindex--;

			int trainNumber = user.booked[delindex].trainNo;
			int numofseats = user.booked[delindex].number_of_seats;


			printf("train Number %d",trainNumber);

			fd=open("train.txt",O_RDWR);
				int rindex =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					rindex++;
					if(ret.trainNo == trainNumber)
					{
						response.commandStatus = 1;
						break;
					}

				}

			if(response.commandStatus == 1) {
			ret.totalBooked -= numofseats;
			ret.totalSeats += numofseats;

			rindex--;
						trainlock.l_start=rindex*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

			lseek(fd,rindex*sizeof(struct train),SEEK_SET);
			if(write(fd,&ret,sizeof(struct train))==-1)
			{
					perror("write error\n");
			}
			
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

			close(fd);
			
			for(int i=delindex ; i<user.totalBookings-1 ;i++)
			{
				user.booked[i] = user.booked[i+1];

			}
			user.totalBookings--;

			usrindex--;
						userlock.l_start=usrindex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);


			lseek(userfd,usrindex*sizeof(struct account),SEEK_SET);
			if(write(userfd,&user,sizeof(struct account))==-1)
			{
					perror("write error\n");
			}

			
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

			close(userfd);
			}

		write(clisd , &response ,sizeof(struct serverResponse));

		break;

		case PREV_BOOKING:

			userfd = open("account.txt",O_RDONLY);

				read_ret =1;
						userlock.l_start=0;
						userlock.l_len=0;
						userlock.l_type = F_RDLCK;
						fcntl(userfd,F_SETLK,&userlock);

				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));

					if(user.uid == cr.uid)
					{

						response.acc = user;
						break;
					}

				}
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

				close(userfd);

			send(clisd , &response ,sizeof(struct serverResponse),0);


		break;

		case UPDATE_BOOKING:

		userfd = open("account.txt",O_RDWR);

				read_ret =1;
				int usrIndex=0;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					usrIndex++;
					if(user.uid == cr.uid)
					{
 
						response.acc = user;
						break;
					}

				}


			write(clisd , &response ,sizeof(struct serverResponse));

				read(clisd , &delcr, sizeof(delcr));
			int updateindex = delcr.updateIndex;
			printf("UPDATE INDEX %d\n",updateindex);
			updateindex--;

			int TrainNumber = user.booked[updateindex].trainNo;
			int Numofseats = user.booked[updateindex].number_of_seats;
			int updatedSeats = delcr.updatedSeats;

			printf("train Number %d",TrainNumber);

				
			response.commandStatus = 0;
			fd=open("train.txt",O_RDWR);
				int Rindex =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					Rindex++;
					if(ret.trainNo == TrainNumber)
					{
						response.commandStatus = 1;
						break;
					}

				}

			if(response.commandStatus == 1) {

		
			ret.totalBooked -= Numofseats;
			ret.totalSeats += Numofseats;

			ret.totalBooked += updatedSeats;
			ret.totalSeats -= updatedSeats;

			Rindex--;
	
						trainlock.l_start=Rindex*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

			lseek(fd,Rindex*sizeof(struct train),SEEK_SET);
			if(write(fd,&ret,sizeof(struct train))==-1)
			{
					perror("write error\n");
			}

		
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);


			close(fd);

			user.booked[updateindex].number_of_seats = updatedSeats;
			printf("SEATS\n: %d",updatedSeats);
			usrIndex--;
			
						userlock.l_start=usrIndex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

			lseek(userfd,usrIndex*sizeof(struct account),SEEK_SET);
			if(write(userfd,&user,sizeof(struct account))==-1)
			{
					perror("write error\n");
			}

						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

			close(userfd);
			}

			write(clisd , &response ,sizeof(struct serverResponse));


		break;

		case LOGOUT:

		return;

		break;
	}
	}
							loginlock.l_type=F_UNLCK;
							fcntl(userfd,F_SETLK,&loginlock);
						}
						else
						{
							printf("Login failed\n");
							send(clisd , "Login failed" , 50 , 0 ); 
						}
						break;

					case 2:
						printf("Login request: Customer\n");
						
						userfound= authenticatUser(clientDet.cred.userid,clientDet.cred.password ,2,userfd);
						if(userfound)
						{
							printf("login success\n");
							write(clisd , "success" , sizeof("success")  );                                                         

	while(1) {

	read(clisd , &cr, sizeof(cr));

	struct clientRequest delcr;
        train = cr.tr;
        breq = cr.bookReq;
	int command = cr.command;
	struct train ret;
	struct account user;
	struct bookingDetails bookdets;

	printf("command %d\n",command);

	int found=0;
	int fd;
	int userfd;
	int read_ret=1;

	switch(command)
	{
		case SEARCH:
				printf("Searching\n");
				fd=open("train.txt",O_RDONLY);

				trainlock.l_start=0;
				trainlock.l_len=0;
				trainlock.l_type = F_RDLCK;

				fcntl(fd,F_SETLK,&trainlock);

				while(read_ret!=0)
				{

					read_ret = read(fd,&ret,sizeof(struct train));

					if(ret.trainNo == train.trainNo)
					{
						found=1;
						response.commandStatus = 1;
						response.tr = ret;
						break;
					}

				}
				
				trainlock.l_type = F_UNLCK;
				fcntl(fd,F_SETLK,&trainlock);

				close(fd);

				write(clisd , &response ,sizeof(struct serverResponse));

		break;

		case BOOK_TICKET:
				printf("Booking ticket\n");
				fd=open("train.txt",O_RDWR);
				int index =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					index++;
					if(ret.trainNo == train.trainNo)
					{
						found=1;
						response.commandStatus = 1;
						response.tr = ret;
						break;
					}

				}

				userfd = open("account.txt",O_RDWR,0774);
				int uindex=0;
				read_ret =1;

				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					uindex++;
					if(user.uid == cr.uid)
					{
						break;
					}

				}
				if(response.commandStatus==1)
				{
					printf("Train found\n");
					printf("Train number %d\n",response.tr.trainNo);
					printf("Available seats %d\n",response.tr.totalSeats - response.tr.totalBooked );
					if(response.tr.totalSeats - response.tr.totalBooked >= breq.number_of_seats)
					{

						//updating train details
						ret.totalBooked += breq.number_of_seats;
						ret.totalSeats -= breq.number_of_seats;

						index--;
						//lock train
						trainlock.l_start=index*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;


						if(fcntl(fd,F_SETLK,&trainlock)  == -1)
						{
							perror("train lock error");

						}
						else
						{
								printf("train Lock acquired\n");
						}

						lseek(fd,index*sizeof(struct train),SEEK_SET);
						if(write(fd,&ret,sizeof(struct train))==-1)
						{
								perror("write error\n");
						}
						else{
						response.bookingStatus =1;
						}
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

						bookdets.trainNo = ret.trainNo;
						bookdets.number_of_seats = breq.number_of_seats;


						int totalBookings = user.totalBookings;
						user.booked[totalBookings]=bookdets;

						user.totalBookings++;
						uindex--;
						//lock user
						userlock.l_start=uindex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						if(fcntl(userfd,F_SETLK,&userlock) == -1)
						{
							perror("user lock error");

						}
						else
						{
								printf("user Lock acquired\n");
						}

						lseek(userfd,uindex*sizeof(struct account),SEEK_SET);
						if(write(userfd,&user,sizeof(struct account))==-1)
						{
								perror("write error\n");
						}
						else{
								printf("User data updated\n");
						}

						//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

					}
					else
					{
						printf("No more available seats\n");
					}

				}
				else
				{
					printf("Train not found\n");
				}


				close(fd);

				write(clisd , &response ,sizeof(struct serverResponse));

		break;

		case CANCEL_TICKET:

			userfd = open("account.txt",O_RDWR);

				read_ret =1;
				int usrindex=0;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					usrindex++;
					if(user.uid == cr.uid)
					{

						response.acc = user;
						break;
					}

				}

			write(clisd , &response ,sizeof(struct serverResponse));
			read(clisd , &delcr, sizeof(delcr));
			int delindex = delcr.deleteIndex;
			printf("DELINDEX %d\n",delindex);
			delindex--;

			int trainNumber = user.booked[delindex].trainNo;
			int numofseats = user.booked[delindex].number_of_seats;


			printf("train Number %d",trainNumber);

			//updating train db

			fd=open("train.txt",O_RDWR);
				int rindex =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					rindex++;
					if(ret.trainNo == trainNumber)
					{
						response.commandStatus = 1;
						break;
					}

				}

			if(response.commandStatus == 1) {
			ret.totalBooked -= numofseats;
			ret.totalSeats += numofseats;

			rindex--;
			//lock train
						trainlock.l_start=rindex*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

			lseek(fd,rindex*sizeof(struct train),SEEK_SET);
			if(write(fd,&ret,sizeof(struct train))==-1)
			{
					perror("write error\n");
			}

			//unlock train
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

			close(fd);
			//updating user db


			for(int i=delindex ; i<user.totalBookings-1 ;i++)
			{
				user.booked[i] = user.booked[i+1];

			}
			user.totalBookings--;

			usrindex--;
			//lock user
						userlock.l_start=usrindex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

			lseek(userfd,usrindex*sizeof(struct account),SEEK_SET);
			if(write(userfd,&user,sizeof(struct account))==-1)
			{
					perror("write error\n");
			}

			//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

			close(userfd);
			}

			send(clisd , &response ,sizeof(struct serverResponse),0);

		break;

		case PREV_BOOKING:

			userfd = open("account.txt",O_RDONLY);

				read_ret =1;
				//lock user
						userlock.l_start=0;
						userlock.l_len=0;
						userlock.l_type = F_RDLCK;
						fcntl(userfd,F_SETLK,&userlock);


				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));

					if(user.uid == cr.uid)
					{

						response.acc = user;
						break;
					}

				}

				//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

				close(fd);

			send(clisd , &response ,sizeof(struct serverResponse),0);


		break;

		case UPDATE_BOOKING:

		userfd = open("account.txt",O_RDWR);

				read_ret =1;
				int usrIndex=0;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					usrIndex++;
					if(user.uid == cr.uid)
					{

						response.acc = user;
						break;
					}

				}


			write(clisd , &response ,sizeof(struct serverResponse));

				read(clisd , &delcr, sizeof(delcr));
			int updateindex = delcr.updateIndex;
			printf("UPDATE INDEX %d\n",updateindex);
			updateindex--;

			int TrainNumber = user.booked[updateindex].trainNo;
			int Numofseats = user.booked[updateindex].number_of_seats;
			int updatedSeats = delcr.updatedSeats;

			printf("train Number %d",TrainNumber);

				//updating train db
			response.commandStatus = 0;
			fd=open("train.txt",O_RDWR);
				int Rindex =0;
				while(read_ret!=0)
				{
					read_ret = read(fd,&ret,sizeof(struct train));
					Rindex++;
					if(ret.trainNo == TrainNumber)
					{
						response.commandStatus = 1;
						break;
					}

				}

			if(response.commandStatus == 1) {

			//reversing old booking
			ret.totalBooked -= Numofseats;
			ret.totalSeats += Numofseats;

			//adding new booking
			ret.totalBooked += updatedSeats;
			ret.totalSeats -= updatedSeats;

			Rindex--;

						trainlock.l_start=Rindex*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

			lseek(fd,Rindex*sizeof(struct train),SEEK_SET);
			if(write(fd,&ret,sizeof(struct train))==-1)
			{
					perror("write error\n");
			}

						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

			close(fd);
			

			user.booked[updateindex].number_of_seats = updatedSeats;
			printf("SEATS\n: %d",updatedSeats);
			usrIndex--;
			//lock user
						userlock.l_start=usrIndex*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

			lseek(userfd,usrIndex*sizeof(struct account),SEEK_SET);
			if(write(userfd,&user,sizeof(struct account))==-1)
			{
					perror("write error\n");
			}

			//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

			close(userfd);
			}

			write(clisd , &response ,sizeof(struct serverResponse));


		break;

		case LOGOUT:

		return;

		break;
	}
	}
							loginlock.l_type=F_UNLCK;
							fcntl(userfd,F_SETLK,&loginlock);
		//realse write 
						}
						else
						{
							printf("Login failed\n");
							send(clisd , "Login failed" , 50 , 0 ); 
						}

						break;

					case 3:
						printf("Login request: Admin\n");

						userfound = authenticatUser(clientDet.cred.userid,clientDet.cred.password ,3,userfd);
						if(userfound)
						{
							printf("login success\n");
							send(clisd , "success" , 10 , 0 ); 
							
						//	AdminOperations(clisd);
						         

	
	while(1) {

	read(clisd , &cr, sizeof(cr));

	struct clientRequest delcr;
         train = cr.tr;
        breq = cr.bookReq;
	int command = cr.command;
	struct train ret;

	struct account user;
	struct bookingDetails bookdets;

	struct account emptyUser;
	emptyUser.uid = -1;
	emptyUser.pin = -1;
	emptyUser.accType = -1;

	struct train emptyTrain;
	emptyTrain.totalBooked = -1;
	emptyTrain.trainNo = -1;
	emptyTrain.totalSeats = -1;


	printf("COMMAND %d\n",command);

	int found=0;
	int fd;
	int userfd;
	int trainfd;
	int read_ret=1;

	switch(command)
	{
		case SEARCH_USER:

			userfd = open("account.txt",O_RDONLY);

				read_ret =1;
				response.commandStatus =0;

				//lock user
						userlock.l_start=0;
						userlock.l_len=0;
						userlock.l_type = F_RDLCK;
						fcntl(userfd,F_SETLK,&userlock);

				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));

					if(user.uid == cr.uid)
					{
						response.commandStatus = 1;
						//response.tr = ret;
						break;
					}

				}
				//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

				close(userfd);

				send(clisd , &response ,sizeof(struct serverResponse),0);

		break;

		case SEARCH_TRAIN:
				printf("Searching...\n");
				fd=open("train.txt",O_RDONLY);

				response.commandStatus =0;

				//lock train
						trainlock.l_start=0;
						trainlock.l_len=0;
						trainlock.l_type = F_RDLCK;
						fcntl(fd,F_SETLK,&trainlock) ;

				while(read_ret!=0)
				{

					read_ret = read(fd,&ret,sizeof(struct train));

					if(ret.trainNo == train.trainNo)
					{
						found=1;
						response.commandStatus = 1;
						response.tr = ret;
						break;
					}

				}

				//unlock train
						trainlock.l_type = F_UNLCK;
						fcntl(fd,F_SETLK,&trainlock);

				close(fd);

				send(clisd , &response ,sizeof(struct serverResponse),0);

		break;


		case ADD_USER:

				userfd = open("account.txt",O_RDWR);

				int addusrptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newuserStatus=1;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					addusrptr++;
					if(user.uid == cr.uid)
					{
						response.newuserStatus=0;
						break;
					}

				}

				if(response.newuserStatus ==1)
				{
					addusrptr--;
					//lock user
						userlock.l_start=addusrptr*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

					lseek(userfd,addusrptr*sizeof(struct account),SEEK_SET);
					if(write(userfd,&cr.newuser,sizeof(struct account))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

					//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

					close(userfd);

				}
				else
				{
					printf("User already exists!!!\n");
				}
				send(clisd , &response ,sizeof(struct serverResponse),0);

		break;

		case DELETE_USER:

			userfd = open("account.txt",O_RDWR);

				int delusrptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newuserStatus=1;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					delusrptr++;
					if(user.uid == cr.uid)
					{
						response.newuserStatus=0;
						break;
					}

				}

				if(response.newuserStatus ==0)
				{
					delusrptr--;
					//lock user
						userlock.l_start=delusrptr*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);

					lseek(userfd,delusrptr*sizeof(struct account),SEEK_SET);
					if(write(userfd,&emptyUser,sizeof(struct account))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

					//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

					close(userfd);

				}
				else
				{
					printf("User does not exists!!!\n");
				}
				send(clisd , &response ,sizeof(struct serverResponse),0);



		break;

		case ADD_TRAIN:

				trainfd = open("train.txt",O_RDWR);

				int addtrainptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newtrainStatus=1;

				while(read_ret!=0)
				{
					read_ret = read(trainfd,&ret,sizeof(struct train));
					addtrainptr++;
					if(ret.trainNo == cr.trainNo)
					{
						response.newtrainStatus=0;
						break;
					}

				}

				if(response.newtrainStatus ==1)
				{
					addtrainptr--;

					 //lock train
						trainlock.l_start=addtrainptr*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(trainfd,F_SETLK,&trainlock) ;

					lseek(trainfd,addtrainptr*sizeof(struct train),SEEK_SET);
					if(write(trainfd,&cr.tr,sizeof(struct train))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

						//unlock train
						trainlock.l_type = F_UNLCK;
						fcntl(trainfd,F_SETLK,&trainlock);

					close(trainfd);

				}
				else
				{
					printf("train already exists!!!\n");
				}
				send(clisd , &response ,sizeof(struct serverResponse),0);



		break;

		case DELETE_TRAIN:

		trainfd = open("train.txt",O_RDWR);

				int deltrainptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newtrainStatus=1;
				while(read_ret!=0)
				{
					read_ret = read(trainfd,&ret,sizeof(struct train));
					deltrainptr++;
					if(ret.trainNo == cr.trainNo)
					{
						response.newtrainStatus=0;
						break;
					}

				}

				if(response.newuserStatus ==0)
				{
					deltrainptr--;
					 //lock train
						trainlock.l_start=deltrainptr*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(trainfd,F_SETLK,&trainlock);

					lseek(trainfd,deltrainptr*sizeof(struct train),SEEK_SET);
					if(write(trainfd,&emptyTrain,sizeof(struct train))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

					//unlock train
						trainlock.l_type = F_UNLCK;
						fcntl(trainfd,F_SETLK,&trainlock);

					close(trainfd);

				}
				else
				{
					printf("Train does not exists!!!\n");
				}
				send(clisd , &response ,sizeof(struct serverResponse),0);


		break;


		case MODIFY_USER:

			userfd = open("account.txt",O_RDWR);

				int modusrptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newuserStatus=1;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&user,sizeof(struct account));
					modusrptr++;
					if(user.uid == cr.uid)
					{
						response.newuserStatus=0;
						response.acc = user;
						break;
					}

				}

				send(clisd , &response ,sizeof(struct serverResponse),0);

				if(response.newuserStatus ==0)
				{
					read(clisd , &cr, sizeof(cr));
					modusrptr--;

					//lock user
						userlock.l_start=modusrptr*sizeof(struct account);
						userlock.l_len=sizeof(struct account);
						userlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&userlock);


					lseek(userfd,modusrptr*sizeof(struct account),SEEK_SET);
					if(write(userfd,&cr.newuser,sizeof(struct account))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

					//unlock user
						userlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&userlock);

				}
				else
				{
					response.commandStatus=0;
				}

				send(clisd , &response ,sizeof(struct serverResponse),0);

		break;

		case MODIFY_TRAIN:

			userfd = open("train.txt",O_RDWR);

				int modtrainptr =0;
				read_ret =1;
				response.commandStatus =1;
				response.newtrainStatus=1;
				while(read_ret!=0)
				{
					read_ret = read(userfd,&ret,sizeof(struct train));
					modtrainptr++;
					if(ret.trainNo == cr.trainNo)
					{
						response.newtrainStatus=0;
						response.tr = ret;
						break;
					}

				}

				send(clisd , &response ,sizeof(struct serverResponse),0);

				if(response.newtrainStatus ==0)
				{
					read(clisd , &cr, sizeof(cr));
					modtrainptr--;
                                         
					//lock train
						trainlock.l_start=modtrainptr*sizeof(struct train);
						trainlock.l_len=sizeof(struct train);
						trainlock.l_type = F_WRLCK;
						fcntl(userfd,F_SETLK,&trainlock);

					lseek(userfd,modtrainptr*sizeof(struct train),SEEK_SET);
//					int alreadybooked=cr.tr.totalBooked;
                                        //cr.tr.totalBooked=cr.tr.totalSeats-alreadybooked;
					if(write(userfd,&cr.tr,sizeof(struct train))==-1)
					{
						response.commandStatus=0;
						perror("write error\n");
					}

						//unlock train
						trainlock.l_type = F_UNLCK;
						fcntl(userfd,F_SETLK,&trainlock);

				}
				else
				{
					response.commandStatus=0;
				}

				write(clisd , &response ,sizeof(struct serverResponse));


		break;

		case LOGOUT_ADMIN:

			return;
		break;

	}

	}
							loginlock.l_type=F_UNLCK;
							fcntl(userfd,F_SETLK,&loginlock);
						}
						else
						{
							printf("Login failed\n");
							send(clisd , "failed" , 10 , 0 ); 
						}
						break;

					default :
						printf("Invalid choice\n");


						break;


					}
return;

}



bool authenticatUser(int userid , int password, int accType,int userfd)
{

	//int userfd = open("account.txt",O_RDONLY);
	int read_ret =1;
	struct account user;
	int found =0;
	int index=0;

	while(read_ret!=0)
	{
		read_ret = read(userfd,&user,sizeof(struct account));
		index++;
		if(user.uid == userid)  
		{
			found =1 ;
			break;
		}		
	}
	//close(userfd);
	
	if(accType==1) //agent
	{
		loginlock.l_type=F_RDLCK;	
	}
	else
	{
		loginlock.l_type=F_WRLCK;
	}
	
	index--;
	loginlock.l_whence=SEEK_SET;
	loginlock.l_start=index*sizeof(struct account);
	loginlock.l_len=sizeof(struct account);
	loginlock.l_pid=getpid();
	
	
	if(found ==1)
	{
		if(userid == user.uid &&  password == user.pin && accType == user.accType)
		{
			if(fcntl(userfd,F_SETLKW,&loginlock) == -1)
			{
				perror("lock error:");
			}
			else
			{
				printf("Lock acquired:\n");
			}
			
			
			return true;
		}
		else
		{
			return false;
		}
	}
	else
		return false;
}
