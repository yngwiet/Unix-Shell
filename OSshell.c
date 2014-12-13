#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>


#define TRUE 1
#define SIZE 100

char inputCom; //get one char every time

char curDir[SIZE]; //current directory
char historyDir[SIZE];

char *comArgv[10]; //command arguments
int comArgc = 0;

char readStr[255];
char readStr1[255]; //a complete command
int strNum = 0; 
int i = 0; //For clearing char pointer
int j = 0; //For test the program
int rn = 0; //return value of execv()

FILE *fp; //for file history

int lineNum = 1;
char ch;

int fd;
int fd1;

int stop = 0;

int pipe_fd[2];
int pid[2];

int count=0;
int count3=0;
char *comArgv1[10];
char *comArgv2[10];
char *comArgv3[10];
int pid1;
int pid2;

int job_count = 0;
int job_num = 0;

int signal_num = 0;

int is_bg = 0; 
int is_fg = 0;
int is_to_bg=0;

// a node is considered as a job
typedef struct node{ 				
   int id;
   int isBg;
   char name[255];
   pid_t pid;
   char status[20];
   struct node *prev;
   struct node *next;
}node;

node *node_list;
node *first = NULL;
node *last = NULL;
node *cur_job = NULL;
node *w = NULL;

node* add_node(pid_t pid, char name[]){
   job_num++;
   cur_job = (node*) malloc(sizeof(node));
   strcpy(cur_job->name,name);
   cur_job->pid = pid;
   
   if (first == NULL) {
      job_count = 0;
      cur_job->prev = first;
      first = cur_job;
   }
   else{
      cur_job->prev = last;
      last->next = cur_job;
      cur_job->next = NULL;
   }
   job_count++;
   cur_job->id = job_count;
   last = cur_job;
   
   return first;
}

node* delete_node(pid_t pid3){
	node* p = NULL;
	job_num--;
	if(first == NULL)
		return NULL;
	p = first;
	while(p!= NULL){
		if(p->pid == pid3){
			if(p == first){
				first = p->next;
			}
			else if(p == last){
			    last = p->prev;
				last->next = NULL;
			}
			else{
			   p->prev->next = p->next;
			   p->next->prev = p->prev;
			}
			p = p->next;
		}
		else
			p = p->next;
	}
	return first;
}

void sig_handle_ctrl_z(int sig){
  if(sig == SIGTSTP){
	if(comArgv[0] == NULL){
	   
    }
    else{  
     // from old child process, press ctrl+z in parent process
     if(w != NULL && w->pid == pid1 && is_fg == 1){
	
		is_fg = 0;
       	
        strcpy(w->status,"Stopped");
        printf("\n");   
        printf("[%d]+ %s       %s\n",w->id,w->status,w->name);//status is not changed yet
		kill(w->pid,SIGTSTP);
     }
     //a new chlid process
     else if(first == NULL | (cur_job != NULL && cur_job->pid != pid1 && is_fg == 0)){
        node_list = add_node(pid1, readStr1);
        strcpy(cur_job->status,"Stopped");
		printf("\n");
        printf("[%d]+ %s       %s\n",cur_job->id,cur_job->status,cur_job->name);
        kill(pid1,SIGTSTP);//pause doesn't need waitpid
     }
     //from fg
     else if(w!=NULL && w->pid !=pid1 && is_fg ==1){
		is_fg = 0;
        
		strcpy(w->status,"Stopped");
		printf("\n");
	
		printf("[%d]+ %s       %s\n",w->id,w->status,w->name);
		kill(w->pid,SIGTSTP);
	 }
     
	}
  }

  else if(sig == SIGINT){
    if(comArgv[0] == NULL){
    }
    else{
     //from old child process, press ctrl+z in parent process
     if(w != NULL && w->pid == pid1 && is_fg == 1){
	    is_fg = 0; 
	    delete_node(w->pid); 
     }
     //a new chlid process
     else if(first == NULL | (cur_job != NULL && cur_job->pid != pid1 && is_fg == 0)){ 
     }
     //from fg
     else if(w!=NULL && w->pid !=pid1 && is_fg ==1){
	    is_fg = 0;
	    delete_node(w->pid);
     }
    }
  }
}

