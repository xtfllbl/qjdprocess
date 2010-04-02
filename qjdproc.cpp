#include <QDebug>
#include <QString>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h> 	// sched_rr_get_interval(pid, &ts);
#include <libgen.h> // basename()
#include <unistd.h> // sysconf()  POSIX.1-2001
#include <sys/time.h>
#include "qjdproc.h"
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>

#define PROCDIR  "/proc" 		// hmmm

bool flag_SMPsim=false;
int 	flag_thread_ok=true;		    // we presume a kernel 2.6.x using NPTL
bool 	flag_devel= false ;
bool	flag_schedstat=false;
int pagesize;

// read() the number of bytes read is returned (zero indicates end  of  file)
// return the number of bytes read if ok, -1 if failed

//从fd中读取max字节到buf中去, return 0 : if error occurs.
inline int read_file(char *name, char *buf, int max)
{
    int fd = open(name, O_RDONLY);
    if(fd < 0) return -1;
    int r = read(fd, buf, max);
    close(fd);
    return r;
}

// return username from /etc/passwd
char *userName(int uid,int euid)
{
    char buff[128];
    struct passwd *pw = getpwuid(uid);
    if(!pw) {
        // dont have name !
        sprintf(buff, "%d", uid);
    } else
        strcpy(buff,pw->pw_name);

    if(uid!=euid)
        strcat(buff, euid == 0 ? "*" : "+");

    return strdup(buff);
}

// return group name (possibly numeric)
char *groupName(int gid)
{
    char *p;
    struct group *gr = getgrgid(gid);
    if(!gr) {
        p = (char *)malloc(11);
        sprintf(p, "%d", gid);
    } else
        p = strdup(gr->gr_name);
    //	s.append("*");
    return p;
}

char buffer_proc[1024*2]; // enough..maybe
//读取proc文件系统的方法,直接给定路径，文件名，读取大小
char * read_proc_file2(char *r_path,char *fname,int *size=NULL)
{
    static int max_size=0;
    char path[256];
    int	 r;

    sprintf(path,"%s/%s",r_path,fname); //将给定的文件名和路径赋给path

    if(strcmp(fname,"exe")==0)  //仍然意义不明，为什么和exe比较？exedomains文件？？？
    {
        if( (r=readlink(path, buffer_proc, sizeof(buffer_proc) - 1)) >= 0)
        {
            buffer_proc[r]=0; //safer
            return buffer_proc;
        }
        else
            return 0;
    }

    int fd = open(path, O_RDONLY);  //将path下的文件打开， readOnly
    if(fd < 0)
        return 0;
    r = read(fd, buffer_proc, sizeof(buffer_proc)-1); //return 0 , -1 , read_count,然后将读取的东西放到buffer_proc里去。
    if(r<0)
        return 0;

    if(max_size<r)
        max_size=r;

    if(size!=0)
        *size=r;

    buffer_proc[r]=0; //safer
    close(fd);

    return buffer_proc;
}

// new process created
Procinfo::Procinfo(Proc *system_proc,int process_id, int thread_id) : refcnt(1)
{
    proc=system_proc;

    if(thread_id<0)     //如果没有线程，则赋值的是进程id
    {
        pid=process_id;
        tgid=process_id; // thread group leader's id
    }
    else        //如果有线程ID，则pid取线程id
    {
        pid=thread_id;
        tgid=process_id; // thread group leader's id
    }

    ppid=0;   // no parent

    generation=-1;
    //    detail = 0;  //暂时注释

    /// per_cpu_times = 0; not yet

    size=0;
    resident=0;
    trs=0;
    drs=0;
    stack=0;
    share=0;
    mem=0;

    //tgid=0;
    pcpu=0;
    pmem=0;

    old_utime=0;	// this must be current utime !
    old_wcpu=0;

    command="noname";
    tty=0;
    nice=0;
    starttime=0;
    state='Z';
    cutime=utime=0;

    nthreads=0; /* number of threads */

    hashstr[0]=0;
    hashlen=0;
}

Procinfo::~Procinfo()
{}

