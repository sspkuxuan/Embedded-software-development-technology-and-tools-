#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define W_sleep 1 //写频率
#define R_sleep 1 //读频率

int ReaderCount=0;//读者计数器
int WriterCount=0;//写者计数器
int SharedData=0;//读者-写着共享数据
pthread_mutex_t mutex_write;//写者互斥锁
sem_t sem_readercount;//信号量控制只有一个线程可以对读者计数器操作
sem_t sem_writercount;//信号量控制只有一个线程可以对写者计数器操作
//写操作函数
void write()
{
	int temp=rand();
	printf("写入数据 %d\n",temp);
	SharedData=temp;
}
//读操作函数
void read()
{
	printf("读到数据 %d\n",SharedData);
}
//读者优先时的写者控制函数
void *writerOfRF(void* in)
{
	while(1)
	{
		pthread_mutex_lock(&mutex_write);
		printf("写线程id %d 进入数据集\n",pthread_self());
		write();
		printf("写线程id %d 退出数据集\n",pthread_self());
		pthread_mutex_unlock(&mutex_write);
		sleep(W_sleep);
	}
	pthread_exit((void*)0);
}
//读者优先时的读者控制函数
void *readerOfRF(void* in)
{
	while(1)
	{
		sem_wait(&sem_readercount);
		ReaderCount++;
		if(ReaderCount>0)
			pthread_mutex_lock(&mutex_write);
		sem_post(&sem_readercount);
		printf("读线程id %d 进入数据集\n",pthread_self());
		read();
		printf("读线程id %d 退出数据集\n",pthread_self());
		sem_wait(&sem_readercount);
		ReaderCount--;
		if(ReaderCount==0)
			pthread_mutex_unlock(&mutex_write);
		sem_post(&sem_readercount);
		sleep(R_sleep);
	}
	pthread_exit((void*)0);
}
//写者优先时的写者控制函数
void *writerOfWF(void* in)
{
	while(1)
	{
		sem_wait(&sem_writercount);
		WriterCount++;
		//sem_post(&sem_writercount);
		pthread_mutex_lock(&mutex_write);
		printf("写线程id %d 进入数据集\n",pthread_self());
		write();
		printf("写线程id %d 退出数据集\n",pthread_self());
		pthread_mutex_unlock(&mutex_write);
		//sem_wait(&sem_writercount);
		WriterCount--;
		sem_post(&sem_writercount);
		sleep(W_sleep);
	}
	pthread_exit((void*)0);
}
//写者优先时的读者控制函数
void *readerOfWF(void* in)
{
	while(1)
	{
		if(WriterCount==0)
		{
			sem_wait(&sem_readercount);
			ReaderCount++;
			if(ReaderCount>0)
				pthread_mutex_lock(&mutex_write);
			sem_post(&sem_readercount);
			printf("读线程id %d 进入数据集\n",pthread_self());
			read();
			printf("读线程id %d 退出数据集\n",pthread_self());
			sem_wait(&sem_readercount);
			ReaderCount--;
			if(ReaderCount==0)
				pthread_mutex_unlock(&mutex_write);
			sem_post(&sem_readercount);
			sleep(R_sleep);
		}
	}
	pthread_exit((void*)0);
}
int main()
{
	pthread_mutex_init(&mutex_write,NULL);
	sem_init(&sem_readercount,0,1);
	sem_init(&sem_writercount,0,1);
	int readernum,writernum;
	printf("请分别输入读者人数和写者人数：");
	scanf("%d%d",&readernum,&writernum);
	int choice;
	printf("读者优先模式输入0，写者优先模式输入1:");
	scanf("%d",&choice);
	pthread_t wid[writernum],rid[readernum];
	int i=0;
	if(choice==0)
	{
		for(i=0;i<writernum;i++)
			pthread_create(&wid[i],NULL,writerOfRF,NULL);
		for(i=0;i<readernum;i++)
			pthread_create(&rid[i],NULL,readerOfRF,NULL);
		sleep(20);
		for(i=0;i<writernum;i++)
			pthread_join(wid[i],NULL);
		for(i=0;i<readernum;i++)
			pthread_join(rid[i],NULL);
	}
	else if(choice==1)
	{
		for(i=0;i<writernum;i++)
			pthread_create(&wid[i],NULL,writerOfWF,NULL);
		sleep(1);
		for(i=0;i<readernum;i++)
			pthread_create(&rid[i],NULL,readerOfWF,NULL);
		sleep(20);
		for(i=0;i<writernum;i++)
			pthread_join(wid[i],NULL);
		for(i=0;i<readernum;i++)
			pthread_join(rid[i],NULL);
	}else{printf("没有这个选项！\n");}
	return 0;
}
