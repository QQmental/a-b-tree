#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
static int NodeSize = 256;

typedef struct ab_tree_node_header ab_tree_node_header;
typedef struct ab_tree_node ab_tree_node;
typedef struct tree_node_properties tree_node_properties;
typedef struct bst_node bst_node;
typedef struct space_head space_head;
typedef struct KV_pair KV_pair;

typedef uint16_t keyspace_index;
typedef uint16_t childspace_index;

typedef struct KeyCompareResult KeyCompareResult;


struct tree_node_properties
{
    size_t key_size, child_size;
    int MaxChildCount;
};

struct bst_node
{
    unsigned int weight : 10; // weight of subtrees plus 1
    unsigned int Is_left_ptr_to_leaf : 1;
    unsigned int Is_right_ptr_to_leaf : 1;
    uint16_t LeftPtr : 10;
    uint16_t RightPtr : 10;
};


struct KeyCompareResult
{
    keyspace_index idx_key;
    uint16_t branch_child; // 0: left, 1: right
};


struct ab_tree_node
{
    char *_node_space;

};

struct ab_tree_node_header
{
    unsigned int IsBottom : 1;
    unsigned int KeyCount : 16;
    unsigned int root_offset : 10;
    keyspace_index FreeKeyListHead : 10;
    childspace_index FreeChildListHead : 10;
};

struct space_head
{
    union
    {
        bst_node node;
        uint16_t free_space_link;
    };
    
};


struct KV_pair
{
    void *key, *val;
};

ab_tree_node *SplitNode(tree_node_properties *node_properties, ab_tree_node *node, int split_idx);
void AddKeyChild(const tree_node_properties *node_properties, const ab_tree_node *node, KV_pair kv);
int SeachKey(const tree_node_properties *node_properties, const ab_tree_node *node, const void *key, KeyCompareResult SearchPath[128], int *SearchPathLength);
void *MakeNode(tree_node_properties *src, size_t node_size, int IsBottom);

void UpdateWeight_to_Root(const tree_node_properties *node_properties, const ab_tree_node *node, KeyCompareResult SearchPath[128], int SearchPathLength); 
keyspace_index GetPredcessor(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index src) ; // get internal predcessor

static inline void UpdateWeight(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key); // update weight and rotate if necessary
static inline keyspace_index RightRotateUp(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key);
static inline keyspace_index LeftRotateUp(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key);
static inline int NULL_PTR(const tree_node_properties *node_properties);
static inline int Is_full_node(const tree_node_properties *node_properties, const ab_tree_node *node);
static inline childspace_index GetFreeChildSpace(const tree_node_properties *node_properties, const ab_tree_node *node);
static inline keyspace_index GetFreeKeySpace(const tree_node_properties *node_properties, const ab_tree_node *node);
static inline  void* ptr_to_ith_childspace(const tree_node_properties *node_properties, const ab_tree_node *node, childspace_index ith);
static inline void* ptr_to_ith_key(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index ith);
static inline void* ptr_to_ith_keyspace(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index ith);
/*
ab_tree_node layout

| ab_tree_node_header | (bst_node, key0) | (bst_node, key1) | (bst_node, key2) | .... | (bst_node, key_MaxChildCount-1) |    (child0)      | (child1)         |       ...          | child_MaxChildCount|
                      |-- a key space ---| key head --^     |                                                                              |-- a child space--|                   
                      |   0th key space  |    1th key space |  2th key space   |                                        |  0th child space |  1th child space |  2th child space   |

a key space includes bst_node and a key

key head is the base of a key

a child space includes a child

collect unused key space by key_head : free_space_link, it's a linked list

collect unused child space by key_head : free_space_link, it's a linked list

bst_node is used to maintain a balaced binary search tree

key_i is not necessary to be less or greater than key_(i+1), in ab_tree_node layout description

it is just an offset of the space

if x is pointer pointed to by a leaf node, x is an offset to child


        |key|
       / 
      /
    |key|
        \ <------ Is_right_ptr_to_leaf is set, and suppose that this ptr is 5, the leaf is stored at childspace child5 
        (leaf)


*/

static inline void* ptr_to_ith_keyspace(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index ith)
{
    void *ret = node->_node_space + sizeof(ab_tree_node_header) + ith*(sizeof(bst_node)+node_properties->key_size);
    return ret;
}

