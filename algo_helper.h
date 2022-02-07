#ifndef ALGO_HELPER_H_INCLUDED
#define ALGO_HELPER_H_INCLUDED
typedef struct AB_internel_node AB_internel_node ;
typedef struct KV_pair KV_pair;

static inline void ShifChildrenBackward(AB_Tree *self, AB_internel_node *node, int pos, size_t child_size);
static inline void ShifKeysBackward(AB_Tree *self, AB_internel_node *node, int pos);

static inline void ShiftKeysForward(AB_Tree *self, AB_internel_node *node, int dst);
static inline void ShiftChildrenForward(AB_Tree *self, AB_internel_node *node, int dst, size_t child_size);

static inline void *Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith);
static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith);

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node);
static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key);

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);
static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos);

static inline void* Ptr_ith_Key(AB_Tree *self, AB_internel_node *node, int ith)
{
    return (void*) ((char*)node + sizeof(AB_internel_node) + self->key_size * ith);
}

static inline int GetKeyRank(AB_Tree *self, AB_internel_node *node, void *key)
{
    #ifndef BIN_SEARCH
    int i = 0 ;
    for(; i < node->KeyCount ; i++)
    {
        int cmp = self->KeyComp(key, Ptr_ith_Key(self, node, i));
        if (cmp == -1)
            break;

        if (cmp == 0)
        {
            self->KeyIsFound = 1;
            break;
        }
    }
    return i ;
    #else
    int ret = node->KeyCount, l = 0, r = node->KeyCount -1;
    while(l <= r)
    {
        int cmp = self->KeyComp(key, Ptr_ith_Key(self, node, (l+r)>>1));
        if (cmp == 0)
        {
            self->KeyIsFound = 1;
            ret = (l+r)>>1;
            break;
        }
        else if (cmp == 1)
        {
            l = ((l+r)>>1)+1;
        }
        else
        {
            ret = (l+r)>>1;
            r = ((l+r)>>1)-1;
        }
    }

    return ret;
    #endif

}

static inline int IsFullNode(AB_Tree *self, AB_internel_node *node)
{
    return node->KeyCount == self->b-1;
}

static inline void *Ptr_ith_Child(AB_Tree *self, AB_internel_node *node, int ith)
{
    size_t value_offset = sizeof(AB_internel_node)+ (self->b-1) * self->key_size ;
    if (node->IsBottom == 0)
        return (void*) ((char*)node + value_offset + sizeof(AB_internel_node*)* ith);
    else
        return (void*) ((char*)node + value_offset + self->value_size * ith);
}




static inline void ShifKeysBackward(AB_Tree *self, AB_internel_node *node, int pos)
{
    memcpy(Ptr_ith_Key(self, node, pos+1),
           Ptr_ith_Key(self, node, pos),
           self->key_size * (node->KeyCount-pos));
}

static inline void ShifChildrenBackward(AB_Tree *self, AB_internel_node *node, int pos, size_t child_size)
{
    memcpy(Ptr_ith_Child(self, node, pos+1),
           Ptr_ith_Child(self, node, pos),
           child_size * (node->KeyCount-pos+1));
}


static inline void ShiftKeysForward(AB_Tree *self, AB_internel_node *node, int dst)
{
    memcpy(Ptr_ith_Key(self, node, dst),
           Ptr_ith_Key(self, node, dst+1),
           self->key_size * (node->KeyCount-dst-1));
}

static inline void ShiftChildrenForward(AB_Tree *self, AB_internel_node *node, int dst, size_t child_size)
{
    memcpy(Ptr_ith_Child(self, node, dst),
           Ptr_ith_Child(self, node, dst+1),
           child_size * (node->KeyCount-dst));
}

static AB_internel_node *SplitNode(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
{
    size_t child_size = sizeof(AB_internel_node *);

    if (cur->IsBottom)
        child_size = self->value_size ;

    AB_internel_node *new_node = MakeNode(self, child_size, cur->IsBottom);
    cur->KeyCount = (self->b)/2;
    new_node->KeyCount = self->b - cur->KeyCount;

    if (new_key_pos < (self->b)/2) //new key will reside in old node
    {
        memcpy(Ptr_ith_Key(self, new_node, 0),
               Ptr_ith_Key(self, cur, cur->KeyCount-1),
               self->key_size * new_node->KeyCount);
        memcpy(Ptr_ith_Child(self, new_node, 0),
               Ptr_ith_Child(self, cur, cur->KeyCount-1),
               child_size * (new_node->KeyCount + 1));

        ShifKeysBackward(self, cur, new_key_pos);
        ShifChildrenBackward(self, cur, new_key_pos, child_size);

        memcpy(Ptr_ith_Key(self, cur, new_key_pos),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Child(self, cur, new_key_pos),
               kv.value,
               child_size);
    }
    else //new key will reside in new node
    {
        memcpy(Ptr_ith_Key(self, new_node, 0),
                Ptr_ith_Key(self, cur, cur->KeyCount),
                self->key_size * (new_key_pos - cur->KeyCount));
        memcpy(Ptr_ith_Key(self, new_node, new_key_pos-cur->KeyCount),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Key(self, new_node, new_key_pos - cur->KeyCount + 1),
                Ptr_ith_Key(self, cur, new_key_pos),
                self->key_size * (cur->KeyCount + new_node->KeyCount - new_key_pos - 1));


        memcpy(Ptr_ith_Child(self, new_node, 0),
                Ptr_ith_Child(self, cur, cur->KeyCount),
                child_size *(new_key_pos - cur->KeyCount));
        memcpy(Ptr_ith_Child(self, new_node, new_key_pos-cur->KeyCount),
               kv.value,
               child_size);
        memcpy(Ptr_ith_Child(self, new_node, new_key_pos - cur->KeyCount + 1),
                Ptr_ith_Child(self, cur, new_key_pos),
                child_size * (cur->KeyCount + new_node->KeyCount - new_key_pos));
    }

    return new_node ;
}

static AB_internel_node *AddData(AB_Tree *self, AB_internel_node *cur, KV_pair kv, int new_key_pos)
{
    size_t child_size = sizeof(AB_internel_node *);

    if (cur->IsBottom)
        child_size = self->value_size ;

    if (IsFullNode(self, cur))
    {

        AB_internel_node *new_node = SplitNode(self, cur, kv, new_key_pos);

        return new_node;

    }
    else
    {
        ShifKeysBackward(self, cur, new_key_pos);
        ShifChildrenBackward(self, cur, new_key_pos, child_size);

        memcpy(Ptr_ith_Key(self, cur, new_key_pos),
               kv.key,
               self->key_size);
        memcpy(Ptr_ith_Child(self, cur, new_key_pos),
               kv.value,
               child_size);
        cur->KeyCount += 1;
        return NULL;
    }
}
#endif // ALGO_HELPER_H_INCLUDED
