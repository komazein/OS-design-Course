#pragma once
#include "fs_types.h"


/**
 * 提供目录树过于庞大导致大量占用内存问题, 提供剪枝
 * 也针对dcache防止内存占用过大
 * (应该)可以实现dache和dentry树的替换策略
 * 
 * 类别T:
 * 如果为cache_replacer, 则为dentryKey
 * 如果为dentry_replacer, 则为dentry*
 */


template <typename T>
class LRUReplacer
{
public:

    explicit LRUReplacer(size_t max_size = 1024) : max_size_(max_size) {}


    /**
     * 
     * @brief 将节点插入到`lru_list`的首部,
     * 如果存在于链表中, 则将其移动到链表的首部
     * 
     * @param dentry_node 待插入lru链表中的节点
     * 
     */
    void Insert(T dentry_node);


    /**
     * 
     * @brief 将当前节点以及其父辈的节点插入到`lru_list`的首部,
     * 适用于剪枝操作时, 由于一个访问时间较近的节点不能被替换出去, 也就是说明其父辈节点也不能换出
     * 所以将其沿着父辈方向的节点依次插入`lru_list`
     * 
     * 
     */
    void InsertDir(T dentry_node);

    /**
     * 
     * @brief 优先替换出`lrulist`中的尾部元素
     * 
     * @return `nullptr` 没有可以替换出的节点, 说明此时lru_list为空
     * 
     */
    std::optional<T> Victim();

    /**
     * 
     * @brief 就是指定需要移除的节点, 即无理由替换
     * 
     * @param dentry_node 强制替换出的节点
     * 
     * @return `ture` 如果在表中并替换成功
     * 
     */
    bool Erase(T dentry_node);


    /**
     * 
     * @brief 可以动态的更新最大链表的限制
     * 
     */
    void set_max_size(size_t max_size) { max_size_ = max_size; }

    size_t get_cur_size()               { return lru_list.size(); }
    size_t get_max_size()               { return max_size_; }

    

private:
    
    // 由于本文件系统只考虑到了单进程, 所以不存在引用计数的问题, 
    // 所以只需要一个链表记录缓存节点即可
    // 其替换的优先级就是LRU
    list<T> lru_list;     // lru链表 
    
    size_t max_size_;           // 最大的链表大小
};



template<typename T>
void LRUReplacer<T>::Insert(T dentry_node)
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

template<typename T>
void LRUReplacer<T>::InsertDir(T dentry_node)
{
    // 按照从下到上的顺序进行插入
    T cur_node = dentry_node;
    while(cur_node != nullptr){
        // 直到添加到根节点才结束
        Insert(cur_node);       // 加入到lru_list中
        cur_node = cur_node->get_parent();      // 继续扫描该路径的父节点加入
    }
}

template<typename T>
std::optional<T> LRUReplacer<T>::Victim()
{
    if(lru_list.empty())    { return std::nullopt; }   // 此时表中无节点, 也就是替换失败

    T victim_node = lru_list.back();
    lru_list.pop_back();                    // 移除最后的元素
    return victim_node;
}

template<typename T>
bool LRUReplacer<T>::Erase(T dentry_node)
{
    bool erased = false;
    auto it = find(lru_list.begin(), lru_list.end(), dentry_node);
    if (it != lru_list.end()) {
        lru_list.erase(it);
        erased = true;
    }

    return erased;
}