static inline void* ptr_to_ith_key(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index ith)
{
    void *ret = node->_node_space + sizeof(ab_tree_node_header) + ith*(sizeof(bst_node)+node_properties->key_size) + sizeof(bst_node);
    return ret;
}


static inline  void* ptr_to_ith_childspace(const tree_node_properties *node_properties, const ab_tree_node *node, childspace_index ith)
{
    void *ret = node->_node_space + sizeof(ab_tree_node_header) + (node_properties->MaxChildCount-1)*(sizeof(bst_node) + node_properties->key_size) + ith*(node_properties->child_size);
    return ret;
}

static inline keyspace_index GetFreeKeySpace(const tree_node_properties *node_properties, const ab_tree_node *node)
{

    keyspace_index Head = ((ab_tree_node_header*)(node->_node_space))->FreeKeyListHead;

    keyspace_index NewHead = ((space_head*)ptr_to_ith_keyspace(node_properties, node, Head))->free_space_link;

    ((ab_tree_node_header*)(node->_node_space))->FreeKeyListHead = NewHead;

    return Head;
}

static inline childspace_index GetFreeChildSpace(const tree_node_properties *node_properties, const ab_tree_node *node)
{
    childspace_index Head = ((ab_tree_node_header*)(node->_node_space))->FreeChildListHead;

    childspace_index NewHead = ((space_head*)ptr_to_ith_childspace(node_properties, node, Head))->free_space_link;

    ((ab_tree_node_header*)(node->_node_space))->FreeChildListHead = NewHead;

    return Head; 
}

static inline void RecycleKeySpace(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index keyspace)
{ 
    //*(keyspace_index*)ptr_to_ith_keyspace(node_properties, node, keyspace) = ;
    
    ((space_head*)ptr_to_ith_keyspace(node_properties, node, keyspace))->free_space_link = ((ab_tree_node_header*)(node->_node_space))->FreeKeyListHead;
    
    ((ab_tree_node_header*)(node->_node_space))->FreeKeyListHead = keyspace;

 
}

static inline void RecycleChildSpace(const tree_node_properties *node_properties, const ab_tree_node *node, childspace_index childspace)
{

    ((space_head*)ptr_to_ith_keyspace(node_properties, node, childspace))->free_space_link = ((ab_tree_node_header*)(node->_node_space))->FreeChildListHead;
    
    ((ab_tree_node_header*)(node->_node_space))->FreeKeyListHead = childspace;
  
}


// at most node_properties->MaxChildCount-1 key space is used, so node_properties->MaxChildCount can be used as if is a nullptr
static inline int NULL_PTR(const tree_node_properties *node_properties)
{
    return node_properties->MaxChildCount;
}

static inline int Is_full_node(const tree_node_properties *node_properties, const ab_tree_node *node)
{
    return node_properties->MaxChildCount-1 == ((ab_tree_node_header*)(node->_node_space))->KeyCount;
}


static inline keyspace_index LeftRotateUp(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key)
{
    bst_node *const cur = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, loc_key));
    
    keyspace_index ret = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, loc_key)))->LeftPtr;
            
    bst_node *const left_child = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->LeftPtr));

    printf("left : %d %d\n",*(int*)(ptr_to_ith_key(node_properties, node, loc_key)), *(int*)(ptr_to_ith_key(node_properties, node, ret)));

    cur->LeftPtr = left_child->RightPtr;
    
    cur->Is_left_ptr_to_leaf = left_child->Is_right_ptr_to_leaf;

    left_child->RightPtr = loc_key;    

    left_child->Is_right_ptr_to_leaf = 0;

    UpdateWeight(node_properties, node, loc_key);

    UpdateWeight(node_properties, node, ret);

    return ret;
}

static inline keyspace_index RightRotateUp(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key)
{
    bst_node *const cur = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, loc_key));
    
    keyspace_index ret = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, loc_key)))->RightPtr;
            
    bst_node *const right_child = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->RightPtr));

    printf("right : %d %d\n",*(int*)(ptr_to_ith_key(node_properties, node, loc_key)), *(int*)(ptr_to_ith_key(node_properties, node, ret)));

    cur->RightPtr = right_child->LeftPtr;
    
    cur->Is_right_ptr_to_leaf = right_child->Is_left_ptr_to_leaf;

    right_child->LeftPtr = loc_key;    

    right_child->Is_left_ptr_to_leaf = 0;

    UpdateWeight(node_properties, node, loc_key);

    UpdateWeight(node_properties, node, ret);

    return ret;
}

