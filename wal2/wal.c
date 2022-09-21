#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

// tables and API of log
typedef struct WriteAheadLog WriteAheadLog;

struct QQmt_data_table;
struct WriteLog_args;
struct WriteAheadLog;
struct QQmt_data_tableList;
struct LogHeader;
struct LogTable_iterator;


enum ins_type
{
    ins_modify = 0,
    ins_delete = 1,
    ins_add = 2
};


/*
            struct QQmt_data_table

| pos_write | byte0 | byte1 | byte2 | byte3 |  byte4 |******************* |
            |   bytes have been written        byte4 |unwritten bytes     |

  now, pos_write is 5
*/

struct QQmt_data_table
{
    uint32_t pos_write;
};

// a list of tables
struct LogTableList
{
    struct QQmt_data_table *table;
    struct LogTableList *next;
};

// tables and API of log
struct WriteAheadLog
{
    struct LogTableList *list_head, *list_tail; // tables are maintained as a list
    
    uint32_t LogTable_size; // size of a LogTable  

    uint32_t _LogHeader_size;
    
    int (*AppendRecord)(struct LogTable_iterator *itor, struct WriteLog_args *arg);
};

struct LogHeader
{
    int type;
    size_t key_size, old_val_size, new_val_size ;

};

struct WriteLog_args
{
    struct LogHeader header;
    const void const *key, *old_val, *new_val;
};

struct LogTable_iterator
{
    struct WriteAheadLog *wal;
    struct LogTableList *node;
    int32_t cur_offset;
};



/*
                
                   WriteAheadLog 
list_head                             list_tail
   |                                      |
   v                   LogTableList       v
|next  |--> |next  |--> |next  |--> |next  |------> NULL
|table |    |table |    |table |    |table |
  |             |            |             | 
  |             |            |             |
  v             v            v             v
table         table        table         table


*/


/*
                                                                    LogTable
| pos_write |  type | key_size | old_val_size | new_val_size | key0 | old_val0| new_val0 | type | key_size | old_val_size | new_val_size | key1 | old_val1| new_val1 | ...
            |            0th log                                                         |            1th log                                                        |


a LogHeader is stored contiguously


*/


static int WrtieAheadLog_append_record(struct LogTable_iterator *itor, struct WriteLog_args *arg);

static WriteAheadLog* new_WriteAheadLog(uint32_t LogTable_size);

static struct LogTableList* AddLogTable(struct WriteAheadLog *wal); 
static struct WriteLog_args SetWriteLog_args(int type, int key_size, int old_val_size, int new_val_size, const void *key, const void *old_val, const void *new_val);

static struct LogTableList *new_LogTableList_node(struct WriteAheadLog *wal);

static struct QQmt_data_table *new_LogTable(struct WriteAheadLog *wal) ;
static int init_LogTable(struct QQmt_data_table *table);





static int32_t LogTable_iterator_read(struct LogTable_iterator *const itor, void *dst, int32_t read_size);
static int32_t LogTable_iterator_write(struct LogTable_iterator *const itor,  const void *__restrict__ src, int32_t offset, int32_t write_size);
static int32_t LogTable_iterator_append(struct LogTable_iterator *const itor,  const void *__restrict__ src, int32_t write_size);
static int32_t LogTable_iterator_forward(struct LogTable_iterator *const itor, uint32_t move_distance);
struct LogTable_iterator *new_Log_iterator(struct WriteAheadLog *wal, int offset);
int init_Log_iterator(struct LogTable_iterator *itor, int offset);
int LogTable_iterator_destroy(struct LogTable_iterator *itor);

static int32_t QQmt_data_table_read(struct QQmt_data_table *table, void *dst, int start, int32_t read_size);
static int32_t QQmt_data_table_write(struct QQmt_data_table *table,  const void *__restrict__ src, int32_t start, int32_t write_size);
static int32_t QQmt_data_table_append(struct QQmt_data_table *table,  const void *__restrict__ src, int32_t write_size);
static int32_t QQmt_data_table_forward(struct QQmt_data_table *table, int32_t move_distance);

