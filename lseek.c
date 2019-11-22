#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/uio.h>

int main() {
    char buf[12];
    int fd = open("hello.dat", O_CREAT|O_RDWR); 
    lseek(fd, 121323434, SEEK_SET);
    write(fd, buf, 1);
    close(fd);
}