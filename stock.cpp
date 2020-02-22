#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#define DAYSUM 360
#define STOCKSUM 158000
#define STOCKNAME2 3000

//#define Debuging

using namespace std;

typedef struct{
	short date_year;
	short date_month;
	short date_day;
	double open;
	double high;
	double low;
	double close;
	double adj_close;
	long volume;
	char code[10];
	double change;
	double range;
	double pct_change;
}stockdata;

typedef struct
{
	int pos;
	double EMA_12;
	double EMA_26;
	double DIF;
	double DEA;
	double MACD;
}MACDK; 

typedef struct
{
	char code[10];
	char start_date[10];
	char end_date[10];
	int GoldCross;
	int DeadCross;
}MACDTOP;

const char *fFormat[10] = { //每个字段依次读取的格式

	"%d-%d-%d", //日期解释为三个整数
	"%lf",
	"%lf",
	"%lf",
	"%lf",
	"%lf",
	"%d",
	"%9s", //可以指定读取多少个字符； 股票代码长度固定  
	"%lf",
	"%lf"
};

struct STOCKNAME
{
	char code[10];
	int start;
	int end;
}stockname[10000];

typedef struct{
	char code[10];//用于存放股票代码
	long long start;//股票开始行  
	short  sameyear[18]={0};
	long long startyear[18];
	long long startmonth[18][13]={{0}};//初始为0，防止某些月不存在，在查询时直接排除掉 
	long long endmonth[18][13];//某年某月结束点 
}stockc;
stockc stockname1[STOCKNAME2];

int acount=0;
stockdata stock[7600000];
MACDTOP MacdTop[10000];
MACDK CalResult[10000];
int index[10000];
int len;
long long k=0;
int row_count = 0 ,stockvariety=1; 	
int count_stock = 1;
double tmpclose=0;
char fields[10][20];
//stockc stocknamez[STOCKNAME2];
char *name[STOCKNAME2]={0};
int dateindex[DAYSUM+10][STOCKNAME2]={0};
int datecount[DAYSUM+10]={0};
int yearleast=2017;

void idexcodeyear(FILE*fp,int row_count)//加载数据时建立 
{
	int m=0;
	if(row_count==0)
	{
	strcpy(stockname1[0].code,stock[0].code); 
	stockname1[0].sameyear[0]=stock[0].date_year ;
	stockname1[0].startyear [0]=0;
	stockname1[0].start =0;
	}
	else
	{
		if(strcmp(stockname1[acount].code,stock[row_count].code )!=0)
		{
			strcpy(stockname1[++acount].code, stock[row_count].code );
			stockname1[acount].sameyear [0]=stock[row_count].date_year ;
			stockname1[acount].start =row_count;
			stockname1[acount].startyear [0]=row_count;
			stockname1[acount].startmonth[0][stock[row_count].date_month ]=row_count;
			stockname1[acount-1].endmonth [m][stock[row_count-1].date_month ]=row_count;
			m=0;
		}
		else
		{
			if(stock[row_count].date_year !=stockname1[acount].sameyear[m]) 
			{
				stockname1[acount].sameyear [++m]=stock[row_count].date_year ;
				stockname1[acount].startyear[m]=row_count;
				stockname1[acount].startmonth[m][stock[row_count].date_month ]=row_count;
				stockname1[acount].endmonth [m-1][stock[row_count-1].date_month ]=row_count;
			}
			else if(stock[row_count].date_month !=stock[row_count-1].date_month )
			{
				stockname1[acount].startmonth[m][stock[row_count].date_month ]=row_count;
				stockname1[acount].endmonth [m][stock[row_count-1].date_month ]=row_count;
			}
			
		}
	}
	
}