static inline void UpdateWeight(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index loc_key)
{
    uint16_t weight_left, weight_right;
    bst_node *const cur = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, loc_key));    

    if (cur->Is_left_ptr_to_leaf)
        weight_left = 0;
    else
        weight_left = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->LeftPtr)))->weight;
    
    if (cur->Is_right_ptr_to_leaf)
        weight_right = 0;
    else
        weight_right = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->RightPtr)))->weight;
    
    cur->weight = 1 + weight_left + weight_right;
}



int CompareIntKey(const void *key1, const void *key2)
{
    //printf("compare %lld %lld\n",*(int*)key1, *(int*)key2);
    if (*(int*)key1 > *(int*)key2)
        return 1;
    if (*(int*)key1 < *(int*)key2)
        return -1;
    return 0;    
}


void init_tree_node_properties(struct tree_node_properties *targ, size_t key_size, size_t child_size)
{
    targ->key_size = key_size;
    targ->child_size = child_size;
    targ->MaxChildCount = (NodeSize-sizeof(struct ab_tree_node_header)) / (key_size + sizeof(struct bst_node) + child_size);

}




void *MakeNode(tree_node_properties *src, size_t node_size, int IsBottom)
{  

    ab_tree_node node;
    node._node_space = (char*)malloc(node_size);
  
    struct ab_tree_node_header *header = (struct ab_tree_node_header *)node._node_space;

    header->FreeChildListHead = 0;

    header->FreeKeyListHead = 0;

    header->IsBottom = IsBottom;

    header->KeyCount = 0;

    header->root_offset = NULL_PTR(src);

    int i ;

    for(i = 0 ; i < src->MaxChildCount-1 ; i++)
    {
        void *ptr = ptr_to_ith_keyspace(src, &node, i);
        ((space_head*)ptr)->free_space_link = i + 1; 
        
    }

    for(i = 0 ; i < src->MaxChildCount ; i++)
    {
        void *ptr = ptr_to_ith_childspace(src, &node, i);
        ((space_head*)ptr)->free_space_link = i + 1;   
    }
    return node._node_space; 
}



int SeachKey(const tree_node_properties *node_properties, const ab_tree_node *node, const void *key, KeyCompareResult SearchPath[128], int *SearchPathLength)
{
    int KeyIsFound = 0, cmp;

    keyspace_index cur_offset = ((ab_tree_node_header*)(node->_node_space))->root_offset;

    bst_node *cur = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, cur_offset));
    
    if(cur_offset == NULL_PTR(node_properties))
    {
        *SearchPathLength = 0;
        return 0;
    }   

    while(1) /* stop when the key is found or when it achieves a leaf */
    {
        cmp = CompareIntKey(key, ptr_to_ith_key(node_properties, node, cur_offset));
        SearchPath[*SearchPathLength].idx_key = cur_offset;
        *SearchPathLength += 1;        
        if (cmp == -1)
        {
            SearchPath[(*SearchPathLength)-1].branch_child = 0;

            if (((bst_node*)ptr_to_ith_keyspace(node_properties, node, cur_offset))->Is_left_ptr_to_leaf)
                break;
            
            cur_offset = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, cur_offset))->LeftPtr;
        }
        else if (cmp == 1)
        {
            SearchPath[(*SearchPathLength)-1].branch_child = 1;

            if (((bst_node*)ptr_to_ith_keyspace(node_properties, node, cur_offset))->Is_right_ptr_to_leaf)
                break;
            
            cur_offset = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, cur_offset))->RightPtr;
        }
        else
        {
            KeyIsFound = 1;
            break;
        }
    }
    return KeyIsFound;
}
 // get internal predcessor
keyspace_index GetPredcessor(const tree_node_properties *node_properties, const ab_tree_node *node, keyspace_index src) 
{
    if(((bst_node*)(ptr_to_ith_keyspace(node_properties, node, src)))->Is_left_ptr_to_leaf)
        return src;

    keyspace_index predecessor = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, src)))->LeftPtr;

    while(((bst_node*)(ptr_to_ith_keyspace(node_properties, node, predecessor)))->Is_right_ptr_to_leaf == 0)
        predecessor = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, predecessor)))->RightPtr;

    return predecessor;
}