static int32_t LogTable_iterator_forward(struct LogTable_iterator *const itor, uint32_t byte_distance)
{
    uint32_t total_move_distance = 0;
    
    while(byte_distance > 0)
    {
        const int32_t pos = sizeof(itor->node->table->pos_write) + itor->cur_offset;
        const int32_t max_offset = itor->wal->LogTable_size;
        
        uint32_t dist = __min(byte_distance, max_offset - pos);

        if (dist + pos > itor->node->table->pos_write + sizeof(itor->node->table->pos_write))
            QQmt_data_table_forward(itor->node->table, (dist + pos) - (itor->node->table->pos_write + sizeof(itor->node->table->pos_write)));

        total_move_distance += dist ;
        
        byte_distance -=  dist ;
        
        itor->cur_offset += dist ;
       
        if (itor->node->next != NULL && itor->cur_offset + sizeof(itor->node->table->pos_write) == itor->wal->LogTable_size) // go to next table ?
        {
            itor->node = itor->node->next;
            itor->cur_offset = 0;
        }    

        if (itor->node->next == NULL) // no pages 
            break;
    }

    return total_move_distance;
}



static int32_t LogTable_iterator_read(struct LogTable_iterator *const itor, void *dst, int32_t read_size)
{
    int32_t total_read_size = 0;

    while(read_size > 0)
    {
        const int32_t pos = sizeof(itor->node->table->pos_write) + itor->cur_offset;
        const int32_t max_offset = __min(itor->node->table->pos_write + sizeof(itor->node->table->pos_write), itor->wal->LogTable_size);
        const int32_t dist =  __min(read_size, max_offset - pos);

        QQmt_data_table_read(itor->node->table, (void*)(((char*)(dst)) + total_read_size),  itor->cur_offset, dist);

        total_read_size += dist ;
        
        read_size -=  dist ;
        
        itor->cur_offset += dist ;
       
        if (itor->node->next != NULL && itor->cur_offset + sizeof(itor->node->table->pos_write) == itor->wal->LogTable_size) // go to next table ?
        {
            itor->node = itor->node->next;
            itor->cur_offset = 0;
        }    

        if (itor->node->next == NULL && itor->cur_offset == itor->node->table->pos_write) // no data could be read, stop reading
            break;
    }

    return total_read_size;
}

static int32_t LogTable_iterator_write(struct LogTable_iterator *const itor,  const void *__restrict__ src, int32_t offset, int32_t write_size)
{
    LogTable_iterator_forward(itor, offset);

    int32_t total_write_size = 0;

    while(write_size > 0)
    {
        const int32_t pos = sizeof(itor->node->table->pos_write) + itor->cur_offset;
        const int32_t max_offset = __min(itor->node->table->pos_write + sizeof(itor->node->table->pos_write) + write_size, itor->wal->LogTable_size);
        const int32_t dist =  __min(write_size, max_offset - pos);
        
        QQmt_data_table_write(itor->node->table, (void*)(((char*)(src)) + total_write_size),  itor->cur_offset, dist);
        
        total_write_size += dist ;
        
        write_size -=  dist ;
        
        itor->cur_offset += dist ;
        

        if (itor->node->next == NULL && sizeof(itor->node->table->pos_write) + itor->cur_offset == itor->wal->LogTable_size) // no page, add a page
        {
            AddLogTable(itor->wal);
            itor->node = itor->node->next;
            itor->cur_offset = 0;
        }
            
    }

    return total_write_size;
}


static int32_t LogTable_iterator_append(struct LogTable_iterator *const itor,  const void *__restrict__ src, int32_t write_size)
{

    itor->node = itor->wal->list_tail;
    
    itor->cur_offset = itor->node->table->pos_write;
    
    return LogTable_iterator_write(itor, src, 0, write_size);
}


