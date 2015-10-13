void gotoxy(int x, int y)
{
	printf("\033[%d;%df", y, x);
	fflush(stdout);
}

void save_position()
{
	printf("\033[s");
	fflush(stdout);
}

void reset_position()
{
	printf("\033[u");
	fflush(stdout);
}

void* running_time()
{
	int i=0,j=0,k=0;
	for(i=0;i<24;k++)
	{
		save_position();

		gotoxy(0,0);

		printf("Tempo execucao: %02d:%02d:%02d",i,j,k);

		reset_position();
		sleep(1);

		if(k==59)
		{
			k=-1;
			if(j==59)
			{
				j=0;
				if(i==23)
					i=0;
				else
					i++;
			}
			else
				j++;
		}
	}
	return NULL;
}
