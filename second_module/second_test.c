#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	int fd;
	int counter = 0;
	int old_counter = 0;

	fd = open("/dev/second", O_RDONLY);
	if (fd == -1) {
		printf("open err\n");
		return -1;
	}
	while (1) {
		read(fd, &counter, sizeof(unsigned int));
		if (counter != old_counter) {
			printf("seconds after open /ded/second: %d\n", counter);
			old_counter = counter;
		}
	}
	return 0;
}
