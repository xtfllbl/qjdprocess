#ifndef PROC_H
#define PROC_H

#include "config.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDir>


#define CPU_TIMES(cpu, kind) cpu_times_vec[cpu * CPUTIMES + kind]
class Proc;

class Procinfo	// Process Infomation
{
public:
    Procinfo(Proc *system_proc,int pid,int thread_id=-1);
    ~Procinfo();
    Proc	*proc;

    int readproc();
    bool isThread();

    int pid;

    char 	hashstr[128*3];	// cache
    int		hashlen;
    int 	hashcmp(char *str);

    QString command;	//COMMAND
    QString cmdline; 	//COMMAND_LINE
    QString username;
    QString groupname;
    QString read;
    QString write;
    int readTemp;
    int writeTemp;

    int session;

    int uid, euid;
    int gid, egid;

    char state;
    int ppid;			// Parent's PID
    int pgrp;
    dev_t tty;			// tty major:minor device

    int nthreads;		// number of threads : LWP(Solaris), task(Linux)
    int tgid;			// thread leader's id

    double tms;		// slice time
    int slpavg;
    unsigned long affcpu;

    int suid, fsuid;
    int sgid, fsgid;
    int tpgid;

    unsigned long cminflt;
    unsigned long cmajflt;

//    unsigned long	io_read;		// byte, testing
//    unsigned long	io_write;		// testing
     long	io_read;		// byte, testing
     long	io_write;		// testing
    unsigned long flags;	//?
    unsigned long minflt;
    unsigned long majflt;

    long utime;
    long old_utime;		// initial value = -1 ;
    long cutime;
    int priority;
    int nice;
    unsigned long starttime;// start time since run in epoch? Linux : jiffies since boot , solaris
    unsigned long wchan;
    QString wchan_str;

    // Memory
    unsigned long mem;		// user Memory define
    unsigned long size;		// SIZE: total memory (K)
    unsigned long resident;	// RSS: pages in resident set (non-swapped) (K)
    unsigned long share;	// shared memory pages (mmaped) (K)
    unsigned long trs;      // text resident set size (K)
    unsigned long lrs;		// shared-lib resident set size (K)
    unsigned long drs;		// data resident set size (K)
    unsigned long dt;		// dirty pages (number of pages, not K), obsolute in Kernel 2.6
    unsigned long stack;	// stack size (K)

    // Linux: the cpu used most of the time of the process
    int which_cpu;

    // computed %cpu and %mem since last update
    float wcpu, old_wcpu;		// %WCPUwheight cpu
    float pcpu;					// %CPU: percent cpu after last update
    float pmem;					// %MEM

    unsigned int generation;	// timestamp

    static const int MAX_CMD_LEN = 512;

    char refcnt;
};

typedef QHash<int,Procinfo*> Proclist;


class Proc
{
public:
    Proc();
    ~Proc();
    void commonPostInit();	// COMMON
    void read_proc_all(); // test
    void refresh();

    static void init_static();
    int read_system();

    Proclist 		procs;	// processes indexed by pid

    /// 内存占用太大，自动关闭
    bool memWarning;

    /// 自定义的用于显示的容器
    QList<int> pidVector;                 //pid
    QList<QString> cmdVector;        //进程名
    QList<QString> statVector;        // 状态
    QList<int> niceVector;
    QList<QString> starttimeVector;
    QList<QString> wchanVector;
    QList<int> whichcpuVector;
    QList<QString> memVector;       //内存使用量
    QList<float> pmemVector;                    //内存使用百分比
    QList<int> slpavgVector;       //睡眠百分比
    QList<unsigned long> stackVector;       //栈空间
    QList<QString> ioreadVector;
    QList<QString> iowriteVector;
    QList<int> pcpuVector;                    //cpu使用百分比
    QList<float> wcpuVector;                    //cpu使用30s内百分比
    QList<QString> cmdlineVector;
    QList<int> uidVector;
    QList<QString> usernameVector;

    /// 原始数据容器
    QList<long> originStarttimeVector;
    QList<long> originMemVector;       //内存使用量
    QList<long> originIoreadVector;
    QList<long> originIowriteVector;

    // class
    int 	num_cpus;			// current number of CPUs
    int		old_num_cpus; 		// previous number of CPUs

    long 	dt_total;				//
    long 	dt_used;				//  cpu used time in clktick

    unsigned int clk_tick;		// the  number  of  clock ticks per second.
    unsigned int boot_time;		// boot time in seconds since the Epoch

    int mem_total, mem_free;		// (Kb)
    int swap_total, swap_free;		// in kB

    int mem_buffers, mem_cached; // Linux

    // the following are pointers to matrices indexed by kind (above) and cpu
    unsigned *cpu_times_vec;
    unsigned *old_cpu_times_vec;

    // accessors for (old_)cpu_times_vec
    unsigned &cpu_times(int cpu, int kind)
    { return cpu_times_vec[cpu * CPUTIMES + kind]; }
    unsigned &old_cpu_times(int cpu, int kind)
    { return old_cpu_times_vec[cpu * CPUTIMES + kind]; }

    enum { CPUTIME_USER,
           CPUTIME_NICE,
           CPUTIME_SYSTEM,
           CPUTIME_IDLE,
           CPUTIMES };

    unsigned int 	current_gen;
};


class Procview : public Proc
{
public:
    Procview();
    void refresh();
    bool enable;		// tmp
    static float avg_factor;		// exponential factor for averaging
};

#endif	// PROC_H