struct LogTable_iterator *new_Log_iterator(struct WriteAheadLog *wal, int offset)
{
    struct LogTable_iterator *itor = malloc(sizeof(struct LogTable_iterator));
    itor->wal = wal;
    int flag = init_Log_iterator(itor, offset);
 ;
    if (flag == 0)
    {
        free(itor);
        itor = NULL;
    }

    return itor;
}




int init_Log_iterator(struct LogTable_iterator *itor, int offset)
{
    if (offset < 0)
        return 0;

    itor->node = itor->wal->list_head;

    itor->cur_offset = 0;
    
    LogTable_iterator_forward(itor, offset);
    return 1;

}

int LogTable_iterator_destroy(struct LogTable_iterator *itor)
{
    free(itor);
    return 1 ;
}





// unsafe read because it doesn't check memory boundary
static int32_t QQmt_data_table_read(struct QQmt_data_table *table, void *__restrict__ dst, int32_t start, int32_t read_size)
{
    void *src = (void*)(((char*)table) + sizeof(table->pos_write) + start);
    memcpy(dst, src, read_size);
    
    return 1;
}

// unsafe write because it doesn't check memory boundary
static int32_t QQmt_data_table_write(struct QQmt_data_table *table,  const void *__restrict__ src, int32_t start, int32_t write_size)
{
    void *dst = (void*)(((char*)table) + sizeof(table->pos_write) + start);
    memcpy(dst, src, write_size);
    
    table->pos_write = __max(start + write_size, table->pos_write);
    return 1;
}

// unsafe write because it doesn't check memory boundary
// append data_size bytes to end of table
static int32_t QQmt_data_table_append(struct QQmt_data_table *table,  const void *__restrict__ src, int32_t write_size)
{
    void *dst = (void*)(((char*)table) + sizeof(table->pos_write) + table->pos_write);
    memcpy(dst, src, write_size);
    table->pos_write += write_size;
    return 1;
}

//move forward pos_write 'move_distance' bytes
static int32_t QQmt_data_table_forward(struct QQmt_data_table *table, int32_t move_distance)
{
    table->pos_write += move_distance;
    return 1;
}


static struct WriteLog_args SetWriteLog_args(int type, int key_size, int old_val_size, int new_val_size, const void *key, const void *old_val, const void *new_val)
{
    struct WriteLog_args ret = {.header.type = type, .header.key_size = key_size, .header.old_val_size = old_val_size, .header.new_val_size = new_val_size,
                                .key = key, .old_val = old_val, .new_val = new_val};

    return ret;
}

static struct QQmt_data_table *new_LogTable(struct WriteAheadLog *wal) 
{
    struct QQmt_data_table *ret = malloc(wal->LogTable_size);

    if (ret == NULL)
    {
        printf("no memory\n");
        abort();
    }

    init_LogTable(ret);

    return ret;
}

static int init_LogTable(struct QQmt_data_table *table)
{
    if (!table)
        return 0;

    table->pos_write = 0;

    return 1;
}


static struct LogTableList *new_LogTableList_node(struct WriteAheadLog *wal)
{
    struct LogTableList *list_node = malloc(sizeof(struct LogTableList));

    list_node->table = new_LogTable(wal);

    list_node->next = NULL;

    return list_node;
}


// add table to a TableList of wal which is a non-empty list
static struct LogTableList* AddLogTable(struct WriteAheadLog *wal)
{   
    struct LogTableList *list_node = new_LogTableList_node(wal);

    wal->list_tail->next = list_node;

    wal->list_tail = list_node;
    //printf("add node %d\n",list_node);

    return list_node;
}


static WriteAheadLog* new_WriteAheadLog(uint32_t LogTable_size)
{
    if (LogTable_size < 16)
    {
        printf("error table should be greater than 16\n");
        return NULL;
    }

    WriteAheadLog *wal = (WriteAheadLog*)malloc(sizeof(WriteAheadLog));

    wal->LogTable_size = LogTable_size;
    
    wal->_LogHeader_size = sizeof(struct LogHeader);

    wal->AppendRecord = WrtieAheadLog_append_record;

    wal->list_head = wal->list_tail = new_LogTableList_node(wal);

    return wal;
}