// miscellaneous static initializations
void Proc::init_static()
{
    pagesize=sysconf(_SC_PAGESIZE); // same getpagesize()  in <unistd.h>
}


// using cache
int  Procinfo::hashcmp(char *sbuf)
{
    int statlen;

    statlen=strlen(sbuf);
    if(statlen>sizeof(hashstr))
    {
        // some user reported 265byte.
        printf("Qps BUG:  hashstr shortage statlen(%d) > hashstr(%ld)\n",statlen,sizeof(hashstr));
        abort();
    }
    else if(statlen==hashlen)
    {
        if(memcmp(hashstr,sbuf,statlen)==0)
        {
            pcpu=0;
            // 1. I am a sleeping process
            //printf("[%d] sleep process \n",pid);
            return 1;
        }
    }
    memcpy(hashstr,sbuf,statlen); // to back
    hashlen=statlen;
    return 0;
}

int mini_sscanf(const char *s,const char *fmt, ...);


int Procinfo::readproc()
{
//    qDebug()<<"3."<<old_utime;

    char cmdbuf[MAX_CMD_LEN];
    char path[64];
    char *sbuf;  // should be enough to acommodate /proc/PID/stat
    char *buf;
    int	i_tty;
    long stime, cstime;

    if(isThread())// flag_thread_ok, 是线程
    {
        sprintf(path,"/proc/%d/task/%d",tgid,pid);
    }
    else    //进程
        sprintf(path,"/proc/%d",pid);


    if(1)
    {
        //	Note: COMMAND , TGID, UID , COMMAND_LINE  never change !
        old_wcpu = wcpu = pcpu = 0.0;

        // read /proc/PID/status
        if((buf= read_proc_file2(path,"status")) ==0 ) return -1;

        if(mini_sscanf(buf,"Name: %S\n",cmdbuf)==0 ) return -1;
        else
        {
            command = cmdbuf;
        }

        if(mini_sscanf(buf, "Tgid: %d ", &tgid)==0) return -1;
        if(mini_sscanf(buf, "Uid: %d %d %d %d", &uid, &euid, &suid, &fsuid) !=4) return -1;
        if(mini_sscanf(buf, "Gid: %d %d %d %d", &gid, &egid, &sgid, &fsgid) !=4) return -1;

        proc->uidVector<<uid;
        username=userName(uid,euid);
        groupname=groupName(gid);
        proc->usernameVector<<username;
        char cmdline_cmd[256]; // !!!! some name .... larger
        //read /proc/pid/cmdline
        int size;
        cmdline_cmd[0]=0;

        // anyone can read [cmdline]
        if((buf= read_proc_file2(path,"cmdline", &size)) ==0 ) return -1;
        else {
            int  cmdlen=strlen(buf);

            if(cmdlen == 0)
            {
                // 1. kthread
                // printf("Qps:debug no_cmdline pid=%d\n",pid );
                cmdline = "";
            }
            // for non-ascii locale language
            // cmdline = codec->toUnicode(cmdbuf,strlen(cmdbuf));
            else
            {
                strcpy(cmdline_cmd,buf); // copy cmd without options

                //change 0x00,0xA to ' '
                for(int i = 0; i < size - 1; i++)  //OVERFLOW
                    if(buf[i]==0 or buf[i]==0x0A)
                        buf[i] = ' ';

                cmdline = buf;
            }
        }
        proc->cmdlineVector<<cmdline;
        // VERY COMPLEX CODE
        // because MAX_length of command is 15, so sometimes cmd_name truncated,
        // 1.check [exe] file ( only owner can read)
        // 2.check [cmdline] ( anyone can read )
        //
        if(command.size()==15)
        {
            if(true)
            {
                // use cmdline
                // ex. hald-addon-input
                // python /usr/lib/system-service-d
                char *p=strcasestr(cmdline_cmd,": "); // VALGRIND: uninit
                if(p!=0 and strncmp(cmdline_cmd,qPrintable(command),15)==0 )
                {
                    *p=0;
                }

                if(strlen(basename(cmdline_cmd))>15 and
                   strncmp(qPrintable(command),basename(cmdline_cmd),15)==0 )
                    command=basename(cmdline_cmd); // UNinit
                //command.append("^");
            }
        }

        if(isThread())	cmdline=command + " (thread)";
    }

    // read /proc/PID/stat
    if((sbuf= read_proc_file2(path,"stat")) ==0 ) return -1;
    //    qDebug()<<sbuf;   //将stat文件全部读出

    if(flag_schedstat==false)// if no change then return.    twice faster !
    {
        if(hashcmp(sbuf)) return 1;
    }

    /*
                Not all values from /proc/#/stat are interesting; the ones left out
                have been retained in comments to see where they should go, in case
                they are needed again.

                Notes :
                        1. man -S 5 proc
                        2. man -S 2 times
                        3. ppid can be changed when parent dead !
                        4. initial utime maybe 0, so %CPU field NotAnumber !!
                utime: user time
                stime: kernel mode tick
                cutime : The  number  of jiffies that this process's waited-for children have been scheduled in user mode.
                jiffies == tick
        */
    unsigned int guest_utime,cguest_utime;

#if 1

    /// 注意这里，黄金地带
    char *p,*p1;
    // in odd cases the name can contain spaces and '(' or ')' and numbers, so this makes
    // parsing more difficult. We scan for the outermost '(' ')' to find the name.
    p = strchr(sbuf, '(');
    p1 = strrchr(sbuf, ')');
    if (p == 0 || p1 == 0) return -1;
    p1++;
    // we can safely use sscanf() on the rest of the string
    sscanf(p1, " %c %d %d %d %d %d"
           " %lu %lu %lu %lu %lu "
           "%ld %ld %ld %ld %d %d %d %*s %lu %*s %*s %*s %*s %*s %*s %*s %*s "
           "%*s %*s %*s %*s %lu %*s %*s %*s %u %*s %*s %*s %u %u",
#else
           // some errors will occur !
           mini_sscanf(sbuf, "%d (%S) %c %d %d %d %d %d"
                       "%lu %lu %lu %lu %lu "
                       "%ld %ld %ld %ld %d %d %d %*s %lu %*s %*s %*s %*s %*s %*s %*s %*s "
                       "%*s %*s %*s %*s %lu %*s %*s %*s %u %*s %*s %*s %u %u",
                       &x_pid, &cmdbuf[0],
#endif
                       &state, &ppid, &pgrp, &session, &i_tty, &tpgid,
                       &flags, &minflt, &cminflt, &majflt, &cmajflt,
                       &utime, &stime , &cutime, &cstime, &priority, &nice,
                       &nthreads,&starttime, &wchan,&which_cpu,
                       &guest_utime,&cguest_utime);

    QString strName(state);
    proc->statVector<<strName;
    proc->niceVector<<nice;
    proc->whichcpuVector<<which_cpu;

    ///    qDebug()<<(long)time(NULL) - (long)proc->boot_time;  //系统启动时间
    starttime= proc->boot_time /* secs */ + (starttime/proc->clk_tick);     //不知数字如何换算至开机时间

    /// 计算程序运行时间
    long u=(long)time(NULL)-(long)starttime;    //神奇的一句话
    proc->originStarttimeVector<<u;
    QString runtime=QString::number(u)+"Sec";
    int a=0,b=0,c=0,d=0,e=0,f=0;
    if(u>=60 && u<3600)
    {
        c=u/60;
        d=u%60;
        runtime=QString::number(c)+"Min "+QString::number(d)+"Sec";
    }
    if(u>=3600 && u<86400)
    {
        a=u/3600;
        b=u%3600;
        if(b>=60)
        {
            c=b/60;
            d=b%60;
        }
        if(b<60)
        {
            d=b;
        }
        runtime=QString::number(a)+"Hour "+QString::number(c)+"Min "+QString::number(d)+"Sec";
    }
    if(u>86400)
    {
        a=u/86400;
        b=u%86400;
        if(b>=3600)
        {
            c=b/3600;
            d=b%3600;
            if(d>=60)
            {
                e=d/60;
                f=d%60;
            }
            if(d<60)
            {
                f=d;
            }
        }
        if(b<3600)
        {
            e=b/60;
            f=b%60;
        }
        runtime=QString::number(a)+"Day "+QString::number(c)+"Hour "+QString::number(e)+"Min "+QString::number(f)+"Sec";
    }
    proc->starttimeVector<<runtime;
    ///    qDebug()<<(long)time(NULL)-(long)starttime;      //进程启动时间
    tty = (dev_t)i_tty; // hmmm
    utime += stime;		// we make no user/system time distinction
    cutime += cstime;


    if(1)
    {
        int dcpu;
        dcpu = utime - old_utime; // user_time from proc
//        qDebug()<<"1."<<old_utime;

        if(proc->dt_total>0)	// move to Proc ??
        {
            pcpu = 100.0 * dcpu / proc->dt_total;
        }

        const float a = Procview::avg_factor;
        wcpu = a * old_wcpu + (1 - a) * pcpu;
    }
    proc->pcpuVector<<pcpu;
    proc->wcpuVector<<wcpu;
//        qDebug()<<pcpu<<wcpu;
    old_wcpu=wcpu;
    old_utime=utime;    // ****
//    qDebug()<<"2."<<old_utime;


    // read /proc/%PID/statm  - memory usage
    if((buf= read_proc_file2(path,"statm")) ==0 ) return -1; // kernel 2.2 ?
    sscanf(buf, "%lu %lu %lu %lu %lu %lu %lu",
           &size, &resident, &share, &trs, &lrs, &drs, &dt);
    size	*=pagesize/1024; // total memory in kByte
    resident*=pagesize/1024;
    share	*=pagesize/1024; // share
    //	trs		;	// text(code)
    //	lrs		;	// zero : lib, awlays zero in Kernel 2.6
    //	drs		;	// data: wrong in kernel 2.6
    //	dt		;	// zero : in Kernel 2.6
    mem= resident-share;            //内存使用量
    proc->originMemVector<<mem;
    pmem = 100.0 * mem / proc->mem_total;       //内存使用的百分比
    QString memT=QString::number(mem)+"K";
    int memTemp;
    memTemp=mem;
    if(mem>1024)
    {
        memTemp=mem/1024;
        memT=QString::number(memTemp)+"M";
    }
    proc->memVector<<memT;

    proc->pmemVector<<pmem;



    // read /proc/PID/status check !!
    if((buf= read_proc_file2(path,"status")) ==0 )  return -1;
    else
    {
        // slpavg : not supported in kernel 2.4; default value of -1
        if(mini_sscanf(buf, "SleepAVG:%d",&slpavg)==0) slpavg =-1;

        if(strstr(buf, "VmSize:"))
        {
            mini_sscanf(buf, "VmData: %d",&drs);	//data	in kByte
            mini_sscanf(buf, "VmStk: %d",&stack);	//stack	in kByte
            mini_sscanf(buf, "VmExe: %d",&trs);		//text
        }
        proc->slpavgVector<<slpavg;     //睡眠百分比
        proc->stackVector<<stack;
    }

    /// read file_io        /// /path/pid/io
    if((buf= read_proc_file2(path,"io")) !=0 )
    {
        mini_sscanf(buf, "read_bytes:%d",&io_read);
        mini_sscanf(buf, "write_bytes:%d",&io_write);

        io_read>>=10; //  divide by 1024
        io_write>>=10; //  divide by 1024
        //qDebug()<<io_read<<io_write;

        proc->originIoreadVector<<io_read;
        proc->originIowriteVector<<io_write;
        /// 将io使用情况转换为相应单位准备显示
        read=QString::number(io_read)+"K";
        write=QString::number(io_write)+"K";

        if(io_read>1024)
        {
            readTemp=io_read/1024;
            read=QString::number(readTemp)+"M";

        }
        if(io_write>1024)
        {
            writeTemp=io_write/1024;
            write=QString::number(writeTemp)+"M";
        }
        proc->ioreadVector<<read;
        proc->iowriteVector<<write;
    }

    /// /path/pid/wchan
    buf= read_proc_file2(path,"wchan");
    wchan_str=buf;
    proc->wchanVector<<wchan_str;       //读出wchan的信息，写入widget

    tms = -1;		    // ditto

    return 2; // return ok.
}

// Description:  read common information  for all processes
// return value
// 		-1 : too fast refresh !
int Proc::read_system()
{
    static bool first_time=true;
    char path[80];
    char buf[1024*8]; // for SMP

    char *p;
    int n;

    if(first_time)
    {
        /* check schedstat  */
        strcpy(path,"/proc/1/schedstat");  // some system doesn't have, centOS有
        if (!stat(path, (struct stat*)buf) )
            flag_schedstat = true;
        else
            flag_schedstat = false;

        strcpy(path, "/proc/stat");
        if((n = read_file(path, buf, sizeof(buf) - 1)) <= 0) return 0;
        buf[n] = '\0';
        p = strstr(buf, "btime");
        if(p==NULL)
        {
            // used
            printf("Qps: A bug occurs ! [boot_time] \n");
            //boot_time= current time
        }else
        {
            p+=6;
            sscanf(p, "%d", &boot_time);
        }

        int max_cpus=512;
        cpu_times_vec = new unsigned[CPUTIMES * max_cpus];
        old_cpu_times_vec = new unsigned[CPUTIMES * max_cpus];

        //init
        for(int cpu = 0; cpu < max_cpus ; cpu++)
            for(int i = 0; i < CPUTIMES; i++)
            {
            cpu_times(cpu, i)=0;
            old_cpu_times(cpu, i) =0;
        }
    }

    // read system status  /proc/stat
    strcpy(path, "/proc/stat");
    if((n = read_file(path, buf, sizeof(buf) - 1)) <= 0)
    {
        printf("Qps Error: /proc/stat can't be read ! check it and \n");
        abort();//	return 0;
    }
    buf[n] = '\0';

    // count (current) cpu of system
    //又来计算一遍CPU的数量
    //    char *p;
    p = strstr(buf, "cpu");
    num_cpus=0;
    while(p < buf + sizeof(buf) - 4 && strncmp(p, "cpu", 3) == 0)
    {
        num_cpus++;
        if(strncmp(p, "cpu0", 4) == 0)	Proc::num_cpus--;   //关键句，不要随意删除
        p = strchr(p, '\n');
        if(p)
            p++;
    }

    if(flag_devel and flag_SMPsim )
    {
        //num_cpus=64;
        int vals[]={2,4,8,16,32};
        int r=rand()%5;
        num_cpus=vals[r];
    }

    // Hotplugging Detection : save total_cpu
    if(Proc::num_cpus != Proc::old_num_cpus) {
        for(int i = 0; i < CPUTIMES; i++)
            cpu_times(num_cpus, i) = cpu_times(Proc::old_num_cpus, i);

        Proc::old_num_cpus=Proc::num_cpus;
    }


    // backup old values :  important*******
    for(int cpu = 0; cpu < Proc::num_cpus+1; cpu++)
    {
        for(int i = 0; i < CPUTIMES; i++)
            old_cpu_times(cpu, i) = cpu_times(cpu, i);
    }

    /*
                /proc/stat
                cpu#	user	nice	system	idle		iowait(2.6)	irq(2.6)	sft(2.6)	steal(2.6.11) 	guest(2.6.24)
                cpu0	3350 	9		535		160879		1929		105			326			5				1200

                Q1: kernel 2.4 cpu0 exist ?
                 A1: 2.6 still exist
        */

    // Total_cpu
//    int total_cpu=Proc::num_cpus;
    unsigned user,nice,system,idle,iowait, irq, sftirq, steal,guest, nflds;
    //将这些信息通通读进到buf中去，读一个nflds＋1
    nflds = sscanf(buf, "cpu %u %u %u %u %u %u %u %u %u", &user, &nice, &system, &idle, &iowait, &irq, &sftirq, &steal,&guest);
    //    qDebug()<<user<<nice<<system<<idle<<iowait<<irq<<sftirq<<steal<<guest;        //system use, not for each process
    if( nflds > 4 ) {
        // kernel 2.6.x
        system+=(irq+sftirq);
        idle+=iowait;
    }
    if( nflds == 9 ){
        system+=steal;
        system+=guest;
    }
    cpu_times(Proc::num_cpus, CPUTIME_USER)=user;           //赋值给cpu的相关参数
    cpu_times(Proc::num_cpus, CPUTIME_NICE)=nice;
    cpu_times(Proc::num_cpus, CPUTIME_SYSTEM)=system;
    cpu_times(Proc::num_cpus, CPUTIME_IDLE)=idle;

    // infobar uses this value, so this ga
    Proc::dt_used=user - old_cpu_times(Proc::num_cpus, CPUTIME_USER);
    Proc::dt_used+=system - old_cpu_times(Proc::num_cpus, CPUTIME_SYSTEM);
    Proc::dt_total= dt_used +
                    nice - old_cpu_times(Proc::num_cpus, CPUTIME_NICE)+
                    idle - old_cpu_times(Proc::num_cpus, CPUTIME_IDLE);

    if(first_time)
    {
        //	printf("\n==================== tooo fast =================================\n");
        //	printf("DEBUG:dt_total=%d dt_used=%d\n",Proc::dt_total,Proc::dt_used);
        //	return -1; // too early refresh again  !! 不在第一次的时候刷新的意思
    }
    if(Proc::dt_total==0)
    {
        //?????
        printf("Error: dt_total=0 , dt_used=%ld(ld) \n",Proc::dt_used,old_cpu_times(Proc::num_cpus,CPUTIME_IDLE));
        dt_total=500; //more tolerable?
        //abort(); // stdlib.h
    }

    //	void watchdog_syscpu(int );
    //	watchdog_syscpu((user-old_cpu_times(num_cpus,CPUTIME_USER))*100/dt_total); // test

    if(flag_devel and flag_SMPsim )     //这2个flag从未被赋真值，不会被执行
    {
        // for Developer only !!! ～～～～原来如此
        // printf("user%d nuce%d system%d idle%d\n",user,nice,system,idle);
        for(int cpu = 0; cpu < num_cpus ; cpu++) {
            //stdlib.h, int rand();
            if(dt_used!=0) cpu_times(cpu, CPUTIME_USER)=old_cpu_times(cpu, CPUTIME_USER ) + rand()%dt_used;
            else 	cpu_times(cpu, CPUTIME_USER)=0;
            cpu_times(cpu, CPUTIME_NICE)=nice;
            cpu_times(cpu, CPUTIME_SYSTEM)=system;
            cpu_times(cpu, CPUTIME_IDLE)=idle;

        }
    }
    else
    {
        // SMP(Multi-CPU)
        for(int cpu = 0; cpu < num_cpus ; cpu++)
        {
            char cpu_buf[10];
            sprintf(cpu_buf, "cpu%d", cpu);
            if((p = strstr(buf, cpu_buf)) != 0)
            {
                //将那些读进来再次进行转存
                nflds = sscanf(p, "%*s %u %u %u %u %u %u %u %u %u",
                               &cpu_times(cpu, CPUTIME_USER), &cpu_times(cpu, CPUTIME_NICE),
                               &cpu_times(cpu, CPUTIME_SYSTEM), &cpu_times(cpu, CPUTIME_IDLE),
                               &iowait, &irq, &sftirq,&steal,&guest);
                //cpu_times(cpu, CPUTIME_USER),cpu_times(cpu, CPUTIME_NICE),
                if( nflds > 4 )
                {
                    // kernel 2.6.x
                    cpu_times(cpu, CPUTIME_SYSTEM)+=(irq+sftirq);
                    cpu_times(cpu, CPUTIME_IDLE)+=iowait;
                }
                if( nflds == 9 )
                {
                    cpu_times(cpu, CPUTIME_SYSTEM)+=(steal+guest);
                }

                // 2.4.27-SMP bug

            }
            else
            {
                fprintf(stderr, "Qps: Error reading info for cpu%d (/proc/stat)\n", cpu);
                abort();
            }
        }
    }

    // read memory info
    strcpy(path, PROCDIR);
    strcat(path, "/meminfo");  //把/meminfo所指字符串添加到path结尾处(覆盖path结尾处的'\0')并添加'\0'
    if((n = read_file(path, buf, sizeof(buf) - 1)) <= 0) return 0;
    buf[n] = '\0';

    // Skip the old /meminfo cruft, making this work in post-2.1.42 kernels
    // as well.  (values are now in kB)
    //匹配，则给予赋值
    if( p = strstr(buf, "MemTotal:"))
        sscanf(p, "MemTotal: %d kB\n", &mem_total);
    if( (p = strstr(buf, "MemFree:")) != NULL )
        sscanf(p, "MemFree: %d kB\n", &mem_free);
    if( (p = strstr(buf, "Buffers:")) != NULL )
        sscanf(p, "Buffers: %d kB\n", &mem_buffers);
    if( (p = strstr(buf, "Cached:")) != NULL )
        sscanf(p, "Cached: %d kB\n", &mem_cached);

    p = strstr(buf, "SwapTotal:");
    sscanf(p, "SwapTotal: %d kB\nSwapFree: %d kB\n", &swap_total, &swap_free);

    first_time=false;   //第一次执行完毕
    return 0;
}

Proc::Proc()
{
    commonPostInit();
    Proc::init_static();
}
Proc::~Proc()
{}

// COMMON for LINUX,SOLARIS
// Polling /proc/PID/*
// 在proc_common的refresh中刷新
void Proc::read_proc_all()
{
    DIR *d = opendir("/proc");
    struct dirent *e;

    //    int loop=0;
    while((e = readdir(d)) != 0)
    {
        if(e->d_name[0] >= '0' and e->d_name[0] <= '9')
        {
            int pid;
            Procinfo *pi=NULL;
            pid=atoi(e->d_name);
            pidVector<<pid;         //取出所有pid，自定义
            pi=procs.value(pid,NULL);

            if (pi==NULL)   // new process
            {
                pi = new Procinfo(this,pid);
                procs.insert(pid,pi);
            }
            int ret=pi->readproc();     //CALL readproc();
            if(ret>0)
            {
                pi->generation = current_gen; // this process is alive
                cmdVector<<pi->command;
            }
        }
    }
    closedir(d);
}

bool Procinfo::isThread()
{
    return pid!=tgid; // how to check
}

/* ========================  Procview ============================  */
float Procview::avg_factor = 1.0;

Procview::Procview()
{
    enable=true;
    refresh(); // before reading Watchdog list
}

void Proc::commonPostInit()
{
    dt_total = 0; // diff system tick
    dt_used = 0; // for infobar
    Proc::num_cpus = 0;
    Proc::old_num_cpus = 0;
    Proc::mem_total = 0;
    Proc::mem_free = 0;
    Proc::mem_buffers = 0;
    Proc::mem_cached = 0;
    Proc::swap_total = 0;
    Proc::swap_free = 0;
    Proc::cpu_times_vec = 0;	// array.
    Proc::old_cpu_times_vec = 0;
    Proc::boot_time = 0;
    Proc::clk_tick = 100; //for most system

    current_gen=0; // !
    clk_tick=sysconf(_SC_CLK_TCK); //The  number  of  clock ticks per second.
}

// Description: update the process list      BottleNeck 1.5%    有瓶颈
// 		read /proc/*
// 		called by Procview::refresh(), every UPDATE .
void Proc::refresh()
{
    current_gen++;

    //init
    if(Proc::read_system()<0) return; // **** should be every refresh !!

    read_proc_all();

}

// read new process info
void Procview::refresh()
{
    if(enable)
    {
        Proc::refresh(); 	// read "/proc/*", then update the process list
    }
}
