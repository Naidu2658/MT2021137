#include<stdio.h>
#include <sys/types.h>
#include <unistd.h>

       #include <sys/stat.h>
       #include <fcntl.h>
int main()
{
	struct bookingDetails{
        int trainNo;
        int number_of_seats;
};

	struct account{
        int uid;
        int pin;
        int accType;
        struct bookingDetails booked[50];
        int totalBookings;
};

	int fd=open("account.txt", O_RDWR);
         struct account admin;
	 admin.uid=100;
	 admin.pin=100;
	 admin.accType=3;
	 admin.totalBookings=0;

	 write(fd, &admin, sizeof(struct account));
	 close(fd);
	 }
