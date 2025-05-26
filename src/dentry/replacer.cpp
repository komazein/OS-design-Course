#include "dentry.h"

void LRUReplacer::Insert(dentry* dentry_node)
{

    // 如果节点已在链表，将其插入到链表头部
    auto active_it = find(lru_list.begin(), lru_list.end(), dentry_node);
    if (active_it != lru_list.end()) { 
        lru_list.erase(active_it);
        lru_list.push_front(dentry_node);
        return;
    }

    // 没有的话, 直接将新节点插入到链表头部
    lru_list.push_front(dentry_node);
}

bool LRUReplacer::Victim()
{
    if(lru_list.empty())    { return false; }   // 此时表中无节点, 也就是替换失败

    // auto victim_node = lru_list.back();
    lru_list.pop_back();                    // 移除最后的元素
    return true;
}

bool LRUReplacer::Erase(dentry* dentry_node)
{
    bool erased = false;
    auto it = find(lru_list.begin(), lru_list.end(), dentry_node);
    if (it != lru_list.end()) {
        lru_list.erase(it);
        erased = true;
    }

    return erased;
}