void redire_o(int count1){
   fd1 = dup(1);
   fd = open(comArgv[count1+1], O_WRONLY | O_CREAT | O_TRUNC, 00600);
   if(fd<0)
      printf("fd: open file wrong.\n");
   dup2(fd, 1);
   close(fd);
}


void back_STDOUT(){
   dup2(fd1, 1);
   close(fd1);
}


void redire_i(){
   fd1 = dup(0);
   fd = open(comArgv[count+1], O_RDONLY | O_CREAT, 00600);
   if(fd<0)
      printf("fd: open file wrong.\n");
   dup2(fd, 0);
   close(fd);
}


void back_STDIN(){
   dup2(fd1, 0);
   close(fd1);
}


void show_hist_re(){
   redire_o(1);
   lineNum = 1;
   fp = fopen(historyDir, "a+");
   printf("%d ",lineNum);
   ch = fgetc(fp);
    while(ch != EOF){
        if(ch == '\n'){
			ch = fgetc(fp);//skip the enter character
			if(ch != EOF){
				lineNum++;
				printf("\n%d ", lineNum);
			}
			else{
				printf("\n");
				break;
			}
        }
        printf("%c", ch);//print it to the screen
		ch = fgetc(fp);//get next character
    }
    fclose(fp);
    back_STDOUT();
}

void show_history(){
	lineNum = 1;
	fp = fopen(historyDir, "a+"); 
	       
	printf("%d ",lineNum);
	ch = fgetc(fp);
	while(ch != EOF){
		if(ch == '\n'){
			ch = fgetc(fp);//skip the enter character
			if(ch != EOF){
				lineNum++;
				printf("\n%d ", lineNum);
			}
			else{
				printf("\n");
				break;
			}
		}
		printf("%c", ch);//print it to the screen
		ch = fgetc(fp);//get next character
	}
	fclose(fp);   
}


//To organise a command from every char user types
void get_command(){
   //clear the array
   strNum = 0;
   
   //To form a whole command string
   while((inputCom != '\n') && (strNum < 255)){
	  readStr[strNum] = inputCom;
      readStr1[strNum] = inputCom;
      strNum++;
      inputCom = getchar();
      #ifdef DEBUG
      //printf for test
      printf("\033[34;1m" "%c\n" "\033[0m", inputCom);
      #endif
    }
   readStr[strNum] = '\0';
   readStr1[strNum] = '\0';

   //printf for test
   #ifdef DEBUG
   printf("\033[31;1m" "string in get_command():%s\n" "\033[0m", readStr);
   #endif
   
   fp = fopen(historyDir, "a+");      //file "history"     
   fputs(readStr, fp);
   fprintf(fp, "\n");
   fclose(fp);
}



//get tokens from a command
void read_command(){
   char *token; //strtok() to get tokens
   comArgc = 0;
   
   //get tokens
   token = strtok(readStr, " ");
   while(token != NULL){
      comArgv[comArgc] = token;
      comArgc++;
      token = strtok(NULL, " ");
     
    }
   comArgv[comArgc+1] = NULL;
}



//pipe
void os_pipe(){
	count++;
	count3 = 0;
    while(comArgv[count]!=NULL){
		comArgv3[count3]=comArgv[count];
        count3++;
        count++;
    }
    comArgv3[count3] = NULL;//The second command

	//create the pipe
	if(pipe(pipe_fd) < 0){
		perror("pipe failed");
	    exit(errno);
	}

	//create a child process
	if((pid[0] = fork()) < 0){
	    perror("fork failed");
	    exit(errno);
    }
	 
	//child process 1
	if(!pid[0]){
		close(pipe_fd[0]);
        dup2(pipe_fd[1], 1);
	    close(pipe_fd[1]);

	    //the command history
        if(strcmp(comArgv2[0], "history") == 0){
	        show_history();
	    }

	    //other build-in commands
	    else{
            rn = execvp(comArgv2[0], comArgv2);
			fprintf(stderr, "%s: command not found\n", comArgv2[0]);
            exit(-1);
	    }
	}


        
	//parent process
	else{
		close(pipe_fd[1]);
	    dup2(pipe_fd[0], 0);
	    close(pipe_fd[0]);

	    //the command history
	    if(strcmp(comArgv3[0], "history") == 0){
			show_history();
	    }

	    //other build-in commands
	    else{
			rn = execvp(comArgv3[0], comArgv3);
            fprintf(stderr, "%s: command not found\n", comArgv3[0]);
            exit(-1);
	    }
	    close(pipe_fd[0]);
        close(pipe_fd[1]); 
	    waitpid(pid[0], &rn, 0);   
    }
}