//update weight and rotate if necessary
void UpdateWeight_to_Root(const tree_node_properties *node_properties, const ab_tree_node *node, KeyCompareResult SearchPath[128], int SearchPathLength)
{
    int i, rotate_flag = 0 ;

    keyspace_index rotated_node = (keyspace_index)NULL_PTR(node_properties); 

    for(i = SearchPathLength-1 ; i >= 0 ; i--) // there is at most one adjust in the path
    {
        printf("i = %d\n",i);
        /*if (rotate_flag == 1) // a node has ratated, no need to check balance
        {
            ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, SearchPath[i].idx_key)))->weight += 1;
            continue;
        }*/
        if (rotated_node != (keyspace_index)NULL_PTR(node_properties))
        {
            if (SearchPath[i].branch_child == 0)
                ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, SearchPath[i].idx_key)))->LeftPtr = rotated_node;
            else
                ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, SearchPath[i].idx_key)))->RightPtr = rotated_node;
            rotate_flag = 1 ;
            rotated_node = (keyspace_index)NULL_PTR(node_properties);
        }

        uint16_t weight_left, weight_right;
        bst_node *const cur = (bst_node*)(ptr_to_ith_keyspace(node_properties, node, SearchPath[i].idx_key));    

        if (cur->Is_left_ptr_to_leaf)
            weight_left = 0;
        else
            weight_left = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->LeftPtr)))->weight;
        
        if (cur->Is_right_ptr_to_leaf)
            weight_right = 0;
        else
            weight_right = ((bst_node*)(ptr_to_ith_keyspace(node_properties, node, cur->RightPtr)))->weight;
       
        if (weight_left > weight_right + 1) // adjust and the left child rotates up
        {
            if (SearchPath[i+1].branch_child == 1)
            {
                printf("-----------------\n");
                //cur->LeftPtr = RightRotateUp(node_properties, node, SearchPath[i+1].idx_key);
                //KeyCompareResult tmp = SearchPath[i+2] ;
                //SearchPath[i+2] = SearchPath[i+1];
                //SearchPath[i+1] = tmp;
            }
               
            rotated_node = LeftRotateUp(node_properties, node, SearchPath[i].idx_key); 
            //KeyCompareResult tmp = SearchPath[i+1] ;
            //SearchPath[i+1] = SearchPath[i];
            //SearchPath[i] = tmp; 
            printf("rotated node %d\n",*(int*)(ptr_to_ith_key(node_properties, node, rotated_node)));

        }   
        else if (weight_left < weight_right - 1) // adjust and the right child rotates up
        {
            if (SearchPath[i+1].branch_child == 0)
            {
                //printf("-----------------\n");
                //cur->RightPtr = LeftRotateUp(node_properties, node, SearchPath[i+1].idx_key);
                //KeyCompareResult tmp = SearchPath[i+2] ;
                //SearchPath[i+2] = SearchPath[i+1];
                //SearchPath[i+1] = tmp;
            }
                
            rotated_node = RightRotateUp(node_properties, node, SearchPath[i].idx_key);
            //KeyCompareResult tmp = SearchPath[i+1] ;
           // SearchPath[i+1] = SearchPath[i];
            //SearchPath[i] = tmp;
            printf("rotated node %d\n",*(int*)(ptr_to_ith_key(node_properties, node, rotated_node)));
        }
        else
            cur->weight += 1;
    }
    if (rotated_node != NULL_PTR(node_properties)) // the root node rotated down, so update root_offset
        ((ab_tree_node_header*)(node->_node_space))->root_offset = rotated_node;

}