int IndexSetting(FILE*fp,int rowcount)//创建索引的函数 （置于main函数中,在调用rowscan函数的后面，逐行扫描读入） 
{
	if(rowcount == 0)  
	{
		strcpy(stockname[count_stock].code,stock[rowcount].code);  
		stockname[count_stock].start = 0;  
		//cout<<stockname[0].start<<endl;  
	}
	
	else if(strcmp(stock[rowcount].code,stock[rowcount - 1].code) != 0)//如果不是同一支股票   
	{
		strcpy(stockname[++count_stock].code,stock[rowcount].code);  
		stockname[count_stock].start = rowcount;
		//这一支股票的开始   
		//stockname[count - 1].end = rowcount - 1;//上一支股票的结束 (可以省略)
	} 
	stockname[count_stock].end = rowcount;//默认将每一个当前位置记录为当前股票的尾角标 
	//cout<<stockname[0].code<<endl;
	return count_stock;//返回共有多少支股票 
}

FILE * loadDataSrcFile(char *filename){
	
	FILE *fp;
	
	if ((fp = fopen(filename, "r")) == NULL) {
		puts("-1\n");
		exit(0);//退出程序		
	}

	return fp; //退出子函数
}

int rowscan(FILE *fp)
{
	int eofsig;
	char buff[30] = {0}, c;	//缓存字符数组	
	bool sign_change=1;
	int field_count = 0;  		
	for(int i = 0; !feof(fp) && i<10; i++)
	{//固定顺序的10个字段
		
		int idx = 0; //重置缓存空间
		while((c = fgetc(fp)))
		{//扫描读取一个字段， EOF表示文件已结束 							
			if(c == ',' || c == '\n' || c == EOF )
			{//字段数据结束
				buff[idx] = 0;
				break;
			}
			else
			{//把每个字段的数据放入缓存
				buff[idx++] = c;
			}
		}
		
		len = strlen(buff);		
		
		switch(i)
		{
			case 0:
				sscanf(buff, fFormat[i], &stock[k].date_year,&stock[k].date_month,&stock[k].date_day);
				break;
			case 1:
				if(len==0) stock[k].open=tmpclose;
					else sscanf(buff, fFormat[i], &stock[k].open);
				break;
			case 2:
				if(len==0) stock[k].high=tmpclose;
					else sscanf(buff, fFormat[i], &stock[k].high);
				break;
			case 3:
				if(len==0) stock[k].low=tmpclose;
					else sscanf(buff, fFormat[i], &stock[k].low);
				break;
			case 4:
				if(len==0) stock[k].close=tmpclose;
					else sscanf(buff, fFormat[i], &stock[k].close);
				break;
			case 5:
				if(len==0) stock[k].adj_close=tmpclose;
					else {
					sscanf(buff, fFormat[i], &stock[k].adj_close);
					tmpclose=stock[k].adj_close;
					}
				break;
			case 6:
				if(len==0) stock[k].volume=0;
					//else sscanf(buff, fFormat[i], &stock[k].volume);
					stock[k].volume = atol(buff);
				break;
			case 7:
				//strcpy(Code, buff);
				sscanf(buff, fFormat[i], stock[k].code);
				break;
			case 8:
				if(len==0) sign_change=0;
				sscanf(buff, fFormat[i], &stock[k].change);
				break;
			case 9:
				if(len==0) stock[k].pct_change=0;
					else sscanf(buff, fFormat[i], &stock[k].pct_change);
				break;
		}
		
		field_count++;
	}
	
	if(sign_change==0){
		stock[k].change=stock[k].pct_change*stock[k-1].close;
		sign_change=1;
	}
		stock[row_count].range=(stock[row_count].high-stock[row_count].low)/stock[row_count].open;
	
	//if(row_count>10200) printf("%d %d %lf %lf %lf %d\n",stock[row_count].volume,stock[row_count].date_day,stock[row_count].high,stock[row_count].adj_close,stock[row_count].change,row_count);
	int tmp=(stock[row_count].date_year-yearleast)*366+(stock[row_count].date_month-1)*31+stock[row_count].date_day-1;

	if(stock[row_count].date_year!=0){
		dateindex[tmp][datecount[tmp]]=row_count;
		datecount[tmp]++;
	}
	
	if(stock[row_count].date_year!=0&&stock[row_count].date_year<yearleast){
		yearleast=stock[row_count].date_year;
	}
		
//	if(row_count%1000==0) cout<<stock[row_count].volume<<endl;
	
	if(eofsig==1) return field_count;
	
	k++;
	return field_count; //返回读取的非空字段数
	
}