void bg(int id){
   pid_t pid; 
   node* p = NULL;
   
   p = first;
   
   while(p!=NULL){
	   if(p->id == id){
			printf("[%d]+      %s &\n",p->id,p->name);
			strcpy(p->status,"Running");
			p->isBg = 1;
			kill(p->pid,SIGCONT);

			pid = waitpid(p->pid,&rn,WNOHANG);
			p = p->next;
	
        }   
        else 
	        p = p->next;
    }
}


void fg(int id){
   
   is_bg = 0;
   is_fg = 1;
 
   w = first;
   signal(SIGTSTP,sig_handle_ctrl_z);
   signal(SIGINT,sig_handle_ctrl_z);
   while(w!=NULL){
       if(w->id == id){
		   strcpy(w->status,"Running");
		   w->isBg = 0;
       
		   printf("%s\n",w->name);
		   kill(w->pid,SIGCONT);
	       waitpid(w->pid,&rn,WUNTRACED);
	       if(WIFEXITED(rn) == TRUE){
	           strcpy(w->status,"Done");
	        } 

	       else if(WIFSIGNALED(rn) == TRUE){
	           strcpy(w->status,"Done");
	        }
	        break;
        }
        else{ 
			w = w->next;
        }
    }
}



//execute a command
int  execute_com(){

   #ifdef DEBUG
   for(j=0;j<comArgc;j++)
      printf("\033[35;1m" "string in parent execute_com():%s\n" "\033[0m", comArgv[j]);
   #endif

   //quit shell
   if(strcmp(comArgv[0], "exit") == 0){
       exit(0);
    }


   //change directory
   if(strcmp(comArgv[0], "cd") == 0){
       change_dir();
       return 0;
    } 

   //show status of last command
   if(strcmp(comArgv[0], "status") == 0){
      if(comArgv[1] != NULL){
         if(strcmp(comArgv[1], ">") == 0){
            redire_o(1);
            printf("status of last command: %d\n", rn);
     	    back_STDOUT();
         }
      }
      else
         printf("status of last command: %d\n", rn);
		 rn = 0;
      return 0;
    }

   //show history
   if(strcmp(comArgv[0], "history") == 0 && comArgv[1] == NULL){
        show_history();
		return 0;
    }
   if(strcmp(comArgv[0], "history") == 0 && strcmp(comArgv[1], ">") == 0){
        show_hist_re();
		return 0;
    }
   
   if(strcmp(comArgv[0], "jobs") == 0){
      //int k = 0;
      node* q = NULL;
      q = first;
      //jobs that are done from the background
      while(q != NULL){
	      if(q->isBg==1 && waitpid(q->pid,&rn,WNOHANG)!=0){
		      
		      strcpy(q->status,"Done");
		      q = q->next;
	      }
	      else q = q->next;
      }

      node* p = NULL;
      p = first;
      if(job_num!=0){
         while(p!=NULL){
            printf("[%d]+ %s       %s\n",p->id,p->status,p->name);
	    
	        if(strcmp(p->status,"Done") == 0){
				node_list = delete_node(p->pid);
			}
			p = p->next;
		}
      }
      return 0;
    }

   if(strcmp(comArgv[0], "bg")==0 && comArgv[1]!= NULL){
      int l;
      l = atoi(comArgv[1]);
      bg(l);
     
      return 0;
    }

   if(strcmp(comArgv[0], "fg")==0 && comArgv[1]!=NULL){
      int l;
      l = atoi(comArgv[1]);
      fg(l);

      return 0;
    }


    //build-in commands
    else{
      
      is_fg = 0;

      for(count=0;count<comArgc;count++){
         if(strcmp(comArgv[count], "&") == 0){
			is_bg = 1;
		}
	 
      }

      rn = 0;
      
      pid1 = fork();
      //fork(),create child process
      if(pid1 == 0){
         
         //For pipe
         for(count=0;count<comArgc;count++){
            if(strcmp(comArgv[count], "|") == 0){
               comArgv2[count] = NULL;
               os_pipe();
            }

			if(strcmp(comArgv[count], "&") == 0){
				comArgv[count] = NULL;
                rn = execvp(comArgv[0], comArgv);
				fprintf(stderr, "%s: command not found\n", readStr);
				exit(-1);
			}
            else{
	            comArgv2[count] = comArgv[count];
            }
	    }
         
         //For I/O redirection
         for(count=0;count<comArgc;count++){
            if(strcmp(comArgv[count], ">") == 0){
				comArgv1[count] = NULL;
				redire_o(count);
                rn = execvp(comArgv1[0], comArgv1);
				fprintf(stderr, "%s :command not found\n", comArgv1[0]);
				exit(-1);
	        }

            else if(strcmp(comArgv[count], "<") == 0){
	            comArgv1[count] = NULL;
	            redire_i();
				rn = execvp(comArgv1[0], comArgv1);
				fprintf(stderr, "%s :command not found\n", comArgv1[0]);
				exit(-1);
	        }
	   

			else if(count == (comArgc - 1)){
				back_STDOUT();
				rn = execvp(comArgv[0], comArgv);
       	        fprintf(stderr, "%s: command not found\n", readStr);
				exit(-1);
	        }


			else{
				comArgv1[count] = comArgv[count];
	        } 
		}
    }//end child process

    //parent
    else{
       signal(SIGINT,sig_handle_ctrl_z); 
       signal(SIGTSTP,sig_handle_ctrl_z);
       //running in foreground
       if(is_bg == 0){
	   //printf("running in foreground\n");
          waitpid(pid1, &rn, WUNTRACED);
        }
       //like sleep 10 &
       else if(is_bg == 1){
	      
			is_bg = 0;
            node_list = add_node(pid1,readStr1);//sleep 10 &
			cur_job->isBg = 1;
			strcpy(cur_job->status,"Running");
			printf("[%d]+ %d\n",cur_job->id,cur_job->pid);

            waitpid(pid1, &rn, WNOHANG);

        }
    }
   
   }

   return 0;
}