void AddKeyChild(const tree_node_properties *node_properties, const ab_tree_node *node, KV_pair kv)
{
    if (node_properties->MaxChildCount -1 == ((ab_tree_node_header*)(node->_node_space))->KeyCount)
    {
        printf("full\n");
        return;
    }

    if (((ab_tree_node_header*)(node->_node_space))->root_offset == NULL_PTR(node_properties)) // is this node an empty node ?
    {       
        childspace_index FreeChildSpace = GetFreeChildSpace(node_properties, node);     
        keyspace_index FreeKeySpace = GetFreeKeySpace(node_properties, node);

        ((ab_tree_node_header*)(node->_node_space))->root_offset = FreeKeySpace;

        bst_node *const new_node = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, FreeKeySpace));      

        new_node->Is_left_ptr_to_leaf = 1;       
        new_node->Is_right_ptr_to_leaf = 1;      
        new_node->weight = 1;
        new_node->LeftPtr = FreeChildSpace;

        memcpy(ptr_to_ith_key(node_properties, node, FreeKeySpace), kv.key, node_properties->key_size);       
        memcpy(ptr_to_ith_childspace(node_properties, node, FreeChildSpace), kv.val, node_properties->child_size);

        ((ab_tree_node_header*)(node->_node_space))->KeyCount += 1;
        printf("add root\n");
        return;
    }   

    KeyCompareResult SearchPath[32]; // SearchPath will not include the new_node
    
    int SearchPathLength = 0;

    int KeyIsFound = SeachKey(node_properties, node, kv.key, SearchPath, &SearchPathLength); // search the key and record the search path

    if (KeyIsFound)  // the key is found, just modify the child
    {     
        childspace_index dst_childspace;   
        keyspace_index predecessor;

        predecessor = GetPredcessor(node_properties, node, SearchPath[SearchPathLength-1].idx_key);   
        bst_node const *predecessor_node = ((bst_node*)ptr_to_ith_key(node_properties, node, predecessor));

        if (predecessor == SearchPath[SearchPathLength-1].idx_key) // true if the node which contains the key has left leaf ptr
            dst_childspace = predecessor_node->LeftPtr;    // then modify its own left child, otherwise modify right child of the predecessor
        else
            dst_childspace = predecessor_node->RightPtr;

        memcpy(ptr_to_ith_childspace(node_properties, node, dst_childspace), kv.val, node_properties->child_size);
    }
    else
    {
        childspace_index FreeChildSpace = GetFreeChildSpace(node_properties, node);      
        keyspace_index FreeKeySpace = GetFreeKeySpace(node_properties, node);

        bst_node *const new_node = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, FreeKeySpace));
        bst_node *const last_node = ((bst_node*)ptr_to_ith_keyspace(node_properties, node, SearchPath[SearchPathLength-1].idx_key));

        memcpy(ptr_to_ith_key(node_properties, node, FreeKeySpace), kv.key, node_properties->key_size);
        memcpy(ptr_to_ith_childspace(node_properties, node, FreeChildSpace), kv.val, node_properties->child_size);

        new_node->Is_left_ptr_to_leaf = 1;
        new_node->Is_right_ptr_to_leaf = 1;
        new_node->RightPtr = last_node->LeftPtr;
        new_node->LeftPtr = FreeChildSpace;
        new_node->weight = 1;

        if (SearchPath[SearchPathLength-1].branch_child == 0)
        {
            last_node->LeftPtr = FreeKeySpace;
            last_node->Is_left_ptr_to_leaf = 0;          
        }
        else
        {
            last_node->RightPtr = FreeKeySpace;
            last_node->Is_right_ptr_to_leaf = 0;   
        }       
        ((ab_tree_node_header*)(node->_node_space))->KeyCount += 1;

        UpdateWeight_to_Root(node_properties, node, SearchPath, SearchPathLength);
    }
}










void Collect_bst_node_by_inorder(tree_node_properties *node_properties, ab_tree_node *node, keyspace_index targ, keyspace_index container[])
{
    int  top_stack = 0, collected_node_length = 0;                                                                                                                                      

    const bst_node *cur = (bst_node *)ptr_to_ith_keyspace(node_properties, node, targ);

    keyspace_index path[cur->weight];

    path[0] = targ;

    int move_state = 0; //0 : is moving down, 1 : just turned back from left child, 2 : just turned back from right child

    while(top_stack >= 0)
    {
        if (move_state == 0)
        {
            if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack]))->Is_left_ptr_to_leaf)
            {
                move_state = 1;
                continue;
            }
            top_stack += 1;
            path[top_stack] = ((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->LeftPtr;
        }
        else if (move_state == 1)
        {
            container[collected_node_length] = path[top_stack];
            collected_node_length += 1;

            if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack]))->Is_right_ptr_to_leaf == 0)
            {
                 move_state = 0;
                 top_stack += 1;
                 path[top_stack] = ((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->RightPtr;
            }    
            else
                move_state = 2;   
        }
        else
        {
            if (top_stack > 0)
            {
                if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->LeftPtr == path[top_stack])
                    move_state = 1;
                else
                    move_state = 2;
            }
            top_stack -= 1;
        }
    }
    int i = 0 ;
    for(; i < collected_node_length ; i++)
        printf("%d ",*(int*)(ptr_to_ith_key(node_properties, node, container[i])));
}

