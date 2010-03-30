#ifndef PROC_H
#define PROC_H

#include "config.h"
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QDir>

class Proc
{
public:
    Proc();
    ~Proc();
    void read_proc_all(); // test
    void refresh();
    int read_system();

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

    /// 原始数据容器，用来排序
    QList<long> originStarttimeVector;
    QList<long> originMemVector;       //内存使用量
    QList<long> originIoreadVector;
    QList<long> originIowriteVector;

    // class
    int 	num_cpus;			// current number of CPUs

    unsigned int clk_tick;		// the  number  of  clock ticks per second.
    unsigned int boot_time;		// boot time in seconds since the Epoch

    int mem_total, mem_free;		// (Kb)
    int swap_total, swap_free;		// in kB

    int mem_buffers, mem_cached; // Linux
};


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
    int	hashlen;
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
    QString strName;

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

    static const int MAX_CMD_LEN = 512;

    char refcnt;
};


#endif	// PROC_H