int LoadStockData(char *filename)
{
	
	FILE *fp = loadDataSrcFile(filename);
	
	for(int i=0; i<9; i++)
	{//读第一行的字段名
		fscanf(fp, "%[^,],", fields[i]); //%[^,] 表示一直读取到逗号,为止。
	}
	fscanf(fp, "%s", fields[9]);	
	fgetc(fp); //跳过第一行的回车符

	while(rowscan(fp))
	{//扫描文件中的每行数据
		
		IndexSetting(fp,row_count);
		idexcodeyear(fp,row_count);
		row_count++;
	}
	fclose(fp);
	return (row_count);
}

//#define Debuging1
int PreDateFinder(int y,int m,int d,int search_start,int search_end)
{
	for(int i =search_end;i >= search_start;i--)
	{
		if(y >= stock[i].date_year)
		{
			if(m >= stock[i].date_month)
			{
				if(d > stock[i].date_day)
				{
					return i;
				}
			}
		}
	}
}

int NextDateFinder(int y,int m,int d,int search_start,int search_end)
{
	for(int i =search_start;i <= search_end;i++)
	{
		if(y <= stock[i].date_year)
		{
			if(m <= stock[i].date_month)
			{
				if(d < stock[i].date_day)
				{
					return i;
				}
			}
		}
	}
}

int SearchPos(char *code ,int Y,int M,int D,int op)
{
	int count = 0;
	int i = 0;
	int flag = 0;
	int search_start,search_end;  
	while((i <= count_stock) && strcmp(stockname[i].code, code) != 0)
	{
		count++;
		//search_start = stockname[i].start,search_end = stockname[i].end;
		i++;
		//cout<<i<<" ";
	}
	//cout<<count<<endl;
	
	search_start = stockname[i].start,search_end = stockname[i].end;
	
	//cout<<stockname[0].start<<" "<<stockname[0].end<<endl;
	
	for(int j = search_start;j <= search_end;j++)
	{
		if(Y == stock[j].date_year)
		{
			if(M == stock[j].date_month)
			{
				if(D == stock[j].date_day)
				{
					#ifdef Debuging1
					flag = 1;
					cout<<j<<endl;
					#endif 
					return j;
				}
			}
		}
	}
	if((flag == 0)&&(op))
	{
		return (NextDateFinder(Y,M,D,search_start,search_end));
		//SearchPos(code,Y,M,D,1);
	}
	else if(op == 0)
	{
		return (PreDateFinder(Y,M,D,search_start,search_end));
		//SearchPos(code,Y,M,D,0);
	}
}

void CalcData(int pos_pre,int pos_start, int pos_end, MACDK CalResult[])
{
	int count1 = pos_start - pos_pre;
	int count2 = pos_end - pos_start;
	int count_ = count1 + count2;
	
	CalResult[0].EMA_12 = stock[pos_pre].close;
	CalResult[0].EMA_26 = stock[pos_pre].close;
	
	CalResult[0].DIF = CalResult[0].EMA_12 - CalResult[0].EMA_26;
	CalResult[0].DEA = 0.0;
	CalResult[0].MACD = 0.0;
	CalResult[0].pos = pos_pre;
	int flag = 1;
	//MACD(i) = 2 × (DIF(i) - DEA(i))
	//DEA(i-1) × 0.8 + DIF(i) × 0.2
	//MACD(i) = 2 × (DIF(i) - DEA(i))
	for(int i = 1;i <= count_;i++)
	{
		int pos = CalResult[0].pos + i;
		 
		CalResult[i].EMA_12 = CalResult[i - 1].EMA_12 * 11 / 13 + stock[pos].close * 2 / 13;
		CalResult[i].EMA_26 = CalResult[i - 1].EMA_26* 25 / 27 + stock[pos].close * 2 / 27;
		CalResult[i].DIF = CalResult[i].EMA_12 - CalResult[i].EMA_26;
		CalResult[i].DEA = CalResult[i - 1].DEA * 0.8 + CalResult[i].DIF * 0.2;
		CalResult[i].MACD = 2 * (CalResult[i].DIF - CalResult[i].DEA);
		CalResult[i].pos = pos;
		if(i == count1 && flag)
		{
			CalResult[0].EMA_12 = CalResult[i].EMA_12;
			CalResult[0].EMA_26 = CalResult[i].EMA_26;
			CalResult[0].DIF = CalResult[i].DIF;
			CalResult[0].DEA = CalResult[i].DEA;
			CalResult[0].MACD = CalResult[i].MACD;
			CalResult[0].pos = pos_start;
			i = 0;count_ = count2;
			flag = 0;
		}
	}
	CalResult[count2 + 1].pos = -1;
}