/*
void RebuildTree(tree_node_properties *node_properties, ab_tree_node *node, keyspace_index targ)
{
    const bst_node *cur = (bst_node *)ptr_to_ith_keyspace(node_properties, node, targ);
   
    if (targ == NULL_PTR(node_properties))
        return ;
    printf(":: %d\n",cur->weight);
    keyspace_index container[cur->weight];

    Collect_bst_node_by_inorder(node_properties, node, targ, container);

    int i = 0 ;
    for(i = 0 ; i < cur->weight ; i++)
    {
        printf("%d ",*(int*)ptr_to_ith_key(node_properties, node, container[i]));

    }
}
*/


void traverse(tree_node_properties *node_properties, ab_tree_node *node)
{
int  top_stack = 0;                                                                                                                                      

    const bst_node *cur = (bst_node *)ptr_to_ith_keyspace(node_properties, node, ((ab_tree_node_header*)(node->_node_space))->root_offset);
    
    keyspace_index path[cur->weight];

    path[0] = ((ab_tree_node_header*)(node->_node_space))->root_offset;

    int move_state = 0; //0 : is moving down, 1 : just turned back from left child, 2 : just turned back from right child

    while(top_stack >= 0)
    {
        if (move_state == 0)
        {
            if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack]))->Is_left_ptr_to_leaf)
            {
                move_state = 1;
                continue;
            }
            top_stack += 1;
            path[top_stack] = ((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->LeftPtr;
        }
        else if (move_state == 1)
        {           
            printf("%d %d\n",*(int*)ptr_to_ith_key(node_properties, node, path[top_stack]), ((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack]))->weight);
            if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack]))->Is_right_ptr_to_leaf == 0)
            {
                 move_state = 0;
                 top_stack += 1;
                 path[top_stack] = ((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->RightPtr;
            }    
            else
                move_state = 2;   
        }
        else
        {
            if (top_stack > 0)
            {
                if (((bst_node *)ptr_to_ith_keyspace(node_properties, node, path[top_stack-1]))->LeftPtr == path[top_stack])
                    move_state = 1;
                else
                    move_state = 2;
            }
            top_stack -= 1;
        }
    }
    printf("finish traverse\n");
}







ab_tree_node *SplitNode(tree_node_properties *node_properties, ab_tree_node *node, int split_idx)
{
    ab_tree_node *new_node;
    new_node->_node_space = MakeNode(node_properties, NodeSize, ((ab_tree_node_header*)node->_node_space)->IsBottom);

    return NULL;
}



int main()
{
    tree_node_properties *src = malloc(sizeof(tree_node_properties));

    init_tree_node_properties(src, 4, 8);

    printf("%d \n",src->MaxChildCount);

    printf("ab_tree_node_header = %d\n",sizeof(ab_tree_node_header));
    ab_tree_node node;


    node._node_space = MakeNode(src, NodeSize, 1);

    int k = 1;
    uint64_t v = 5434162213215;
    KV_pair kv = {.key = &k, .val = &v};
    AddKeyChild(src, &node, kv);

    printf("%d %lld\n",*(int*)ptr_to_ith_key(src, &node, 0), *(uint64_t*)ptr_to_ith_childspace(src, &node, 0));
    k = 5;
    AddKeyChild(src, &node, kv); 
    k = 3;
    AddKeyChild(src, &node, kv); 
    k = 7;
    AddKeyChild(src, &node, kv);
    //traverse(src, &node);
    k = 11;
    AddKeyChild(src, &node, kv);
    k = 9;
    AddKeyChild(src, &node, kv);
    k = 5243;
    AddKeyChild(src, &node, kv);
    k = 5241;
    AddKeyChild(src, &node, kv);
    k = 5242;
    AddKeyChild(src, &node, kv);
    traverse(src, &node);
    keyspace_index node_container[100];
    Collect_bst_node_by_inorder(src, &node, 5, node_container);
/*    
    AddKeyChild(src, &node, kv);

    printf("%d",*(int*)ptr_to_ith_key(src, &node, 0));
*/
    //printf("%d %d\n",*(int*)n, sizeof(struct ab_tree_node_header));

    //printf("%d \n",src->MaxChildCount);
    /*AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);    
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);
    AddKeyChild(src, &node, kv);  */


}