//change directory
int change_dir(){
   rn = 0;
   if(comArgv[1] == NULL || (strcmp(comArgv[1], "~") == 0))
      chdir(getenv("HOME"));
  
   else{
      if(chdir(comArgv[1]) == -1){
         printf(" %s: no such directory\n", comArgv[1]);
		 rn = -1;
      }
    }   
   getcwd(curDir,SIZE);

   return 0;
}


//clear the comArgv[] and the comArgv1[] char pointer
void clear_pointer(){

   for( i=0;i<comArgc;i++ ){
      comArgv[i] = NULL;
      comArgv1[i] = NULL;
      comArgv2[i] = NULL;
      comArgv3[i] = NULL;
   } 
}



int main(void){

   getcwd(curDir,SIZE);
   strcpy(historyDir, curDir);
   strcat(historyDir, "/history");
   printf("[OS shell:%s]$ ", curDir);
   
   while(TRUE){
     
      signal(SIGINT,sig_handle_ctrl_z);
      signal(SIGTSTP,sig_handle_ctrl_z);
      getcwd(curDir,SIZE);
      inputCom = getchar();
      
      #ifdef DEBUG
      //printf for test
      printf("\033[31;1m" "%c\n" "\033[0m", inputCom);
      #endif
      

      switch(inputCom){

        case '\n':
           printf("[OS shell:%s]$ ", curDir);
           break;

         default:
            get_command();
            read_command();
            execute_com();
	        clear_pointer();
	        printf("[OS shell:%s]$ ", curDir);
            break;
      }
 
   }
   
   
   return 0;
}
