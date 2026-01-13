/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 如果能改成 unique_ptr 就更好了!
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    std::unique_ptr<Node> next;
    Node* prev = nullptr;
    int value;

    // 这个构造函数有什么可以改进的？
    // Node(int val) {
    //     value = val;
    // }
    explicit Node(int val) : value(val) {}

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        // node->next = next;
        node->next = std::move(this->next);
        node->prev = this;
        if (this->next) {
            node->next->prev = node.get();
        }
    }

    void erase() {
        if (this->next)
            // next->prev = prev;
            this->next->prev = this->prev;
        if (this->prev) {
            // prev->next = next;
            this->prev->next = std::move(next);
        }
    }

    ~Node() {
        printf("~Node(%d)\n", this->value);   // 应输出多少次？为什么少了？
        // 原来是 shared_ptr 循环引用了析构不掉
    }
};

struct List {
    std::unique_ptr<Node> head = nullptr;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        this->head = std::make_unique<Node>(other.head->value); // 浅拷贝
        Node* this_head = this->head.get();
        Node* other_next = other.head->next.get();
        while (other_next) {
            this_head->next = std::make_unique<Node>(other_next->value);
            this_head->next->prev = this_head;
            this_head = this_head->next.get();
            other_next = other_next->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // unique_ptr 本身就没有拷贝构造和拷贝赋值，List可以视作unique_ptr类型变量的集合肯定删了没问题

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        if (head) {
            head->prev = nullptr;
        }
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head);
        if (head)
            head->prev = node.get();
        // head = node;
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

// void print(List lst) {  // 有什么值得改进的？
void print(List const& lst) { 
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    // 未通过erase()测试。其他没问题
    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}