void print1(int count2,MACDK CalResult[])
	{
		for(int i = 0;i <= count2;i++)
		{
			printf("%s\t %d-%d-%d\t %.2f\t %-10d %-10.4f\t %-8.4f %-8.4f %-8.4f %-8.4f \n",stock[CalResult[i].pos].code,stock[CalResult[i].pos].date_year,
			stock[CalResult[i].pos].date_month,stock[CalResult[i].pos].date_day,stock[CalResult[i].pos].close,stock[CalResult[i].pos].volume,CalResult[i].EMA_12,CalResult[i].EMA_26,
			CalResult[i].DIF,CalResult[i].DEA,CalResult[i].MACD);//逐行输出； 
		}
	}
	
void My_CalcMacd(char *code,char *start_date,char *end_date,bool output)
{
	int pos_pre,pos_start,pos_end,count2;
	int Y[3],M[3],D[3];
	
	sscanf(start_date,"%d-%d-%d",&Y[0],&M[0],&D[0]);
	sscanf(end_date,"%d-%d-%d",&Y[1],&M[1],&D[1]);
	//sscanf(start_date,"%d-%d-%d",&Y[2],&M[2],&D[2]);
	
	//PreDateFinder(&Y[2],&M[2],&D[2]);
	//cout<<M[2]<<D[2]<<M[0]<<D[0]<<endl;
	pos_start = SearchPos(code,Y[0],M[0],D[0],1);
	pos_end = SearchPos(code,Y[1],M[1],D[1],0);
	//pos_pre = SearchPos(code,Y[2],M[2],D[2]);
	pos_pre = pos_start - 30;
	#ifdef Debuging
	cout<<pos_start<<"! "<<pos_end<<"! "<<pos_pre<<endl;
	#endif
	count2 = pos_end - pos_start;
	
	CalcData(pos_pre, pos_start, pos_end, CalResult);
	
	if(output == true)
	print1(count2,CalResult);
	
}


void CalcMacd(char *code ,char *strat_date ,char *end_date)
{
	My_CalcMacd(code,strat_date,end_date,true);
}

void CrossCalc(int pos,double dis_pre)
{
	int i = 2;
	double dis = 0.0;
	//double dis_pre = CalResult[0].DIF - CalResult[0].DEA;
	MacdTop[pos].GoldCross = MacdTop[pos].DeadCross = 0;
	
	while(CalResult[i].pos != -1)
	{
		dis = CalResult[i].DIF - CalResult[i].DEA;
		if(dis * dis_pre < 0)//没有考虑DIF和DEA相等的情况，黄线和白线应该不会重合吧。。。 
		{
			if(dis > 0)
			{
				MacdTop[pos].GoldCross++;
			}
			else
			{
				MacdTop[pos].DeadCross++;
			}
		}
		i++;
		dis_pre = dis; 
	}
}

