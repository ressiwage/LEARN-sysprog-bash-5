// #include <unistd.h>
// #include <sys/wait.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// int main()
// {
// 	printf("I am process %d\n", (int) getpid());
// 	char *mem = (char *) calloc(1, 100);
// 	pid_t child_pid = fork();
// 	pid_t my_pid = getpid();
// 	if (child_pid == 0) {
// 		printf("%d: I am child, fork returned %d\n",
// 		       (int) my_pid, (int) child_pid);
// 		printf("%d: child is terminated with code 100\n",
// 		       (int) my_pid);
// 		printf("%d: memory values are set to 1\n", (int) my_pid);
// 		memset(mem, 1, 100);
// 		return 100;
// 	}
// 	printf("%d: I am parent, fork returned %d\n",
// 	       (int) my_pid, (int) child_pid);
// 	int stat;
// 	pid_t wait_result = wait(&stat);
// 	printf("%d: wait returned %d and stat %d\n", (int) my_pid,
// 	       (int) wait_result, stat);
// 	printf("%d: memory values are %d\n", (int) my_pid, (int) mem[0]);
// 	printf("%d: returned child code was %d\n", (int) my_pid,
// 	       WEXITSTATUS(stat));
// 	return 0;
// }




#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main()
{
	int to_child[2];
	pipe(to_child); // to_child = [read <- write]
	dup2(to_child[0], 0); //to_child[0] -- std read; дефолтный std read закрыт
	if (fork() == 0) {
		close(to_child[1]); // если мы в ребенке то закрываем дескриптор записи
		return execlp("python3", "python3", "-i", NULL); // дитя пишет в дефолтный дескриптор а читает из to_child[0]
	}
	close(to_child[0]); // если мы в родителе закрываем дескриптор чтения
	const char cmd[] = "print(100 + 200)";
	write(to_child[1], cmd, sizeof(cmd)); // дескриптор to_child[1] пишет так что его можно прочитать из to_child[0] заменяющего std read
	close(to_child[1]); //закрываем запись в ребенка, потому что иначе наш стдинпут фризит т.к. стдин закрыт но ребенок требует ввод
	wait(NULL);
	return 0;
/*
	объяснение 2:
	имеем 2 дескриптора, to_child 0, 1
	to_child 0 -- стандартный ввод, т.е. весь ввод теперь идет через дескриптор, а не stdin
	оба дескриптора дублируются в ребенке и т.к. to_child[1] пишет в to_child[0] он нам не нужен, мы не хотим из ребенка писать в него же
	если мы в родителе то мы не ждем ввода, а потому закрываем дескриптор ввода подмененный to_child[0]
	затем мы пишем в дескриптор to_child[1], он пишет в to_child[0] потому что жива ссылка на него в ребенке
	ребенок читает из открытого to_child[0] как из stdin и пишет в дефолтный не подмененный stdout
*/

///mvp
// int descriptors[2];
// pipe(descriptors);
// dup2(descriptors[0], 0);
// if(fork()==0){
//     close(descriptors[1]);
//     return execlp("python3", "python3", "-i", NULL);
// }
// write(descriptors[1], "print(\"abobus\")\n", sizeof("print(\"abobus\")\n"));
// // close(descriptors[1]);
// wait(NULL);
}