static int WrtieAheadLog_append_record(struct LogTable_iterator *itor, struct WriteLog_args *arg)
{
    LogTable_iterator_append(itor, (void*)&arg->header, sizeof(arg->header));

    LogTable_iterator_append(itor, (void*)arg->key, arg->header.key_size);

    LogTable_iterator_append(itor, (void*)arg->old_val, arg->header.old_val_size);

    LogTable_iterator_append(itor, (void*)arg->new_val, arg->header.new_val_size);


    return 0;
}


static int Get_next_ith_Record(struct LogTable_iterator *itor, int ith)
{
    if (ith < 0)
    {
        printf("wrong ith = %d",ith);
        return 0;
    }
    while(ith)
    {
        struct LogHeader header;
        LogTable_iterator_read(itor, &header, sizeof(header));
        LogTable_iterator_forward(itor, header.key_size + header.old_val_size + header.new_val_size);
        ith -= 1;
    }
    
    return 1;
}


int main()
{
   
    int a = ins_add;
    a -= 1+3;
    printf("%d\n",((intptr_t)1034) +2);

    WriteAheadLog *wal = new_WriteAheadLog(4096*2);
     
    struct LogTable_iterator *itor = new_Log_iterator(wal, 0);
 
    int32_t k = 20;
    int64_t old_val = 10, new_val = 1555555;
    struct WriteLog_args log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
    wal->AppendRecord(itor, &log_arg);
    
    k = 8;
    old_val = 103;
    new_val = 99933;
    log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
    wal->AppendRecord(itor, &log_arg);

    k = 9;
    old_val = 1;
    new_val = 2;
    log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
    wal->AppendRecord(itor, &log_arg);

    k = 10;
    old_val = 11;
    new_val = 12;
    log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
    wal->AppendRecord(itor, &log_arg);

    k = 11;
    old_val = 50;
    new_val = 6;
    log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
    wal->AppendRecord(itor, &log_arg);


    //LogTable_iterator_destroy(itor);
    init_Log_iterator(itor, 0);

    struct LogHeader lh;
    int32_t tk;   
    int64_t tov, tnv;

    //Get_next_ith_Record(itor, 2);

    Get_next_ith_Record(itor, 4);
    LogTable_iterator_read(itor, &lh, sizeof(lh));
    LogTable_iterator_read(itor, &tk, 4);
    LogTable_iterator_read(itor, &tov, 8);
    LogTable_iterator_read(itor, &tnv, 8);
    printf("{%d %d %d %d} %d %d %d\n",lh.type, lh.key_size, lh.old_val_size, lh.new_val_size, tk, tov, tnv);
    init_Log_iterator(itor, 0);
    int i = 0;
    uint64_t st, ed;
    int sum = 0;
    //wal->AppendRecord(itor, &log_arg);
    for(; i < 10000000 ; i++)
    {
        //k = i + 12;
        //log_arg = SetWriteLog_args(ins_modify, sizeof(int32_t), sizeof(int64_t), sizeof(int64_t), (void*)&k, (void*)&old_val, (void*)&new_val);
        st = clock();
        wal->AppendRecord(itor, &log_arg);
        ed = clock();
        sum += ed-st;
    }
    printf("%d\n",sum);
    init_Log_iterator(itor, 0);
    LogTable_iterator_forward(itor, 52*15550);
    //Get_next_ith_Record(itor, 100);
    LogTable_iterator_read(itor, &lh, sizeof(lh));
    LogTable_iterator_read(itor, &tk, 4);
    LogTable_iterator_read(itor, &tov, 8);
    LogTable_iterator_read(itor, &tnv, 8);
    printf("{%d %d %d %d} %d %d %d\n",lh.type, lh.key_size, lh.old_val_size, lh.new_val_size, tk, tov, tnv);



    




}