int cmp(const void *p1,const void *p2)
{
	int r1 = *(int*)p1;
	int r2 = *(int*)p2;
	
	if(MacdTop[r1].GoldCross < MacdTop[r2].GoldCross)
		{
			return 1;
		}
	else if(MacdTop[r1].GoldCross == MacdTop[r2].GoldCross)
	{
		if(MacdTop[r1].DeadCross > MacdTop[r2].DeadCross)
		{
			return 1;
		}
		else if(MacdTop[r1].DeadCross == MacdTop[r2].DeadCross)
		{
			if(strcmp(MacdTop[r1].code,MacdTop[r2].code) > 0)
			{
				return 1;
			}
			else if(strcmp(MacdTop[r1].code,MacdTop[r2].code) == 0)
				return 0;
			else	
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;
}

//股票代码 起始日期 终止日期 金叉点个数 死叉点个数
int print2(int s[],int k)
{
	int count = 0;
	for(int i = 1;i < k;i++,count++)
	{
		if(strlen(MacdTop[i].code) == 0)
		break;
		printf("%s\t%s\t%s\t%d\t%d\n",MacdTop[s[i]].code,MacdTop[s[i]].start_date,
		MacdTop[s[i]].end_date,MacdTop[s[i]].GoldCross,MacdTop[s[i]].DeadCross);
	}
	return count;
}

int My_MACDTopK(char *start_date,char *end_date,int k,bool output)
{
	char *code = NULL;MACDK *getdata = CalResult;
	
	for(int i = 1;i <= count_stock;i++)
	{	
	
		index[i] = i;
		
    
		strcpy(MacdTop[i].code, stockname[i].code);
		strcpy(MacdTop[i].start_date,start_date);
		strcpy(MacdTop[i].end_date,end_date);
		
		My_CalcMacd(MacdTop[i].code,start_date,end_date,false);
		//cout<<1; 
		double dis_pre = CalResult[1].DIF - CalResult[1].DEA;
		CrossCalc(i,dis_pre);
	}
	//MacdTop[10000].code = {'\0'};
	
	qsort(index,count_stock,sizeof(index[0]),cmp); 
	
	if(output == true)
	{
		k = print2(index,k);
	}
	return k;
}

int MACDTopK(char *start_date,char *end_date,int k)
{
	k = My_MACDTopK(start_date,end_date,k,true);
	return k;
} 

int Query(char * para_str ){


	int l=strlen(para_str);
	char samecode[10]={'\0'};
	char w[5]={'\0'};
	int ot=0;
	int syear=0;
	int smonth=0;//-C 000001.SZ -h -d 2017-01-03
	int sday=0;
	for(int i=0;i<l;i++)
	{
	if(para_str[i]=='C')
		{
			for(int j=i+2;j<i+11;j++)
			{
				samecode[j-2-i]=para_str[j];
			}
			i+=11;
			
		}
		else if(para_str[i]=='d')
		{
			int r;
			for(r=i+2;r<i+6;r++)
			{
				syear=syear*10+(para_str[r]-'0');
			}
			for( r=i+7;r<i+9;r++)
			{
				smonth=smonth*10+(para_str[r]-'0');
			}
			for(r=i+10;r<i+12;r++)
			{
				sday=sday*10+(para_str[r]-'0');
			}
			i+=11;
		}
		else if(para_str[i]=='o'|| para_str[i]=='a'||para_str[i]=='h'||para_str[i]=='l'||para_str[i]=='c') 
		{
			w[ot]=para_str[i];
			ot+=1;
		}
		
	}
	int found;
	int exitin=0;
	for(int i=0;i<acount;i++)
	{
		if(strcmp(stockname1[i].code,samecode)==0)
		{
		found=i;
		i=acount;
		exitin=1;
		}
	}
	if(exitin==0) 
	{
		return -1;
	}
	else
	{
	long long maid=-1;
	for(int i=0;i<18;i++)
	{
		if(stockname1[found].sameyear [i]==syear)
		{
			if((found!=0 || i!=0 || smonth!=1 )&& stockname1[found].startmonth[i][smonth]==0) break;//该年该月没有数据，只有第一个的第一年第一月可以起始为0 
			long long endmon;
			if(found==acount-1) endmon=STOCKSUM;
			else endmon=stockname1[found].endmonth[i][smonth];
			 
			for(long long j=stockname1[found].startmonth[i][smonth];j<endmon;j++)
			{
				if(stock[j].date_day ==sday) 
				{
					maid=j;
					i=18;
					exitin=1;
					break;
				}
			}
		}
	}

	
	if(exitin==0||maid==-1) 
	{
		return -1;
	}
	else
	{
		if(smonth<10 && sday<10)
		cout<<samecode<<" "<<syear<<'-'<<'0'<<smonth<<'-'<<'0'<<sday<<" ";
		else if(smonth<10 && sday>=10)
		{
			cout<<samecode<<" "<<syear<<'-'<<'0'<<smonth<<'-'<<sday<<" ";
		}
		else if(smonth>=10 && sday<10)
		{
			cout<<samecode<<" "<<syear<<'-'<<smonth<<'-'<<'0'<<sday<<" ";
		}
		else
		{
			cout<<samecode<<" "<<syear<<'-'<<smonth<<'-'<<sday<<" ";
		}
	for(int i=0;i<ot;i++)
	{
		if(w[i]=='c') cout<<'-'<<'c'<<" "<<stock[maid].close<<" ";
		if(w[i]=='a') if((long)(stock[maid].adj_close *1000000)%10!=0)cout<<'-'<<'a'<<" ",printf("%.6f",stock[maid].adj_close ),cout<<" ";
		else  cout<<'-'<<'a'<<" "<<stock[maid].adj_close <<" ";
		if(w[i]=='h') cout<<'-'<<'h'<<" "<<stock[maid].high<<" ";
		if(w[i]=='l') cout<<'-'<<'l'<<" "<<stock[maid].low<<" ";
		if(w[i]=='o') cout<<'-'<<'o'<<" "<<stock[maid].open<<" ";
	}
	return 0;
	}
	}
	
}

int compbtsvol(const void *a,const void *b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
	
	if(stock[p1].volume>stock[p2].volume)
		return -1;
	else if(stock[p1].volume<stock[p2].volume)
		return 1;
	else return strcmp(stock[p1].code,stock[p2].code);
	
}

int compstbvol(const void*a,const void*b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
		
	if(stock[p1].volume>stock[p2].volume)
		return 1;
	else if(stock[p1].volume<stock[p2].volume)
		return -1;
	else return strcmp(stock[p1].code,stock[p2].code);
	
}

int compbtsran(const void*a,const void*b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
		
	if(stock[p1].range>stock[p2].range)
		return -1;
	else if(stock[p1].range<stock[p2].range)
		return 1;
	else return strcmp(stock[p1].code,stock[p2].code);	
}

int compstbran(const void*a,const void*b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
		
	if(stock[p1].range>stock[p2].range)
		return 1;
	else if(stock[p1].range<stock[p2].range)
		return -1;
	else return strcmp(stock[p1].code,stock[p2].code);	
}

int compbtsadj(const void*a,const void*b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
		
	if(stock[p1].adj_close>stock[p2].adj_close)
		return -1;
	else if(stock[p1].adj_close<stock[p2].adj_close)
		return 1;
	else return strcmp(stock[p1].code,stock[p2].code);	
}

int compstbadj(const void*a,const void*b)
{
	int p1=*((int *)a);
	int p2=*((int *)b);
		
	if(stock[p1].adj_close<stock[p2].adj_close)
		return -1;
	else if(stock[p1].adj_close>stock[p2].adj_close)
		return 1;
	else return strcmp(stock[p1].code,stock[p2].code);	
	
}

int TopK(char *date, char *data, int k, int desc)
{	
	int Y,M,D;
	sscanf(date,"%d-%d-%d", &Y,&M,&D);
	int tmp=(Y-yearleast)*366+(M-1)*31+D-1;
	//printf("%d",datecount[tmp]);
	
	if(datecount[tmp]==0){
		printf("-1\n");
		return -1;
	}
	
	int *specidata=NULL;
	
	specidata=(int (*))calloc(datecount[tmp],sizeof(int));
	
	for(int x=0;x<datecount[tmp];x++)
	{
		//memcpy(specidata+x,stock[dateindex[tmp][x]],sizeof(stock[0]));
		specidata[x]=dateindex[tmp][x];
	 } 
	
	if(strcmp(data,"Volumn")==0){
		if(desc==0) qsort(specidata,datecount[tmp],sizeof(int),compbtsvol);
		else qsort(specidata,datecount[tmp],sizeof(int),compstbvol);
	}
	if(strcmp(data,"Range")==0){
		if(desc==0) qsort(specidata,datecount[tmp],sizeof(int),compbtsran);
		else qsort(specidata,datecount[tmp],sizeof(int),compstbran);
	}
	if(strcmp(data,"AdjClose")==0){
		if(desc==0) qsort(specidata,datecount[tmp],sizeof(int),compbtsadj);
		else qsort(specidata,datecount[tmp],sizeof(int),compstbadj);
	}
	
	for(int i=0;i<k;i++)
	{
		printf("%s %d/%d/%d %.2f %.2f %.6f %.2f %d\n",
		stock[specidata[i]].code,stock[specidata[i]].date_year,
		stock[specidata[i]].date_month,stock[specidata[i]].date_day,
		stock[specidata[i]].open,stock[specidata[i]].close,stock[specidata[i]].adj_close,
		stock[specidata[i]].pct_change,stock[specidata[i]].volume);
	}
	
	return 0;
}

int SearchData(char *code ,int Y,int M,int D,int sig)
{
	int std=(Y-yearleast)*366+(M-1)*31+D-1;
    int i = 0;
    int search_start=stockname[0].start,search_end=stockname[0].end;
    for(;i<count_stock;i++)
    {
    	if(strcmp(stockname[i].code,code) == 0) break;
	}
    search_start = stockname[i].start;
    search_end = stockname[i].end;
    int tmptime[2]={0};
    for(int j = search_start;j <= search_end;j++)
    {	
    	if(j==search_start){
    		tmptime[0]=(stock[j].date_year-yearleast)*366+(stock[j].date_month-1)*31+stock[j].date_day-1;
    		tmptime[1]=(stock[j+1].date_year-yearleast)*366+(stock[j+1].date_month-1)*31+stock[j+1].date_day-1;
		}
		else if(j!=search_start){
			tmptime[0]=tmptime[1];
			tmptime[1]=(stock[j+1].date_year-yearleast)*366+(stock[j+1].date_month-1)*31+stock[j+1].date_day-1;
		}
		else{
			tmptime[0]=(stock[j].date_year-yearleast)*366+(stock[j].date_month-1)*31+stock[j].date_day-1;
			tmptime[1]=(stock[j].date_year-yearleast)*366+(stock[j].date_month-1)*31+stock[j].date_day-1;
		}
		
	    if(tmptime[0]>=std&&sig==0)
        {
            return j;
        }
        if(tmptime[0]<=std&&sig==1&&tmptime[1]>=std)
        {
        	return j;
		}
    }
    return -1;
}


double bl4(double p)
{
	int a = 0;
	a = ((p * 10000) + 0.5);;
	return (a / 10000.0);
}

double Calculate(char *para_str)
{
	char m,n;
	char samecode[10]={0}; 
	char start_date[15]={0},end_date[15]={0};
	int l=strlen(para_str);
	int j,k,num=0;
	int Y[2],M[2],D[2];
	int pos_start,pos_end;
	int ylx=0;
	int i=0;
	for(;i<l;i++)
	{
		if(para_str[i]=='C')
		{
			i+=2;
			for(int j=0;i<l;i++,j++)
			{
				if (para_str[i]==' '||para_str[i]=='-')break;
				samecode[j]=para_str[i];
			}
		}
		else if(para_str[i]=='d')
		{
			if(ylx==0)
			{
				i+=2;
				for(int j=0;para_str[i]!=' ';i++,j++)
				{
					start_date[j]=para_str[i];
				}
				ylx=1;
			}
			else{
				i+=2;
				for(int j=0;para_str[i]!=' ';i++,j++)
				{
					end_date[j]=para_str[i];
				}
			}
			
		}
		
		else if(para_str[i]=='o'|| para_str[i]=='c'||para_str[i]=='h'||para_str[i]=='l') 
		{
			m=para_str[i];
		}
		else if(para_str[i]=='v'|| para_str[i]=='s')
		{
			n=para_str[i];
		}
	}
	sscanf(start_date,"%d-%d-%d",&Y[0],&M[0],&D[0]);
	sscanf(end_date,"%d-%d-%d",&Y[1],&M[1],&D[1]);
    double sum=0.0,aver=0.0,e=0.0;
	pos_start=SearchData(samecode, Y[0], M[0], D[0],0);
	if(pos_start==-1) 
	{
		cout<<-1;
		return -1;
	}
        pos_end=SearchData(samecode, Y[1], M[1], D[1],1);
		for(j=pos_start;j<=pos_end;j++)
   		{
    		num++;
    	    if(m=='o'){
			 	sum+=stock[j].open;continue;
        	}
        else if(m=='c'){
            	sum+=stock[j].close;continue;
        	}
        else if(m=='h'){
            	sum+=stock[j].high;continue;
        	}
        else if(m=='l'){
            	sum+=stock[j].low;
        	}
    	}
    	aver=sum/num;
    	if(n=='s'){
    		for(k=pos_start;k<=pos_end;k++)
    		{
        		if(m=='o'){
            		e+=(stock[k].open-aver)*(stock[k].open-aver);
        		}
        		if(m=='c'){
            		e+=(stock[k].open-aver)*(stock[k].open-aver);
        		}
        		if(m=='h'){
            		e+=(stock[k].high-aver)*(stock[k].high-aver);
        		}
        		if(m=='l'){
            		e+=(stock[k].low-aver)*(stock[k].low-aver);
        		}
    		}
   			e=sqrt(e/num);
			cout<<bl4(e);
			return bl4(e);
		}
		else 
		{
			cout<<bl4(aver);
			return bl4(aver);
		}
}


int min(int a,int b,int c)  
{  
    int tmp=a<b?a:b;  
    return tmp<c?tmp:c;  
}


int FuzzyMatch (char *query, int threshold)
{
	printf("实现你的模糊匹配查询功能\n"); 
	
	int len_b=strlen(query); 
	int count=0;//计数，一共有多少个模糊code 
	
	//下面都在计算编辑距离	
	for(int l=1;l<=count_stock;l++)//这里的 row_count 是stockname 数组存放数据的个数 (股票的总只数） 
							//也可以是 stock数组存放数据的个数
	{	
		int len_a=strlen(stockname[l].code);
		int dist[10][20]={0}; 
		for (int i=0;i<=len_a;i++)  
    	{  
    		dist[i][0]=i;
    	}
    	for (int j=0;j<=len_b;j++)  
    	{  
   			dist[0][j]=j;
		}  
    	for (int i=1;i<=len_a;i++)  
    	{  
        	for (int j=1;j<=len_b;j++)  
        	{  
            	if(stockname[l].code[i-1]==query[j-1]){
            		dist[i][j]=min(dist[i-1][j-1],dist[i][j-1]+1,dist[i-1][j]+1);
				}
				else {
					dist[i][j]=min(dist[i-1][j-1]+1,dist[i][j-1]+1,dist[i-1][j]+1);
				}	
        	}  
    	} 
		int n=dist[len_a][len_b];//编辑距离 
		if(n<=threshold)
		{
			count++;
			cout<<stockname[l].code<<endl;
		} 
  	}
	return count;

}

int main()
{
	//sh_sz_A_stock_data 
	//FILE *fp = loadDataSrcFile("test1.csv");
		                               //↑这个自己改 
	LoadStockData("F://stock_data_sample_2016_rep.csv");
//	cout<<endl;	      
	
	//My_CalcMacd("000001.SZ","2016-2-18","2017-2-18",true); 
	  
//	CalcMacd("002222.SZ","2016-02-24","2016-03-29");
//	cout<<endl;
//	TopK("2016-02-03","Range",10,0);
//	cout<<endl;
//	Query("-C 600689.SS -c -o -d 2016-03-28");
//	cout<<endl;
//	FuzzyMatch ("100008-ST",3);
//	cout<<endl;
//	Calculate("-C 300292.SZ -d 2016-01-22 -d 2016-03-31 -c -s");
//	cout<<endl;
//	MACDTopK("2011-2-29","2016-3-9",10);
//	cout<<endl;
//	Calculate(char *para_str)
//	cout<<count_stock<<endl; 
	//cout<<stockname[10].code;
	//fclose(fp);
	
	return 0;
}
